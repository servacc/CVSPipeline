#pragma once

#include <filesystem>
#include <memory>

namespace cvs::pipeline {

class IPipeline {
 public:
  virtual ~IPipeline() = default;

  virtual int exec() = 0;
};

using IPipelineUPtr = std::unique_ptr<IPipeline>;
using IPipelinePtr  = std::shared_ptr<IPipeline>;

}  // namespace cvs::pipeline
