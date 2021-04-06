#include "../../include/cvs/pipeline/registrationhelper.hpp"

#include "modulemanager.hpp"
#include "pipeline.hpp"

namespace cvs::pipeline {

void registerDefault(cvs::common::FactoryPtr<std::string> factory) {
  factory->registerType<IModuleManagerUPtr(cvs::common::Config&)>("Default", impl::ModuleManager::make);
  factory->registerType<IPipelineUPtr(cvs::common::Config&, const cvs::common::FactoryPtr<std::string>&)>(
      "Default", impl::Pipeline::make);
}

}  // namespace cvs::pipeline
