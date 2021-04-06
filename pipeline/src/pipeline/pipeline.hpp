#pragma once

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/pipeline/iexecutiongraph.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/ipipeline.hpp>

#include <map>

namespace cvs::pipeline::impl {

class Pipeline : public IPipeline {
 public:
  static IPipelineUPtr make(cvs::common::Config&, const cvs::common::FactoryPtr<std::string>&);

  Pipeline();

  IExecutionNodePtr getNode(std::string_view) const override;

  void waitForAll() override;

 private:
  IExecutionGraphPtr graph;

  std::map<std::string, IExecutionNodePtr> nodes;
};

}  // namespace cvs::pipeline::impl
