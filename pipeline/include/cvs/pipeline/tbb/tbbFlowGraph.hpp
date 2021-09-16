#pragma once

#include <cvs/pipeline/iexecutionGraph.hpp>
#include <tbb/flow_graph.h>

#include <memory>

namespace cvs::pipeline::tbb {

class TbbFlowGraph : public IExecutionGraph {
 public:
  TbbFlowGraph();

  void waitForAll() override;

  ::tbb::flow::graph&       native();
  const ::tbb::flow::graph& native() const;

 private:
  ::tbb::flow::graph graph;
};

using TbbFlowGraphPtr  = std::shared_ptr<TbbFlowGraph>;
using TbbFlowGraphUPtr = std::unique_ptr<TbbFlowGraph>;

}  // namespace cvs::pipeline::tbb
