#include "../../../include/cvs/pipeline/tbb/tbbFlowGraph.hpp"

namespace cvs::pipeline::tbb {

TbbFlowGraph::TbbFlowGraph() {}

void TbbFlowGraph::waitForAll() { graph.wait_for_all(); }

::tbb::flow::graph&       TbbFlowGraph::native() { return graph; }
const ::tbb::flow::graph& TbbFlowGraph::native() const { return graph; }

}  // namespace cvs::pipeline::tbb
