#include <boost/program_options.hpp>
#include <cvs/common/config.hpp>
#include <cvs/common/configbase.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/imodulemanager.hpp>
#include <cvs/pipeline/ipipeline.hpp>
#include <cvs/pipeline/registrationhelper.hpp>
#include <cvs/pipeline/tbb/tbbhelpers.hpp>
#include <fmt/format.h>

#include <iostream>

extern cvs::common::FactoryPtr<std::string> pipelineFactory();

using namespace std::string_literals;
namespace po = boost::program_options;

namespace {

CVSCFG_DECLARE_CONFIG(PipelineConfig, CVSCFG_VALUE_DEFAULT(type, std::string, "Default"))

int execPipeline(const std::string &module_manager_key, const std::string &config_path_string) {
  auto config_opt = cvs::common::Config::makeFromFile(config_path_string);
  if (!config_opt)
    throw std::runtime_error(fmt::format(R"s(Can't load config "{}")s", config_path_string));

  LOG_GLOB_INFO(R"(Config "{}" loaded)", config_path_string);

  cvs::logger::initLoggers(config_opt);

  auto factory = pipelineFactory();

  cvs::pipeline::registerDefault(factory);
  cvs::pipeline::tbb::registerBase(factory);

  auto config = *config_opt;

  auto module_manager = factory->create<cvs::pipeline::IModuleManagerUPtr>(module_manager_key, config);
  if (!module_manager)
    throw std::runtime_error(fmt::format(R"s(Can't create module manager for key "{}")s", module_manager_key));

  module_manager.value()->loadModules();
  module_manager.value()->registerTypes(factory);

  auto pipeline_cfg  = config.getFirstChild("Pipeline").value();
  auto pipeline_type = pipeline_cfg.parse<PipelineConfig>()->type;

  auto pipeline = factory->create<cvs::pipeline::IPipelineUPtr, cvs::common::Config &,
                                  const cvs::common::FactoryPtr<std::string> &>(pipeline_type, pipeline_cfg, factory);
  if (!pipeline)
    throw std::runtime_error(fmt::format(R"s(Can't create pipeline object for type "{}")s", pipeline_type));

  return pipeline.value()->exec();
}

}  // namespace

int main(int argc, char *argv[]) {
  try {
    std::string             module_manager_key;
    std::string             config_path_string;
    po::options_description desc("Allowed options:");
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
    LOG_GLOB_ERROR(R"(Exception: "{}")", e.what());
  }
  catch (...) {
    LOG_GLOB_ERROR(R"(Unknown exception)");
  }

  return 1;
}
