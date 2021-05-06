#include <boost/program_options.hpp>
#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/imodulemanager.hpp>
#include <cvs/pipeline/ipipeline.hpp>
#include <cvs/pipeline/registrationhelper.hpp>
#include <cvs/pipeline/tbb/tbbhelpers.hpp>

#include <iostream>

extern cvs::common::FactoryPtr<std::string> pipelineFactory();

using namespace std::string_literals;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {
  int result = 1;

  try {
    std::string             module_manager_key;
    std::string             pipeline_key;
    std::string             config_path_string;
    po::options_description desc("Allowed options:");
    desc.add_options()                                                                                           //
        ("help,h", "produce help message")                                                                       //
        ("version,v", "CVSPipeline library version")                                                             //
        ("config,c", po::value(&config_path_string)->default_value(DEFAULT_CONFIG), "Config file path")          //
        ("pipeline,p", po::value(&pipeline_key)->default_value("Default"), "Key for pipeline object")            //
        ("module,m", po::value(&module_manager_key)->default_value("Default"), "Key for module manager object")  //
        ;                                                                                                        //

    po::variables_map cmd_line_vars;
    po::store(po::parse_command_line(argc, argv, desc), cmd_line_vars);
    po::notify(cmd_line_vars);

    if (cmd_line_vars.contains("help")) {
      std::cout << desc << std::endl;
      return 0;
    }

    if (cmd_line_vars.contains("version")) {
      std::cout << PROJECT_VERSION << std::endl << PROJECT_DESCRIPTION << std::endl;
      return 1;
    }

    auto config_opt = cvs::common::Config::makeFromFile(config_path_string);
    if (!config_opt)
      LOG_GLOB_CRITICAL(R"s(Can't load config "{}")s", config_path_string);

    LOG_GLOB_INFO(R"(Config "{}" loaded)", config_path_string);

    cvs::logger::initLoggers(config_opt);

    auto factory = pipelineFactory();

    cvs::pipeline::registerDefault(factory);
    cvs::pipeline::tbb::registerBase(factory);

    auto config = *config_opt;

    auto module_manager = factory->create<cvs::pipeline::IModuleManagerUPtr>(module_manager_key, config);
    if (!module_manager) {
      LOG_GLOB_CRITICAL(R"s(Can't create module manager for key "{}")s", module_manager_key);
      return 1;
    }

    module_manager.value()->loadModules();
    module_manager.value()->registerTypes(factory);

    auto pipeline = factory->create<cvs::pipeline::IPipelineUPtr, cvs::common::Config &,
                                    const cvs::common::FactoryPtr<std::string> &>(pipeline_key, *config_opt, factory);
    if (!pipeline) {
      LOG_GLOB_CRITICAL(R"s(Can't create pipeline object for key "{}")s", pipeline_key);
      return 1;
    }

    result = pipeline.value()->exec();
  }
  catch (std::exception &e) {
    LOG_GLOB_ERROR(R"(Exception: "{}")", e.what());
    result = 1;
  }
  catch (...) {
    LOG_GLOB_ERROR(R"(Unknown exception)");
    result = 1;
  }

  return result;
}
