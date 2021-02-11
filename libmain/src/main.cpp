#include <cvs/common/configuration.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/pipeline/ipipeline.hpp>

DEFINE_TYPE(IPipeline, DefaultPipeline);

int main(int argc, char *argv[]) {
  auto confuguration = std::make_shared<cvs::common::Configuration>();
  confuguration->init({});  // TODO: path to config from args

  std::string pipelinte_name = IPipelineDefaultPipelineKey;  // TODO: from confuguration ???

  auto pipeline = cvs::common::Factory::create<cvs::pipeline::IPipeline>(pipelinte_name, confuguration);
  if (!pipeline) {
    // TODO: log error
    return 1;
  }

  pipeline->init(confuguration);
  auto result = pipeline->exec();
  pipeline->free();

  return result;
}
