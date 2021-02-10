#pragma once

#include <filesystem>
#include <memory>

namespace cvs::pipeline {

class IPipeline {
 public:
  virtual ~IPipeline() = default;

  virtual void init(std::filesystem::path) = 0;
  virtual int  exec()                      = 0;
  virtual void free()                      = 0;
};

using IPipelineUPtr = std::unique_ptr<IPipeline>;
using IPipelinePtr  = std::shared_ptr<IPipeline>;

}  // namespace cvs::pipeline
