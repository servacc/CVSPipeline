#pragma once

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/loggable.hpp>
#include <cvs/pipeline/iexecutionGraph.hpp>
#include <cvs/pipeline/iexecutionNode.hpp>
#include <cvs/pipeline/ipipeline.hpp>

#include <map>

namespace cvs::pipeline::impl {

class Pipeline : public IPipeline, public cvs::logger::Loggable<Pipeline> {
 public:
  using NodesMap = std::map<std::string, IExecutionNodePtr>;

  static IPipelineUPtr make(const common::Properties &, const cvs::common::FactoryPtr<std::string> &);

  IExecutionNodePtr getNode(std::string_view) const override;

  int exec() override;

  virtual void onStarted();
  virtual void onStopped();

  void waitForAll() override;

 protected:
  static IExecutionGraphPtr parseGraph(const common::Properties &, const cvs::common::FactoryPtr<std::string> &);
  static NodesMap           parseNodes(const common::Properties &,
                                       const cvs::common::FactoryPtr<std::string> &,
                                       IExecutionGraphPtr &);
  static void               parseConnections(const common::Properties &, const NodesMap &);
  static void initPipeline(Pipeline &, const common::Properties &, const cvs::common::FactoryPtr<std::string> &);

  Pipeline();

 protected:
  IExecutionGraphPtr graph;
  bool               autostart = false;

  NodesMap nodes;
};

}  // namespace cvs::pipeline::impl
