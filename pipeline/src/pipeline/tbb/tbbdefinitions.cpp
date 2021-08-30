#include "../../../include/cvs/pipeline/tbb/tbbdefinitions.hpp"

namespace cvs::pipeline::tbb {

const std::string TbbDefaultName::graph = "TbbGraph";

const std::string TbbDefaultName::overwrite_in  = "TbbOverwriteInDefault";
const std::string TbbDefaultName::overwrite_out = "TbbOverwriteOutDefault";
const std::string TbbDefaultName::buffer_in     = "TbbBufferInDefault";
const std::string TbbDefaultName::buffer_out    = "TbbBufferOutDefault";
const std::string TbbDefaultName::broadcast_in  = "TbbBroadcastInDefault";
const std::string TbbDefaultName::broadcast_out = "TbbBroadcastOutDefault";
const std::string TbbDefaultName::continue_name = "TbbContinueDefault";
const std::string TbbDefaultName::function      = "TbbFunctionDefault";
const std::string TbbDefaultName::multifunction = "TbbMultifunctionDefault";
const std::string TbbDefaultName::join          = "TbbJoinDefault";
const std::string TbbDefaultName::source        = "TbbSourceDefault";
const std::string TbbDefaultName::split         = "TbbSplitDefault";

}  // namespace cvs::pipeline::tbb
