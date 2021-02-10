#include <cvs/common/factory.hpp>
#include <cvs/pipeline/ipipeline.hpp>

DEFINE_TYPE(IPipeline, DefaultPipeline);

int main(int argc, char *argv[]) {
  std::string pipelinte_name = IPipelineDefaultPipelineKey;  // TODO: from args ???

  auto pipeline = cvs::common::Factory::create<cvs::pipeline::IPipeline>(pipelinte_name);
  if (!pipeline) {
    // TODO: log error
    return 1;
  }

  pipeline->init({});
  auto result = pipeline->exec();
  pipeline->free();

  return result;
}
