#pragma once

#include <cvs/common/configuration.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/tbb/tbbflowgraph.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

template <typename Element, typename Policy = ::tbb::flow::queueing>
class TbbJoinNode;

template <typename Policy, typename Result, typename Arg0, typename Arg1, typename... Args>
class TbbJoinNode<IElement<Result(Arg0, Arg1, Args...)>, Policy> : public IExecutionNode {
 public:
  using ArgumentsType = std::tuple<Arg0, Arg1, Args...>;

  static auto make(common::Configuration, IExecutionGraphPtr graph, IElementPtr<Result(Arg0, Arg1, Args...)>) {
    if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph))
      return std::make_unique<TbbJoinNode>(g);
    return std::unique_ptr<TbbJoinNode>{};
  }

  TbbJoinNode(TbbFlowGraphPtr g)
      : node(g->native()) {}

  std::any receiver(std::size_t i) override { return getPort<0, Arg0, Arg1, Args...>(i); }
  std::any sender(std::size_t) override { return std::make_any<::tbb::flow::sender<ArgumentsType>*>(&node); }

  bool connect(std::any sndr, std::size_t i) override { return connectToPort<0, Arg0, Arg1, Args...>(sndr, i); }

 private:
  template <std::size_t I>
  std::any getPort(std::size_t) {
    return {};
  }

  template <std::size_t I, typename A0, typename... AN>
  std::any getPort(std::size_t i) {
    if (I == i)
      return std::make_any<::tbb::flow::receiver<A0>*>(&::tbb::flow::input_port<I>(node));
    return getPort<I + 1, AN...>(i);
  }

  template <std::size_t I>
  bool connectToPort(std::any, std::size_t) {
    return false;
  }

  template <std::size_t I, typename A0, typename... AN>
  bool connectToPort(std::any sndr, std::size_t i) {
    if (I == i) {
      ::tbb::flow::sender<A0>* s = std::any_cast<::tbb::flow::sender<A0>*>(sndr);
      ::tbb::flow::make_edge(*s, ::tbb::flow::input_port<I>(node));

      return true;
    }

    return connectToPort<I + 1, AN...>(sndr, i);
  }

 private:
  ::tbb::flow::join_node<ArgumentsType> node;
};

}  // namespace cvs::pipeline::tbb
