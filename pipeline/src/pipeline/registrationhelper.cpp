#include "../../include/cvs/pipeline/registrationhelper.hpp"

#include "../../include/cvs/pipeline/impl/modulemanager.hpp"
#include "../../include/cvs/pipeline/impl/pipeline.hpp"

namespace cvs::pipeline {

void registerDefault(const cvs::common::FactoryPtr<std::string>& factory) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.helper");

  std::string module_manager_key = "Default";
  std::string pipeline_key       = "Default";

  if (factory->tryRegisterType<IModuleManagerUPtr(cvs::common::Config&)>(module_manager_key,
                                                                         impl::ModuleManager::make)) {
    LOG_DEBUG(logger, R"(Register: key "{}" type "{}")", module_manager_key,
              boost::core::demangle(typeid(impl::ModuleManager).name()));
  } else {
    LOG_DEBUG(logger, R"(Type "{}" for key "{}" is not registered)",
              boost::core::demangle(typeid(impl::ModuleManager).name()), module_manager_key);
  }

  if (factory->tryRegisterType<IPipelineUPtr(cvs::common::Config&, const cvs::common::FactoryPtr<std::string>&)>(
          pipeline_key, impl::Pipeline::make)) {
    LOG_DEBUG(logger, R"(Register: key "{}" type "{}")", pipeline_key,
              boost::core::demangle(typeid(impl::Pipeline).name()));
  } else {
    LOG_DEBUG(logger, R"(Type "{}" for key "{}" is not registered)",
              boost::core::demangle(typeid(impl::ModuleManager).name()), pipeline_key);
  }
}

}  // namespace cvs::pipeline
