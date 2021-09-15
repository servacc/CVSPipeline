#pragma once

#include <cvs/common/config.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/tbb/tbbflowgraph.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

template <typename Tuple, typename Policy = ::tbb::flow::queueing>
class TbbJoinNode;

template <typename Policy, typename... Args>
class TbbJoinNode<std::tuple<Args...>, Policy> : public IInputExecutionNode<NodeType::ServiceIn, Args...>,
                                                 public IOutputExecutionNode<NodeType::ServiceIn, std::tuple<Args...>> {
 public:
  using ArgumentsType = std::tuple<Args...>;

  static_assert(sizeof...(Args) > 1, "The number of arguments must be equal to or more than two.");

  static auto make(const common::Properties&, IExecutionGraphPtr graph, std::shared_ptr<ArgumentsType>) {
    if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph))
      return std::make_unique<TbbJoinNode>(g);
    return std::unique_ptr<TbbJoinNode>{};
  }

  TbbJoinNode(TbbFlowGraphPtr g)
      : node(g->native()) {}

  bool tryPut(const Args&...) override { return false; }
  bool tryGet(ArgumentsType&) override { return false; }

  std::any receiver(std::size_t i) override { return getPort<0, Args...>(i); }
  std::any sender(std::size_t) override { return std::make_any<::tbb::flow::sender<ArgumentsType>*>(&node); }

  bool connect(std::any sender, std::size_t i) override { return connectToPort<0, Args...>(std::move(sender), i); }

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
  bool connectToPort(std::any sender, std::size_t i) {
    if (I == i) {
      if (typeid(::tbb::flow::sender<A0>*) != sender.type())
        return false;

      ::tbb::flow::sender<A0>* s = std::any_cast<::tbb::flow::sender<A0>*>(sender);
      ::tbb::flow::make_edge(*s, ::tbb::flow::input_port<I>(node));

      return true;
    }

    return connectToPort<I + 1, AN...>(std::move(sender), i);
  }

 private:
  ::tbb::flow::join_node<ArgumentsType, Policy> node;
};

}  // namespace cvs::pipeline::tbb
