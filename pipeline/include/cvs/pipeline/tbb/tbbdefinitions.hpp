#pragma once

#include <string>

namespace cvs::pipeline::tbb {

struct TbbDefaultName {
  static const std::string graph;

  static const std::string broadcast;
  static const std::string continue_name;
  static const std::string function;
  static const std::string join;
  static const std::string source;
  static const std::string split_name;
};

}  // namespace cvs::pipeline::tbb
