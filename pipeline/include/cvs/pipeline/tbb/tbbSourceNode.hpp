#pragma once

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/pipeline/dataCounter.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionNode.hpp>
#include <cvs/pipeline/tbb/tbbFlowGraph.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

template <typename Element>
class TbbSourceNode;

template <typename Output>
class TbbSourceNode<IElement<Output()>> : public ISourceExecutionNode<NodeType::Functional>,
                                          public IOutputExecutionNode<NodeType::Functional, Output>,
                                          public DataCounter {
 public:
  static auto make(const common::Properties&              properties,
                   IExecutionGraphPtr                     graph,
                   const common::FactoryPtr<std::string>& factory,
                   IElementPtr<Output()>                  element) {
    if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph)) {
      auto node = std::make_unique<TbbSourceNode>(std::move(g), std::move(element));
      DataCounter::init(*node, properties, factory);
      return node;
    }
    return std::unique_ptr<TbbSourceNode>{};
  }

  TbbSourceNode(TbbFlowGraphPtr graph, IElementPtr<Output()> element)
      : node(graph->native(), std::function([this, element](::tbb::flow_control& fc) -> Output {
               beforeProcessing();
               auto res = element->process();
               afterProcessing();
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
