#include "pipeline.hpp"

#include <cvs/common/factory.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::impl {

Pipeline::Pipeline() {}

int Pipeline::exec() { return 1; }

}  // namespace cvs::pipeline::impl
