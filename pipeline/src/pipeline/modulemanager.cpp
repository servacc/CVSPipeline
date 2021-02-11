#include "modulemanager.hpp"

#include <cvs/common/factory.hpp>

REGISTER_TYPE(IModuleManager,
              DefaultModuleManager,
              [](const std::string&, cvs::common::ConfigurationPtr) -> cvs::pipeline::IModuleManagerPtr {
                return std::make_shared<cvs::pipeline::impl::ModuleManager>();
              });

namespace cvs::pipeline::impl {

ModuleManager::ModuleManager() {}

void ModuleManager::init(common::ConfigurationPtr) {}
void ModuleManager::free() {}

}  // namespace cvs::pipeline::impl
