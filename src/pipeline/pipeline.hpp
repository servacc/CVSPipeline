#pragma once

#include <cvs/pipeline/ipipeline.hpp>

namespace cvs::pipeline {

class Pipeline : public IPipeline {
 public:
  Pipeline();

  void init(std::filesystem::path) override;
  int  exec() override;
  void free() override;
};

}  // namespace cvs::pipeline
