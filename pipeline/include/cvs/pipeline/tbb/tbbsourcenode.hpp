#pragma once

#include <cvs/common/configuration.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/tbb/tbbflowgraph.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

template <typename Element>
class TbbSourceNode;

template <typename Output>
class TbbSourceNode<IElement<Output(bool*)>> : public ISourceExecutionNode, public IOutputExecutionNode<Output> {
 public:
  static auto make(common::Configuration, IExecutionGraphPtr graph, IElementPtr<Output(bool*)> element) {
    if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph))
      return std::make_unique<TbbSourceNode>(std::move(g), std::move(element));
    return std::unique_ptr<TbbSourceNode>{};
  }

  TbbSourceNode(TbbFlowGraphPtr graph, IElementPtr<Output(bool*)> element)
      : node(graph->native(), std::function([element](::tbb::flow_control& fc) -> Output {
               bool stop = false;
               auto res  = element->process(&stop);
               if (stop)
                 fc.stop();
               return std::move(res);
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
