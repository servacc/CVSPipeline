#include "../../../include/cvs/pipeline/tbb/tbbhelpers.hpp"

namespace cvs::pipeline::tbb {

void registerBase(const cvs::common::FactoryPtr<std::string> &factory) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.tbb.helper");

  if (factory->tryRegisterTypeDefault<IExecutionGraphUPtr(), TbbFlowGraph>(TbbDefaultName::graph)) {
    LOG_DEBUG(logger, R"(Register: key "{}" type "{}")", TbbDefaultName::graph,
              boost::core::demangle(typeid(TbbFlowGraph).name()));
  } else {
    LOG_DEBUG(logger, R"(Type "{}" for key "{}" is not registered)", boost::core::demangle(typeid(TbbFlowGraph).name()),
              TbbDefaultName::graph);
  }
}

}  // namespace cvs::pipeline::tbb
