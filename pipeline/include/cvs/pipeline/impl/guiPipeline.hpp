#pragma once

#include <cvs/pipeline/iexecutiongraph.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/impl/pipeline.hpp>
#include <cvs/pipeline/iview.hpp>

#include <map>

// namespace cvs::pipeline {

// template <typename, typename>
// class IGuiExecutionNode;

// template <typename... In, typename... Out>
// class IGuiExecutionNode<std::tuple<In...>, std::tuple<Out...>> : public virtual IExecutionNode {
// public:
//  virtual int exec() = 0;

//  virtual void onRcvData() = 0;
//  virtual void sendData()  = 0;

//  virtual void onEvent() = 0;
//};

// template <typename... T>
// using IGuiExecutionNodePtr = std::shared_ptr<IGuiExecutionNode<T...>>;
// template <typename... T>
// using IGuiExecutionNodeUPtr = std::unique_ptr<IGuiExecutionNode<T...>>;

//}  // namespace cvs::pipeline

namespace cvs::pipeline::impl {

class GuiPipeline : public Pipeline {
 public:
  static IPipelineUPtr make(cvs::common::Config &, const cvs::common::FactoryPtr<std::string> &);

  int exec() override;

 protected:
  GuiPipeline() = default;

 protected:
  IViewUPtr view;
};

}  // namespace cvs::pipeline::impl
