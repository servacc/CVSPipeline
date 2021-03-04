#include "../../../include/cvs/pipeline/tbb/tbbhelpers.hpp"

namespace cvs::pipeline::tbb {

const std::string TbbDefaultName::graph_name = "TbbGraph";

const std::string TbbDefaultName::broadcast_name = "BroadcastDefault";
const std::string TbbDefaultName::continue_name  = "ContinueDefault";
const std::string TbbDefaultName::function_name  = "FunctionDefault";
const std::string TbbDefaultName::join_name      = "JoinDefault";
const std::string TbbDefaultName::source_name    = "SourceDefault";
const std::string TbbDefaultName::split_name     = "SplitDefault";

void registrateBase() {
  common::Factory::registrateDefault<IExecutionGraphUPtr(), TbbFlowGraph>(TbbDefaultName::graph_name);
}

}  // namespace cvs::pipeline::tbb
