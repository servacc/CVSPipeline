#pragma once

#include <cvs/common/config.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/tbb/tbbflowgraph.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

template <NodeType Type, typename T>
class TbbBufferNode : public IInputExecutionNode<Type, T>, public IOutputExecutionNode<Type, T> {
 public:
  static auto make(common::Config&, IExecutionGraphPtr graph, std::shared_ptr<T>) {
    if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph))
      return std::make_unique<TbbBufferNode>(g);
    return std::unique_ptr<TbbBufferNode>{};
  }

  TbbBufferNode(TbbFlowGraphPtr graph)
      : node(graph->native()) {}

  bool tryGet(T& val) override { return node.try_get(val); }
  bool tryPut(const T& val) override { return node.try_put(val); }

  std::any receiver(std::size_t i) override {
    return i == 0 ? std::make_any<::tbb::flow::receiver<T>*>(&node) : std::any{};
  }
  std::any sender(std::size_t i) override {
    return i == 0 ? std::make_any<::tbb::flow::sender<T>*>(&node) : std::any{};
  }

  bool connect(std::any sndr, std::size_t i) override {
    if (i == 0) {
      ::tbb::flow::sender<T>* s = std::any_cast<::tbb::flow::sender<T>*>(sndr);
      ::tbb::flow::make_edge(*s, node);
      return true;
    }
    return false;
  }

 private:
  ::tbb::flow::buffer_node<T> node;
};

template <typename T>
using TbbBufferNodeIn = TbbBufferNode<NodeType::ServiceIn, T>;
template <typename T>
using TbbBufferNodeOut = TbbBufferNode<NodeType::ServiceOut, T>;

}  // namespace cvs::pipeline::tbb
