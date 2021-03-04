#pragma once

#include <cvs/common/configuration.hpp>

#include <memory>

namespace cvs::pipeline {

class IModuleManager {
 public:
  virtual ~IModuleManager() = default;
};

using IModuleManagerPtr  = std::shared_ptr<IModuleManager>;
using IModuleManagerUPtr = std::unique_ptr<IModuleManager>;

}  // namespace cvs::pipeline
