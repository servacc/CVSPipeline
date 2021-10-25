#pragma once

#include <cvs/pipeline/iexecutionNode.hpp>
#include <cvs/pipeline/tbb/tbbFlowGraph.hpp>

namespace cvs::pipeline::tbb {

template <NodeType Type, template <typename> class TbbNodeType, typename T>
class TbbBufferTemplateNode : public IInputExecutionNode<Type, T>, public IOutputExecutionNode<Type, T> {
 public:
  TbbBufferTemplateNode(TbbFlowGraphPtr graph)
      : node(graph->native()) {}

  bool tryGet(T& val) override { return node.try_get(val); }
  bool tryPut(const T& val) override { return node.try_put(val); }

  std::any receiver(std::size_t i) override {
    return i == 0 ? std::make_any<::tbb::flow::receiver<T>*>(&node) : std::any{};
  }
  std::any sender(std::size_t i) override {
    return i == 0 ? std::make_any<::tbb::flow::sender<T>*>(&node) : std::any{};
  }

  bool connect(std::any sender, std::size_t i) override {
    if (i == 0) {
      if (typeid(::tbb::flow::sender<T>*) == sender.type()) {
        ::tbb::flow::sender<T>* s = std::any_cast<::tbb::flow::sender<T>*>(sender);
        ::tbb::flow::make_edge(*s, node);
        return true;
      }
    }
    return false;
  }

 protected:
  TbbNodeType<T> node;
};

}  // namespace cvs::pipeline::tbb
