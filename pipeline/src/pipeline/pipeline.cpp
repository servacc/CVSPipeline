#include "pipeline.hpp"

#include <cvs/common/factory.hpp>

REGISTER_TYPE(IPipeline,
              DefaultPipeline,
              [](const std::string&, cvs::common::ConfigurationPtr) -> cvs::pipeline::IPipelinePtr {
                return std::make_shared<cvs::pipeline::impl::Pipeline>();
              });

namespace cvs::pipeline::impl {

Pipeline::Pipeline() {}

void Pipeline::init(common::ConfigurationPtr) {}
int  Pipeline::exec() { return 1; }
void Pipeline::free() {}

}  // namespace cvs::pipeline::impl
