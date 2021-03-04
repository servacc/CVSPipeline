#pragma once

#include <filesystem>
#include <memory>

namespace cvs::common {

class Configuration {
 public:
  Configuration();
};

using ConfigurationUPtr = std::unique_ptr<Configuration>;
using ConfigurationPtr  = std::shared_ptr<Configuration>;

}  // namespace cvs::common
