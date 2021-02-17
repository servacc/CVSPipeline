#pragma once

#include <cvs/common/configuration.hpp>

#include <memory>

namespace cvs::pipeline {

class IModule {
 public:
  virtual IModule(const common::Configuration&) = 0;
  virtual ~IModule() = default;
  virtual std::string name() = 0;
  virtual int version() = 0;
};

using IModulePtr  = std::shared_ptr<IModule>;
using IModuleUPtr = std::unique_ptr<IModule>;

}  // namespace cvs::pipeline
