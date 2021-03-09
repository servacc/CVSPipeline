#include "../../../include/cvs/pipeline/tbb/tbbhelpers.hpp"

namespace cvs::pipeline::tbb {

void registrateBase() {
  common::Factory::registrateDefault<IExecutionGraphUPtr(), TbbFlowGraph>(TbbDefaultName::graph_name);
}

}  // namespace cvs::pipeline::tbb
