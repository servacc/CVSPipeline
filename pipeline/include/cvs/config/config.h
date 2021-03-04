#pragma once

#include <string>
#include <optional>
#include <vector>

#include <boost/property_tree/ptree.hpp>


class Config {

 public:
  explicit Config() = default;
  explicit Config(const boost::property_tree::ptree &tree, std::string name = "");
  explicit Config(const boost::property_tree::ptree::value_type &iterator);

  // actually should be make(const std::string& file_content)
  static std::optional<Config> make(const std::string &file_name);

  template <class Config_parser>
  Config_parser parse() {
    return Config_parser::parse_and_make(_tree);
  }

  [[nodiscard]] std::string_view get_name() const;

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

  [[nodiscard]] bool has_value() const;

 private:
  boost::property_tree::ptree _tree;
  std::string _key;
};