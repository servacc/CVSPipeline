#include "modulemanager.hpp"

#include <boost/dll.hpp>
#include <cvs/common/configbase.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/imodule.hpp>

namespace fs = std::filesystem;

using LibraryUPtr = std::unique_ptr<boost::dll::shared_library>;

namespace {

CVSCFG_DECLARE_CONFIG(ModuleManagerConfig, CVSCFG_VALUE_OPTIONAL(module_path, std::string))

}

namespace cvs::pipeline::impl {

struct ModuleManager::ModuleInfo {
  IModuleUPtr module;
  LibraryUPtr library;
};

}  // namespace cvs::pipeline::impl

namespace cvs::pipeline::impl {

std::unique_ptr<ModuleManager> ModuleManager::make(cvs::common::Config& config) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.ModuleManager");

  auto manager_cfg = config.getFirstChild("ModuleManager").value();

  auto paths = manager_cfg.getFirstChild("module_path").value();

  std::set<std::filesystem::path> module_path;
  module_path.insert(CVSPipeline_MODULE_DIR);
  for (auto& p : paths.getChildren()) {
    auto path = p.getValueOptional<fs::path>({}).value();
    module_path.insert(std::move(path));
  }

  for (auto iter = module_path.begin(); iter != module_path.end();) {
    if (!fs::exists(*iter)) {
      LOG_DEBUG(logger, R"s(Skip "{}")s", iter->string());
      iter = module_path.erase(iter);
    } else
      ++iter;
  }

  auto ptr         = std::make_unique<ModuleManager>();
  ptr->module_path = std::move(module_path);

  return ptr;
}

ModuleManager::ModuleManager()
    : cvs::logger::Loggable<ModuleManager>("cvs.pipeline.ModuleManager") {}

ModuleManager::~ModuleManager() { clear(); }

void ModuleManager::loadModules() {
  for (auto& dir : module_path) {
    for (auto& file : fs::directory_iterator(dir)) {
      const bool is_library =
          file.is_regular_file() && (file.path().extension() == ".so" || file.path().extension() == ".dll");
      if (!is_library) {
        LOG_TRACE(logger(), R"s(Skip file "{}")s", file.path().string());
        continue;
      }

      LOG_TRACE(logger(), R"s(Loading "{}"...)s", file.path().string());

      ModuleInfo info;

      info.library = std::make_unique<LibraryUPtr::element_type>(
          file.path().string(), boost::dll::load_mode::rtld_global | boost::dll::load_mode::rtld_lazy);

      info.module = makeModule(*info.library);

      if (info.module) {
        LOG_DEBUG(logger(), R"s(Add module "{}")s", info.module->name());
        modules.emplace(info.module->name(), std::move(info));
      } else {
        LOG_DEBUG(logger(), R"s(Unable to create module from "{}")s", file.path().string());
      }
    }
  }
}

void ModuleManager::registerTypes(cvs::common::FactoryPtr<std::string> factory) {
  for (auto& module : modules) {
    LOG_DEBUG(logger(), R"s(Register types from module "{}")s", module.first);
    module.second.module->registerTypes(factory);
  }
}

void ModuleManager::clear() {
  for (auto& m : modules) {
    m.second.module.reset();
    m.second.library.reset();
  }

  modules.clear();
}

}  // namespace cvs::pipeline::impl
