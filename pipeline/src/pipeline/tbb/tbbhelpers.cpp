#include "../../../include/cvs/pipeline/tbb/tbbhelpers.hpp"

namespace cvs::pipeline::tbb {

void registerBase(cvs::common::FactoryPtr<std::string> factory) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.tbb.helper");

  factory->registerTypeDefault<IExecutionGraphUPtr(), TbbFlowGraph>(TbbDefaultName::graph);

  LOG_DEBUG(logger, R"s(Register: key "{}" type "{}")s", TbbDefaultName::graph,
            boost::core::demangle(typeid(TbbFlowGraph).name()));
}

}  // namespace cvs::pipeline::tbb
