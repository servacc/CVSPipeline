#include <cvs/pipeline/registrationhelper.hpp>
#include <cvs/pipeline/tbb/tbbhelpers.hpp>

cvs::common::FactoryPtr<std::string> pipelineFactory() { return cvs::common::Factory<std::string>::defaultInstance(); }
