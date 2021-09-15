#pragma once

#include <cvs/common/config.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/tbb/tbbflowgraph.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

template <typename Element>
class TbbSourceNode;

template <typename Output>
class TbbSourceNode<IElement<Output()>> : public ISourceExecutionNode<NodeType::Functional>,
                                          public IOutputExecutionNode<NodeType::Functional, Output> {
 public:
  static auto make(const common::Properties&, IExecutionGraphPtr graph, IElementPtr<Output()> element) {
    if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph))
      return std::make_unique<TbbSourceNode>(std::move(g), std::move(element));
    return std::unique_ptr<TbbSourceNode>{};
  }

  TbbSourceNode(TbbFlowGraphPtr graph, IElementPtr<Output()> element)
      : node(graph->native(), std::function([element](::tbb::flow_control& fc) -> Output {
               auto res = element->process();
               if (element->isStopped())
                 fc.stop();
               return res;
             })) {}

  void activate() override { node.activate(); }

  bool tryGet(Output& val) override { return node.try_get(val); }

  std::any receiver(std::size_t) override { return std::any{}; }
  std::any sender(std::size_t i) override {
    return i == 0 ? std::make_any<::tbb::flow::sender<Output>*>(&node) : std::any{};
  }

  bool connect(std::any, std::size_t) override { return false; }

 private:
  ::tbb::flow::input_node<Output> node;
};

}  // namespace cvs::pipeline::tbb
