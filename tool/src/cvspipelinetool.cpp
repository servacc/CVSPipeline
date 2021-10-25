#include <cvs/pipeline/registrationHelper.hpp>
#include <cvs/pipeline/tbb/tbbHelpers.hpp>

cvs::common::FactoryPtr<std::string> pipelineFactory() { return cvs::common::Factory<std::string>::defaultInstance(); }
