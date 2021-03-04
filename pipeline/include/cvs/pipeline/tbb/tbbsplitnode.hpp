#pragma once

#include <cvs/common/configuration.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/tbb/tbbflowgraph.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

template <typename Element>
class TbbSplitNode;

template <typename... Result, typename... Args>
class TbbSplitNode<IElement<std::tuple<Result...>(Args...)>> : public IExecutionNode {
 public:
  using ResultType = std::tuple<Result...>;

  auto make(common::Configuration, IExecutionGraphPtr graph, IElementPtr<std::tuple<Result...>(Args...)>) {
    if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph))
      return std::make_unique<TbbSplitNode>(g);
    return std::unique_ptr<TbbSplitNode>{};
  }

  TbbSplitNode(TbbFlowGraphPtr) {}

  std::any receiver(std::size_t i) override {
    return i == 0 ? std::make_any<::tbb::flow::receiver<ResultType>*>(&node) : std::any{};
  }
  std::any sender(std::size_t i) override { return getPort<0, Result...>(i); }

  bool connect(std::any sndr, std::size_t i) override {
    if (i == 0) {
      ::tbb::flow::sender<ResultType>* s = std::any_cast<::tbb::flow::sender<ResultType>*>(sndr);
      ::tbb::flow::make_edge(*s, node);
      return true;
    }
    return false;
  }

 private:
  template <std::size_t I>
  std::any getPort(std::size_t) {
    return {};
  }

  template <std::size_t I, typename A0, typename... AN>
  std::any getPort(std::size_t i) {
    if (I == i)
      return std::make_any<::tbb::flow::sender<A0>*>(&::tbb::flow::output_port<I>(node));
    return getPort<I + 1, AN...>(i);
  }

 private:
  ::tbb::flow::split_node<ResultType> node;
};

}  // namespace cvs::pipeline::tbb
