#include "../../include/cvs/pipeline/registrationHelper.hpp"

#include "../../include/cvs/pipeline/impl/guiPipeline.hpp"
#include "../../include/cvs/pipeline/impl/moduleManager.hpp"
#include "../../include/cvs/pipeline/impl/pipeline.hpp"

#include <cvs/logger/tools/fpsCounter.hpp>

namespace {

CVS_CONFIG(FPSCounterSettings, "") {
  CVS_FIELD(name, std::string, "Counter and logger name");
  CVS_FIELD_DEF(count, std::size_t, 10, "Log every <count> frame to debug");
  CVS_FIELD_DEF(ro, double, 0.1, "");
  CVS_FIELD_DEF(thread_safe, bool, true, "");
};

}  // namespace

namespace cvs::pipeline {

void registerDefault(const cvs::common::FactoryPtr<std::string>& factory) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.helper").value();

  std::string module_manager_key = "Default";
  std::string pipeline_key       = "Default";
  std::string gui_pipeline_key   = "Gui";

  std::string fps_counter_key = "NodeCounter";

  if (factory->tryRegisterType<IModuleManagerUPtr(const common::Properties&)>(module_manager_key,
                                                                              impl::ModuleManager::make)) {
    LOG_DEBUG(logger, R"(Register: key "{}" type "{}")", module_manager_key,
              boost::core::demangle(typeid(impl::ModuleManager).name()));
  } else {
    LOG_DEBUG(logger, R"(Type "{}" for key "{}" is not registered)",
              boost::core::demangle(typeid(impl::ModuleManager).name()), module_manager_key);
  }

  if (factory->tryRegisterType<IPipelineUPtr(const common::Properties&, const cvs::common::FactoryPtr<std::string>&)>(
          pipeline_key, impl::Pipeline::make)) {
    LOG_DEBUG(logger, R"(Register: key "{}" type "{}")", pipeline_key,
              boost::core::demangle(typeid(impl::Pipeline).name()));
  } else {
    LOG_DEBUG(logger, R"(Type "{}" for key "{}" is not registered)",
              boost::core::demangle(typeid(impl::ModuleManager).name()), pipeline_key);
  }

  if (factory->tryRegisterType<IPipelineUPtr(const common::Properties&, const cvs::common::FactoryPtr<std::string>&)>(
          gui_pipeline_key, impl::GuiPipeline::make)) {
    LOG_DEBUG(logger, R"(Register: key "{}" type "{}")", gui_pipeline_key,
              boost::core::demangle(typeid(impl::Pipeline).name()));
  } else {
    LOG_DEBUG(logger, R"(Type "{}" for key "{}" is not registered)",
              boost::core::demangle(typeid(impl::ModuleManager).name()), gui_pipeline_key);
  }

  auto counter_registered = factory->tryRegisterType<logger::tools::IFPSCounterPtr(const common::Properties&)>(
      fps_counter_key,
      [counters = std::make_shared<std::map<std::string, logger::tools::IFPSCounterPtr>>()](
          const common::Properties& properties) -> logger::tools::IFPSCounterPtr {
        auto settings = FPSCounterSettings::make(properties).value();

        if (auto iter = counters->find(settings.name); iter != counters->end())
          return iter->second;

        logger::tools::IFPSCounterPtr counter =
            logger::tools::IFPSCounter::make(settings.name, settings.count, settings.ro, settings.thread_safe);
        counters->insert({settings.name, counter});
        return counter;
      });

  if (counter_registered) {
    LOG_DEBUG(logger, R"(Register: key "{}" type "{}")", fps_counter_key,
              boost::core::demangle(typeid(logger::tools::IFPSCounter).name()));
  } else {
    LOG_DEBUG(logger, R"(Type "{}" for key "{}" is not registered)",
              boost::core::demangle(typeid(logger::tools::IFPSCounter).name()), fps_counter_key);
  }
}

}  // namespace cvs::pipeline
