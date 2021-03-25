#include <boost/program_options.hpp>
#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/ipipeline.hpp>

using namespace std::string_literals;
namespace po = boost::program_options;

int main(int argc, char *argv[]) {
  std::string             pipeline_key;
  std::string             config_path_string;
  po::options_description desc("Allowed options:");
  desc.add_options()                                                                                           //
      ("help,h", "produce help message")                                                                       //
      ("version,v", "CVSPipeline library version")                                                             //
      ("config,c", po::value(&config_path_string)->default_value(DEFAULT_CONFIG), "common::Config file path")  //
      ("pipeline,p", po::value(&pipeline_key)->default_value("Default"), "key for pipeline object")            //
      ;                                                                                                        //

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  if (vm.contains("version")) {
    std::cout << PROJECT_VERSION << std::endl << PROJECT_DESCRIPTION << std::endl;
    return 1;
  }

  auto config_opt = cvs::common::Config::makeFromFile(config_path_string);

  cvs::logger::initLoggers(config_opt);

  if (!config_opt.has_value())
    LOG_GLOB_CRITICAL("Can't load config \"{}\"", config_path_string);

  auto pipeline = cvs::common::Factory::create<cvs::pipeline::IPipelineUPtr>(pipeline_key, *config_opt);
  if (!pipeline) {
    LOG_GLOB_CRITICAL("Can't create pipeline object for key \"{}\"", pipeline_key);
    return 1;
  }

  return (*pipeline)->exec();
}
