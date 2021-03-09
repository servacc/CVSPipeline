#include <cvs/config/config.h>

#include <boost/property_tree/json_parser.hpp>
#include <utility>

std::optional<Config> Config::make(const std::string &file_name) {

  boost::property_tree::ptree root;
  try {
    boost::property_tree::read_json(file_name, root);
  }
  catch (const std::runtime_error &error) {
    // TODO: logs
    return std::nullopt;
  }

  return Config(root);
}

Config::Config(const boost::property_tree::ptree &tree, std::string name)
    : _tree(tree), _key(std::move(name))
{}

Config::Config(const boost::property_tree::ptree::value_type &iterator)
    : _tree(iterator.second), _key(iterator.first)
{}

std::vector<Config> Config::getChildren() const {
  return std::vector<Config>(_tree.begin(), _tree.end());
}

std::string_view Config::getName() const {
  return _key;
}

// clang-format off
bool Config::has_value() const {
// clang-format on
  return !_key.empty();
}
