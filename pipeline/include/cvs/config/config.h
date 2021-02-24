#pragma once

#include "config_base.hpp"

class Config {
 public:
  // actually should be make(const std::string& file_content)
  static std::optional<Config> make(const std::string &file_name);

  template <class Config_parser>
  Config_parser parse() {
    return Config_parser::parse_and_make(_tree);
  }

  [[nodiscard]] const std::string_view get_name() const;

  [[nodiscard]] std::vector<Config> get_children() const;

  template <class Result_type>
  [[nodiscard]] std::optional<Result_type> get_value_optional(const std::string& name) const {
    auto result = _tree.get_value_optional<Result_type>(name);
    if (result) {
      return result;
    }
    else {
      return std::nullopt;
    }
  }

  template <class Result_type>
  [[nodiscard]] Result_type get_value_or_default(const std::string& name, Result_type default_value) const {
    return _tree.get(name, default_value);
  }

 private:
  explicit Config(const boost::property_tree::ptree &tree);
  explicit Config(const boost::property_tree::ptree::value_type &iterator);

 private:
  boost::property_tree::ptree _tree;
  std::string _key;
};