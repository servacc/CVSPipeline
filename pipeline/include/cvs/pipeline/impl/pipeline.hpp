#pragma once

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/loggable.hpp>
#include <cvs/pipeline/iexecutiongraph.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/ipipeline.hpp>

#include <map>

namespace cvs::pipeline::impl {

class Pipeline : public IPipeline, public cvs::logger::Loggable<Pipeline> {
 public:
  static IPipelineUPtr make(cvs::common::Config&, const cvs::common::FactoryPtr<std::string>&);

  Pipeline();

  IExecutionNodePtr getNode(std::string_view) const override;

  int exec() override;

  virtual void onStarted();
  virtual void onStopped();

  void waitForAll() override;

 private:
  IExecutionGraphPtr graph;
  bool               autostart = false;

  std::map<std::string, IExecutionNodePtr> nodes;
};

}  // namespace cvs::pipeline::impl
