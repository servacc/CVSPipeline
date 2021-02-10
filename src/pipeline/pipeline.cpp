#include "pipeline.hpp"

#include <cvs/common/factory.hpp>

REGISTER_TYPE(IPipeline, DefaultPipeline, [](const std::string&) -> cvs::pipeline::IPipelinePtr {
  return std::make_shared<cvs::pipeline::Pipeline>();
});

namespace cvs::pipeline {

Pipeline::Pipeline() {}

void Pipeline::init(std::filesystem::path) {}
int  Pipeline::exec() { return 1; }
void Pipeline::free() {}

}  // namespace cvs::pipeline
