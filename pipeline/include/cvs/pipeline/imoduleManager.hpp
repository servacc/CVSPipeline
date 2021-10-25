#pragma once

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>

#include <memory>

namespace cvs::pipeline {

class IModuleManager {
 public:
  virtual ~IModuleManager() = default;

  virtual void loadModules()                                       = 0;
  virtual void registerTypes(cvs::common::FactoryPtr<std::string>) = 0;
};

using IModuleManagerPtr  = std::shared_ptr<IModuleManager>;
using IModuleManagerUPtr = std::unique_ptr<IModuleManager>;

}  // namespace cvs::pipeline
