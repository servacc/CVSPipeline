#include <cvs/common/configuration.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/pipeline/ipipeline.hpp>

int main(int argc, char *argv[]) {
  cvs::common::Configuration config;

  auto pipeline = cvs::common::Factory::create<cvs::pipeline::IPipelineUPtr>("Default", config);
  if (!pipeline) {
    // TODO: log error
    return 1;
  }

  return pipeline->exec();
}
