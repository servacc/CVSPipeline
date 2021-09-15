#pragma once

#include <cvs/pipeline/iexecutiongraph.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/impl/pipeline.hpp>
#include <cvs/pipeline/iview.hpp>

namespace cvs::pipeline::impl {

class GuiPipeline : public Pipeline {
 public:
  static IPipelineUPtr make(const common::Properties &, const cvs::common::FactoryPtr<std::string> &);

  int exec() override;

 protected:
  GuiPipeline() = default;

 protected:
  IViewUPtr view;
};

}  // namespace cvs::pipeline::impl
