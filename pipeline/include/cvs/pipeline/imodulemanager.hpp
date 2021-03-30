#pragma once

#include <cvs/common/config.hpp>

#include <memory>

namespace cvs::pipeline {

class IModuleManager {
 public:
  virtual ~IModuleManager() = default;

  virtual void loadModules(cvs::common::Config&) = 0;
};

using IModuleManagerPtr  = std::shared_ptr<IModuleManager>;
using IModuleManagerUPtr = std::unique_ptr<IModuleManager>;

}  // namespace cvs::pipeline
