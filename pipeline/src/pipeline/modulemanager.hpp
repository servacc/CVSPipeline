#pragma once

#include <cvs/pipeline/imodulemanager.hpp>

namespace cvs::pipeline::impl {

class ModuleManager : public IModuleManager {
 public:
  ModuleManager();

  void init(common::ConfigurationPtr) override;
  void free() override;
};

}  // namespace cvs::pipeline::impl
