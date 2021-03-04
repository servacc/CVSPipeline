#pragma once

#include <cvs/pipeline/ipipeline.hpp>

namespace cvs::pipeline::impl {

class Pipeline : public IPipeline {
 public:
  Pipeline();

  int exec() override;
};

}  // namespace cvs::pipeline::impl
