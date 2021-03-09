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

  template <typename Config_parser>
  auto parse() {
    return Config_parser::parseAndMake(_tree);
  }

  [[nodiscard]] std::string_view getName() const;

  [[nodiscard]] std::vector<Config> getChildren() const;

  template <typename ResultType>
  [[nodiscard]] std::optional<ResultType> getValueOptional(const std::string& name) const {
    auto result = _tree.get_value_optional<ResultType>(name);
    if (result) {
      return result;
    }
    else {
      return std::nullopt;
    }
  }

  template <typename ResultType>
  [[nodiscard]] ResultType getValueOrDefault(const std::string& name, ResultType default_value) const {
    return _tree.get(name, default_value);
  }

  // clang-format off
  [[nodiscard]] bool has_value() const;
  // clang-format on

 private:
  boost::property_tree::ptree _tree;
  std::string _key;
};