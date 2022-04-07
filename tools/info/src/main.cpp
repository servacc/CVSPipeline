#include <boost/program_options.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/imoduleManager.hpp>
#include <cvs/pipeline/registrationHelper.hpp>

#include <iostream>

namespace po = boost::program_options;

void printDescription(const std::string &key, const std::vector<std::string> &element_desc) {
  std::cout << "Element key: " << key << std::endl;
  for (auto str : element_desc)
    std::cout << str << std::endl;
}

int main(int argc, char *argv[]) {
  try {
    std::string module_manager_key;
    std::string config_path_string;
    std::string element_name;

    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help,h"   , "Produce help message")
        ("version,v", "CVSPipeline library version")
        ("config,c" , po::value(&config_path_string)->default_value(DEFAULT_CONFIG),
                      "Config file path. Module loading paths are taken from the specified "
                      "configuration file.")
        ("module,m" , po::value(&module_manager_key)->default_value("Default"),
                      "Key for module manager object")
        ("element,e", po::value(&element_name),
                      "The name of the element for which to display help");
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

    const auto config = cvs::common::CVSConfigBase::load(std::filesystem::path(config_path_string));

    LOG_GLOB_INFO(R"(Config "{}" loaded)", config_path_string);

    auto factory = cvs::common::Factory<std::string>::defaultInstance();

    cvs::logger::initLoggers(config);

    cvs::pipeline::registerDefault(factory);

    auto module_manager = factory->create<cvs::pipeline::IModuleManagerUPtr>(module_manager_key, config);

    module_manager.value()->loadModules();
    module_manager.value()->registerTypes(factory);

    auto info_map = factory->create<std::map<std::string, std::vector<std::string>> *>("info").value();

    if (!element_name.empty()) {
      if (info_map->count(element_name) > 0)
        printDescription(element_name, info_map->at(element_name));
      else
        std::cout << fmt::format(R"(There is no entry for the key "{}".)", element_name) << std::endl;
    } else {
      for (auto i = info_map->begin(); i != info_map->end(); ++i) {
        printDescription(i->first, i->second);
      }
    }

    return 0;
  }
  catch (std::exception &e) {
    LOG_GLOB_ERROR(R"(Exception: "{}")", cvs::common::exceptionStr(e));
  }
  catch (...) {
    LOG_GLOB_ERROR(R"(Unknown exception)");
  }

  return 1;
}