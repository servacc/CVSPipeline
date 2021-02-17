#pragma once

#include "config_base.hpp"

class Config {
 public:
  // actually should be make(const std::string& file_content)
  static std::optional<Config> make(const std::string &file_name);

  template <class Config_parser>
  Config_parser parse() {
    static_assert(std::is_base_of<Config_base, Config_parser>::value, "Provided class not derived from Config_base class");
    return Config_parser(_tree);
  }

 private:
  Config(const std::string &filename);

 private:
  boost::property_tree::ptree _tree;

};