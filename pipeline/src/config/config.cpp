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

Config::Config(
  const boost::property_tree::ptree &tree,
  const std::optional<std::reference_wrapper<const boost::property_tree::ptree> > global,
  std::string name
)
    : tree_(tree), global_(global), key_(std::move(name))
{}

Config::Config(
  const boost::property_tree::ptree::value_type &iterator,
  std::optional<std::reference_wrapper<const boost::property_tree::ptree> > global
)
    : tree_(iterator.second), global_(global), key_(iterator.first)
{}

std::vector<Config> Config::getChildren() const {
  std::vector<Config> result(tree_.begin(), tree_.end());

  if (global_) {
    for (auto &config : result) {
      config.setGlobal(global_.value());
    }
  }

  return result;
}

std::string_view Config::getName() const {
  return key_;
}

// clang-format off
bool Config::has_value() const {
// clang-format on
  return !key_.empty();
}

void Config::setGlobal(const boost::property_tree::ptree &global) {
  global_ = global;
}
