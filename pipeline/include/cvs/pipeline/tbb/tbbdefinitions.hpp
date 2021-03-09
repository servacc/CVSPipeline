#pragma once

#include <string>

namespace cvs::pipeline::tbb {

struct TbbDefaultName {
  static const std::string graph_name;

  static const std::string broadcast_name;
  static const std::string continue_name;
  static const std::string function_name;
  static const std::string join_name;
  static const std::string source_name;
  static const std::string split_name;
};

}  // namespace cvs::pipeline::tbb
