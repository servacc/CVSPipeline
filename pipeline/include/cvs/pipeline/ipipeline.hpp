#pragma once

#include <cvs/pipeline/iexecutionnode.hpp>

#include <memory>

namespace cvs::pipeline {

class IPipeline {
 public:
  virtual ~IPipeline() = default;

  virtual IExecutionNodePtr getNode(std::string_view) const = 0;

  virtual int exec() = 0;

  virtual void waitForAll() = 0;
};

using IPipelineUPtr = std::unique_ptr<IPipeline>;
using IPipelinePtr  = std::shared_ptr<IPipeline>;

}  // namespace cvs::pipeline
