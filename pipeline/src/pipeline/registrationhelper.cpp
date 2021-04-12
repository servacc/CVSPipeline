#include "../../include/cvs/pipeline/registrationhelper.hpp"

#include "modulemanager.hpp"
#include "pipeline.hpp"

namespace cvs::pipeline {

void registerDefault(cvs::common::FactoryPtr<std::string> factory) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.tbb.helper");

  factory->registerType<IModuleManagerUPtr(cvs::common::Config&)>("Default", impl::ModuleManager::make);
  LOG_DEBUG(logger, R"s(Register: key "{}" type "{}")s", "Default",
            boost::core::demangle(typeid(impl::ModuleManager).name()));

  factory->registerType<IPipelineUPtr(cvs::common::Config&, const cvs::common::FactoryPtr<std::string>&)>(
      "Default", impl::Pipeline::make);
  LOG_DEBUG(logger, R"s(Register: key "{}" type "{}")s", "Default",
            boost::core::demangle(typeid(impl::Pipeline).name()));
}

}  // namespace cvs::pipeline
