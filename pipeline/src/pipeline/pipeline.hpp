#pragma once

#include <cvs/pipeline/ipipeline.hpp>

namespace cvs::pipeline::impl {

class Pipeline : public IPipeline {
 public:
  Pipeline();

  void init(common::ConfigurationPtr) override;
  int  exec() override;
  void free() override;
};

}  // namespace cvs::pipeline::impl
