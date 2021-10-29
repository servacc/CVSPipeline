#pragma once

#include <string>

namespace cvs::pipeline::tbb {

struct TbbDefaultName {
  static const std::string graph;

  static const std::string overwrite_in;
  static const std::string overwrite_out;
  static const std::string buffer_in;
  static const std::string buffer_out;
  static const std::string broadcast_in;
  static const std::string broadcast_out;
  static const std::string continue_name;
  static const std::string function;
  static const std::string multifunction;
  static const std::string join;
  static const std::string source;
  static const std::string split;
  static const std::string queue_in;
  static const std::string queue_out;
};

}  // namespace cvs::pipeline::tbb
