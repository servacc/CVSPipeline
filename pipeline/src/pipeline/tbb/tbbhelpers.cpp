#include "../../../include/cvs/pipeline/tbb/tbbhelpers.hpp"

namespace cvs::pipeline::tbb {

void registrateBase() {
  common::Factory::registrateDefault<IExecutionGraphUPtr(), TbbFlowGraph>(TbbDefaultName::graph);
}

}  // namespace cvs::pipeline::tbb
