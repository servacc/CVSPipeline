#include "../../../include/cvs/pipeline/tbb/tbbhelpers.hpp"

namespace cvs::pipeline::tbb {

void registrateBase(cvs::common::FactoryPtr<std::string> factory) {
  factory->registerTypeDefault<IExecutionGraphUPtr(), TbbFlowGraph>(TbbDefaultName::graph);
}

}  // namespace cvs::pipeline::tbb
