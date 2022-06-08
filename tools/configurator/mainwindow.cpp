#include "mainwindow.hpp"

#include "./ui_mainwindow.h"
#include "comboItemDelegate.hpp"
#include "headerProxyModel.hpp"

#include <cvs/common/general.hpp>
#include <cvs/logger/loggerTypes.hpp>
#include <cvs/pipeline/iexecutionGraph.hpp>
#include <cvs/pipeline/iexecutionNode.hpp>
#include <cvs/pipeline/impl/moduleManager.hpp>

#include <QDataWidgetMapper>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QStandardItemModel>
#include <filesystem>
#include <regex>
#include <typeindex>

namespace fs = std::filesystem;

namespace {

QString strToQstr(const std::string_view &str) { return QString::fromLocal8Bit(str.data(), str.size()); }

}  // namespace

struct JsonKeys {
  static constexpr const char *loggers          = "loggers";
  static constexpr const char *module_manager   = "ModuleManager";
  static constexpr const char *module_path_list = "module_path";
};

enum ModelRootRows {
  Loggers = 0,
  LoggerSinks,
  LoggerLevels,
  LoggerTime,

  ModulePaths,
  Modules,

  Elements,

  Count
};

enum ModuleColumns {
  Dafault = 0,
  ModuleName,
  ModuleVersion,

  ElementName,
  ElementDescription,

  ModuleColumnsCount
};

class MainWindow::Private : public Ui::MainWindow {
 public:
  Private(::MainWindow *ptr)
      : parent(ptr) {}

  void initModel();

  void loadConfig(const fs::path &);
  void loadLoggers(const QJsonObject &root);
  void loadModulePaths(const QJsonObject &root);
  void loadModules(const std::filesystem::path &, const QModelIndex &);
  void loadElements(const std::string &name, int mod_ver, const QModelIndex &root_index);

  void initMapper();

  ::MainWindow *parent = nullptr;

  QStandardItemModel config_model;
  HeaderProxyModel   loggers_model;

  ComboItemDelegate logger_sink_delegate;
  ComboItemDelegate logger_level_delegate;
  ComboItemDelegate logger_time_delegate;

  std::unique_ptr<cvs::pipeline::impl::ModuleManager>         module_manager;
  std::map<std::string, cvs::common::FactoryPtr<std::string>> module_factories;

  QDataWidgetMapper module_info_mapper;
};

void MainWindow::Private::initMapper() {
  module_info_mapper.setModel(&config_model);

  module_info_mapper.addMapping(module_name, ModuleColumns::ModuleName, "text");
  module_info_mapper.addMapping(module_version, ModuleColumns::ModuleVersion, "text");
  module_info_mapper.addMapping(elem_name, ModuleColumns::ElementName, "text");
  module_info_mapper.addMapping(elem_description, ModuleColumns::ElementDescription, "plainText");

  QObject::connect(module_paths->selectionModel(), &QItemSelectionModel::currentChanged,
                   [&](const QModelIndex &current, const QModelIndex &previous) {
                     module_info_mapper.setRootIndex(current.parent());
                     module_info_mapper.setCurrentModelIndex(current);
                   });
}

void MainWindow::Private::loadElements(const std::string &module_name, int mod_ver, const QModelIndex &root_index) {
  auto factory = std::make_shared<cvs::common::Factory<std::string>>();
  module_manager->registerTypesFrom(module_name, factory);
  module_factories.emplace(module_name, factory);

  auto info_map = factory->create<std::map<std::string, std::vector<std::string>> *>("info");

  auto keys = factory->keys();
  config_model.insertColumns(0, ModuleColumnsCount, root_index);

  using create_node_fun_t = typename cvs::pipeline::IExecutionNodeUPtr(
      const std::string &, const cvs::common::Properties &, cvs::pipeline::IExecutionGraphPtr &);

  for (auto k = keys.begin(); k != keys.end(); ++k) {
    auto name = k->first;
    if (k->first.find(".in[") != std::string::npos || k->first.find(".out[") != std::string::npos ||
        k->first.find(".dummy") != std::string::npos || k->first.front() == '*')
      continue;

    for (auto sig : k->second) {
      if (sig != typeid(create_node_fun_t))
        continue;

      auto i = config_model.rowCount(root_index);

      config_model.insertRow(i, root_index);
      auto index = config_model.index(i, 0, root_index);

      auto mod_name_index    = config_model.index(i, ModuleColumns::ModuleName, root_index);
      auto mod_version_index = config_model.index(i, ModuleColumns::ModuleVersion, root_index);
      auto elem_name_index   = config_model.index(i, ModuleColumns::ElementName, root_index);
      auto elem_desc_index   = config_model.index(i, ModuleColumns::ElementDescription, root_index);

      std::string descr;
      if (info_map) {
        for (auto s : info_map.value()->at(k->first))
          descr += s + '\n';
      }

      config_model.setData(index, k->first.c_str());

      config_model.setData(mod_name_index, module_name.c_str());
      config_model.setData(mod_version_index, mod_ver);
      config_model.setData(elem_name_index, k->first.c_str());
      config_model.setData(elem_desc_index, descr.c_str());

      config_model.item(ModelRootRows::ModulePaths)
          ->child(root_index.parent().row())
          ->child(root_index.row())
          ->child(i)
          ->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
    }
  }
}

void MainWindow::Private::loadModules(const std::filesystem::path &path, const QModelIndex &root_index) {
  auto modules = module_manager->loadModulesFrom(path);
  config_model.insertColumns(0, ModuleColumnsCount, root_index);
  config_model.insertRows(0, modules.size(), root_index);

  for (auto i = 0; i < modules.size(); ++i) {
    auto index             = config_model.index(i, 0, root_index);
    auto mod_name_index    = config_model.index(i, ModuleColumns::ModuleName, root_index);
    auto mod_version_index = config_model.index(i, ModuleColumns::ModuleVersion, root_index);

    config_model.setData(index, modules[i].c_str());
    config_model.setData(mod_name_index, modules[i].c_str());
    config_model.setData(mod_version_index, module_manager->moduleVersion(modules[i]));

    config_model.item(ModelRootRows::ModulePaths)
        ->child(root_index.row())
        ->child(i)
        ->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);

    loadElements(modules[i], module_manager->moduleVersion(modules[i]), index);
  }
}

void MainWindow::Private::loadModulePaths(const QJsonObject &root) {
  using cvs::logger::LoggerConfig;

  auto json      = QJsonDocument(root).toJson().toStdString();
  module_manager = cvs::pipeline::impl::ModuleManager::make(cvs::common::CVSConfigBase::load(json));

  auto paths = module_manager->modulesPaths();

  auto model_root = config_model.index(ModelRootRows::ModulePaths, 0);
  config_model.insertRows(0, paths.size(), model_root);

  int row = 0;
  for (auto p : paths) {
    auto index = config_model.index(row, 0, model_root);
    config_model.setData(index, p.c_str());

    config_model.item(ModelRootRows::ModulePaths)
        ->child(row)
        ->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);

    loadModules(p, index);

    ++row;
  }
}

void MainWindow::Private::initModel() {
  config_model.setColumnCount(1);
  config_model.setRowCount(ModelRootRows::Count);

  // logger proxy model
  {
    using cvs::logger::LoggerConfig;

    loggers_model.setSourceModel(&config_model);

    auto name_hdr      = strToQstr(LoggerConfig::name_descriptor.description());
    auto level_hdr     = strToQstr(LoggerConfig::level_descriptor.description());
    auto pattern_hdr   = strToQstr(LoggerConfig::pattern_descriptor.description());
    auto time_type_hdr = strToQstr(LoggerConfig::time_type_descriptor.description());
    auto sink_hdr      = strToQstr(LoggerConfig::sink_descriptor.description());

    loggers_model.setHeaderData(0, Qt::Horizontal, name_hdr, Qt::DisplayRole);
    loggers_model.setHeaderData(1, Qt::Horizontal, level_hdr, Qt::DisplayRole);
    loggers_model.setHeaderData(2, Qt::Horizontal, sink_hdr, Qt::DisplayRole);
    loggers_model.setHeaderData(3, Qt::Horizontal, pattern_hdr, Qt::DisplayRole);
    loggers_model.setHeaderData(4, Qt::Horizontal, time_type_hdr, Qt::DisplayRole);
  }

  // logger sinks
  {
    auto log_sinks_root = config_model.index(ModelRootRows::LoggerSinks, 0);
    config_model.insertColumns(0, 1, log_sinks_root);
    config_model.insertRows(0, 3, log_sinks_root);

    boost::property_tree::translator_between<std::string, cvs::logger::Sink>::type sink_translator;

    for (int i = 0; i < 3; ++i) {
      auto index = config_model.index(i, 0, log_sinks_root);
      config_model.setData(index, sink_translator.put_value(cvs::logger::Sink(i))->c_str());
    }
  }

  // logger levels
  {
    auto log_level_count = 7;
    auto log_levels_root = config_model.index(ModelRootRows::LoggerLevels, 0);
    config_model.insertColumns(0, 1, log_levels_root);
    config_model.insertRows(0, log_level_count, log_levels_root);

    boost::property_tree::translator_between<std::string, spdlog::level::level_enum>::type level_translator;

    for (int i = 0; i < log_level_count; ++i) {
      auto index = config_model.index(i, 0, log_levels_root);
      config_model.setData(index, level_translator.put_value(spdlog::level::level_enum(i))->c_str());
    }
  }

  // logger time
  {
    auto log_time_count  = 2;
    auto log_levels_root = config_model.index(ModelRootRows::LoggerTime, 0);
    config_model.insertColumns(0, 1, log_levels_root);
    config_model.insertRows(0, log_time_count, log_levels_root);

    boost::property_tree::translator_between<std::string, spdlog::pattern_time_type>::type level_translator;

    for (int i = 0; i < log_time_count; ++i) {
      auto index = config_model.index(i, 0, log_levels_root);
      config_model.setData(index, level_translator.put_value(spdlog::pattern_time_type(i))->c_str());
    }
  }

  // modules paths
  {
    auto paths_root = config_model.index(ModelRootRows::ModulePaths, 0);
    config_model.insertColumns(0, 1, paths_root);
  }
}

void MainWindow::Private::loadConfig(const std::filesystem::__cxx11::path &path) {
  QFile file(path);
  if (!file.open(QFile::ReadOnly)) {
    QMessageBox::critical(parent, tr("Read error"), tr(R"(Can't open file "%0")").arg(path.c_str()));
    return;
  }

  QJsonParseError error;
  auto            doc = QJsonDocument::fromJson(file.readAll(), &error);
  if (error.error != QJsonParseError::NoError) {
    QMessageBox::critical(parent, tr("Parse error"), error.errorString());
    return;
  }

  auto root = doc.object();

  loadLoggers(root);
  loadModulePaths(root);
}

void MainWindow::Private::loadLoggers(const QJsonObject &root) {
  using cvs::logger::LoggerConfig;

  if (!root.contains(JsonKeys::loggers))
    return;

  if (!root[JsonKeys::loggers].isArray())
    return;

  auto loggers = root[JsonKeys::loggers].toArray();

  auto root_index = config_model.index(ModelRootRows::Loggers, 0);
  config_model.insertColumns(0, 5, root_index);
  config_model.insertRows(0, loggers.size(), root_index);

  auto name_tooltip      = strToQstr(LoggerConfig::name_descriptor.description());
  auto level_tooltip     = strToQstr(LoggerConfig::level_descriptor.description());
  auto pattern_tooltip   = strToQstr(LoggerConfig::pattern_descriptor.description());
  auto time_type_tooltip = strToQstr(LoggerConfig::time_type_descriptor.description());
  auto sink_tooltip      = strToQstr(LoggerConfig::sink_descriptor.description());

  boost::property_tree::translator_between<std::string, spdlog::level::level_enum>::type level_translator;
  boost::property_tree::translator_between<std::string, cvs::logger::Sink>::type         sink_translator;
  boost::property_tree::translator_between<std::string, spdlog::pattern_time_type>::type pattern_translator;

  int row = 0;
  for (auto val : loggers) {
    auto logger_cfg = cvs::logger::LoggerConfig::make(QJsonDocument(val.toObject()).toJson().toStdString());

    if (!logger_cfg) {
      try {
        std::rethrow_exception(logger_cfg.exception());
      }
      catch (std::exception e) {
        QMessageBox::critical(parent, tr("Logger parse error"), e.what());
      }
      continue;
    }

    // current logger indexes
    auto name_index      = config_model.index(row, 0, root_index);
    auto level_index     = config_model.index(row, 1, root_index);
    auto sink_index      = config_model.index(row, 2, root_index);
    auto pattern_index   = config_model.index(row, 3, root_index);
    auto time_type_index = config_model.index(row, 4, root_index);

    // current logger values
    config_model.setData(name_index, logger_cfg->name.c_str());

    config_model.setData(level_index, logger_cfg->level, Qt::UserRole);
    config_model.setData(level_index, level_translator.put_value(logger_cfg->level)->c_str(), Qt::DisplayRole);

    config_model.setData(pattern_index, logger_cfg->pattern ? logger_cfg->pattern.value().c_str() : QVariant{});

    config_model.setData(time_type_index, int(logger_cfg->time_type), Qt::UserRole);
    config_model.setData(time_type_index, pattern_translator.put_value(logger_cfg->time_type)->c_str(),
                         Qt::DisplayRole);

    auto sink_val = logger_cfg->sink.value_or(cvs::logger::Sink::NO_SINK);
    config_model.setData(sink_index, sink_translator.put_value(sink_val)->c_str(), Qt::DisplayRole);
    config_model.setData(sink_index, int(sink_val), Qt::UserRole);

    auto test0 = config_model.data(sink_index, Qt::DisplayRole);
    auto test1 = config_model.data(sink_index, Qt::UserRole);

    // tooltips
    config_model.setData(name_index, name_tooltip, Qt::ToolTipRole);
    config_model.setData(level_index, level_tooltip, Qt::ToolTipRole);
    config_model.setData(pattern_index, pattern_tooltip, Qt::ToolTipRole);
    config_model.setData(time_type_index, time_type_tooltip, Qt::ToolTipRole);
    config_model.setData(sink_index, sink_tooltip, Qt::ToolTipRole);
    ++row;
  }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m(std::make_unique<Private>(this)) {
  m->setupUi(this);

  m->initModel();

  m->loadConfig("/home/ssysoenko/work/build/vscode/fitPlane/test/road_model_4k_highway_09_09_2021.json");

  {
    m->logger_sink_delegate.setModel(&m->loggers_model, m->loggers_model.index(ModelRootRows::LoggerSinks, 0));
    m->logger_level_delegate.setModel(&m->loggers_model, m->loggers_model.index(ModelRootRows::LoggerLevels, 0));
    m->logger_time_delegate.setModel(&m->loggers_model, m->loggers_model.index(ModelRootRows::LoggerTime, 0));

    m->loggers_table->setItemDelegateForColumn(1, &m->logger_level_delegate);
    m->loggers_table->setItemDelegateForColumn(2, &m->logger_sink_delegate);
    m->loggers_table->setItemDelegateForColumn(4, &m->logger_time_delegate);
    m->loggers_table->setModel(&m->loggers_model);
    m->loggers_table->setRootIndex(m->loggers_model.index(ModelRootRows::Loggers, 0));
  }

  {
    m->module_paths->setModel(&m->config_model);
    m->module_paths->setRootIndex(m->config_model.index(ModelRootRows::ModulePaths, 0));
  }

  m->pipeline_tree->setModel(&m->config_model);

  m->initMapper();
}

MainWindow::~MainWindow() {}
