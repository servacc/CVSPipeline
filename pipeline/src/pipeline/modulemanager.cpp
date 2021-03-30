#include "modulemanager.hpp"

#include <cvs/common/configbase.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/pipeline/imodule.hpp>

#include <filesystem>

namespace fs = std::filesystem;

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

ModuleManager::ModuleManager()
    : cvs::logger::Loggable<ModuleManager>("cvs.pipeline.ModuleManager") {}

ModuleManager::~ModuleManager() { clear(); }

void ModuleManager::loadModules(cvs::common::Config& cfg) {
  auto                  config_opt = cfg.parse<ModuleManagerConfig>();
  std::filesystem::path modules_dir;
  if (config_opt)
    modules_dir = config_opt->module_path.value_or(CVSPipeline_MODULE_DIR);
  else
    modules_dir = CVSPipeline_MODULE_DIR;

  for (auto& file : fs::directory_iterator(modules_dir)) {
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
