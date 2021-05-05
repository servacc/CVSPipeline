#pragma once

#include <cvs/common/config.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/tbb/tbbflowgraph.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

template <typename Element>
class TbbSplitNode;

template <typename... RX>
class TbbSplitNode<std::tuple<RX...>> : public IInputExecutionNode<NodeType::ServiceOut, std::tuple<RX...>>,
                                        public IOutputExecutionNode<NodeType::ServiceOut, RX...> {
 public:
  using ResultType = std::tuple<RX...>;

  static_assert(sizeof...(RX) > 1, "The number of arguments must be equal to or more than two.");

  static auto make(common::Config&, IExecutionGraphPtr graph, std::shared_ptr<ResultType>) {
    if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph))
      return std::make_unique<TbbSplitNode>(g);
    return std::unique_ptr<TbbSplitNode>{};
  }

  TbbSplitNode(TbbFlowGraphPtr graph)
      : node(graph->native()) {}

  bool tryPut(const ResultType&) override { return false; }
  bool tryGet(RX&...) override { return false; }

  std::any receiver(std::size_t i) override {
    return i == 0 ? std::make_any<::tbb::flow::receiver<ResultType>*>(&node) : std::any{};
  }
  std::any sender(std::size_t i) override { return getPort<0, RX...>(i); }

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
