#pragma once

#include <memory>

namespace cvs::pipeline {

class IExecutionGraph {
 public:
  virtual ~IExecutionGraph() = default;

  virtual void waitForAll() = 0;
};

using IExecutionGraphPtr  = std::shared_ptr<IExecutionGraph>;
using IExecutionGraphUPtr = std::unique_ptr<IExecutionGraph>;

}  // namespace cvs::pipeline
