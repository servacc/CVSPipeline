#include <boost/program_options.hpp>
#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/imoduleManager.hpp>
#include <cvs/pipeline/ipipeline.hpp>
#include <cvs/pipeline/registrationHelper.hpp>
#include <cvs/pipeline/tbb/tbbHelpers.hpp>
#include <fmt/format.h>

#include <iostream>

extern cvs::common::FactoryPtr<std::string> pipelineFactory();

using namespace std::string_literals;
namespace po = boost::program_options;

namespace {

CVS_CONFIG(PipelineConfig, "") { CVS_FIELD_DEF(type, std::string, "Default", ""); };

int execPipeline(const std::string &module_manager_key, const std::string &config_path_string) {
  const auto config = cvs::common::CVSConfigBase::load(std::filesystem::path(config_path_string));

  LOG_GLOB_INFO(R"(Config "{}" loaded)", config_path_string);

  cvs::logger::initLoggers(config);

  const auto factory = pipelineFactory();

  cvs::pipeline::registerDefault(factory);
  cvs::pipeline::tbb::registerBase(factory);

  auto module_manager = factory->create<cvs::pipeline::IModuleManagerUPtr>(module_manager_key, config);

  module_manager.value()->loadModules();
  module_manager.value()->registerTypes(factory);

  const auto pipeline_cfg  = config.get_child("Pipeline");
  auto       pipeline_type = PipelineConfig::make(pipeline_cfg);

  auto pipeline = factory->create<cvs::pipeline::IPipelineUPtr>(pipeline_type->type, pipeline_cfg, factory);

  return pipeline.value()->exec();
}

}  // namespace

int main(int argc, char *argv[]) {
  try {
    std::string             module_manager_key;
    std::string             config_path_string;
    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help,h"    , "Produce help message")
        ("version,v" , "CVSPipeline library version")
        ("config,c"  , po::value(&config_path_string)->default_value(DEFAULT_CONFIG), "Config file path")
        ("module,m"  , po::value(&module_manager_key)->default_value("Default")     , "Key for module manager object");
    // clang-format on

    po::variables_map cmd_line_vars;
    po::store(po::parse_command_line(argc, argv, desc), cmd_line_vars);
    po::notify(cmd_line_vars);

    if (cmd_line_vars.contains("help")) {
      std::cout << desc << std::endl;
      return 0;
    }

    if (cmd_line_vars.contains("version")) {
      std::cout << PROJECT_VERSION << std::endl << PROJECT_DESCRIPTION << std::endl;
      return 0;
    }

    return execPipeline(module_manager_key, config_path_string);
  }
  catch (std::exception &e) {
    LOG_GLOB_ERROR(R"(Exception: "{}")", cvs::common::exceptionStr(e));
  }
  catch (...) {
    LOG_GLOB_ERROR(R"(Unknown exception)");
  }

  return 1;
}
