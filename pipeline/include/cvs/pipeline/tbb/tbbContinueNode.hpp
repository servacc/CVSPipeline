#pragma once

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/pipeline/dataCounter.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionNode.hpp>
#include <cvs/pipeline/tbb/tbbFlowGraph.hpp>

namespace cvs::pipeline::tbb {

template <typename>
class TbbContinueNodeBase;

template <typename Result>
class TbbContinueNodeBase<IElement<Result()>> : public IInputExecutionNode<NodeType::Functional>,
                                                public IOutputExecutionNode<NodeType::Functional, Result>,
                                                public DataCounter {
 public:
  using ElementResultType = Result;
  using NodeResultType    = Result;

  TbbContinueNodeBase(TbbFlowGraphPtr graph, int number_of_predecessors, IElementPtr<ElementResultType()> element)
      : node(graph->native(),
             number_of_predecessors,
             [this, e = std::move(element)](::tbb::flow::continue_msg) -> Result {
               try {
                 LOG_TRACE(IExecutionNode::logger(), "Begin processing continue node {}", IExecutionNode::info.name);
                 beforeProcessing();
                 auto result = e->process();
                 afterProcessing();
                 LOG_TRACE(IExecutionNode::logger(), "End processing continue node {}", IExecutionNode::info.name);
                 return result;
               }
               catch (...) {
                 cvs::common::throwWithNested<std::runtime_error>("Exception in {}", IExecutionNode::info.name);
               }

               throw std::runtime_error(R"(Someone removed "return" from the method body of ContinueNode.)");
             }) {}

  bool tryGet(Result& val) override { return node.try_get(val); }

 protected:
  ::tbb::flow::continue_node<NodeResultType> node;
};

template <>
class TbbContinueNodeBase<IElement<void()>> : public IInputExecutionNode<NodeType::Functional>,
                                              public IOutputExecutionNode<NodeType::Functional>,
                                              public DataCounter {
 public:
  using ElementResultType = void;
  using NodeResultType    = ::tbb::flow::continue_msg;

  TbbContinueNodeBase(TbbFlowGraphPtr graph, int number_of_predecessors, IElementPtr<ElementResultType()> element)
      : node(graph->native(),
             number_of_predecessors,
             [this, e = std::move(element)](::tbb::flow::continue_msg) -> NodeResultType {
               try {
                 beforeProcessing();
                 e->process();
                 afterProcessing();
                 return NodeResultType{};
               }
               catch (...) {
                 cvs::common::throwWithNested<std::runtime_error>("Exception in {}", IExecutionNode::info.name);
               }

               throw std::runtime_error(R"(Someone removed "return" from the method body of ContinueNode.)");
             }) {}

  bool tryGet() override {
    ::tbb::flow::continue_msg msg;
    return node.try_get(msg);
  }

 protected:
  ::tbb::flow::continue_node<NodeResultType> node;
};

}  // namespace cvs::pipeline::tbb

namespace cvs::pipeline::tbb {

template <typename Element>
class TbbContinueNode : public TbbContinueNodeBase<Element> {
 public:
  using ElementResultType = typename TbbContinueNodeBase<Element>::ElementResultType;
  using NodeResultType    = typename TbbContinueNodeBase<Element>::NodeResultType;

  static auto make(const common::Properties&              properties,
                   IExecutionGraphPtr                     graph,
                   const common::FactoryPtr<std::string>& factory,
                   IElementPtr<ElementResultType()>       element) {
    int number_of_predecessors = 0;
    if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph)) {
      auto node = std::make_unique<TbbContinueNode>(g, number_of_predecessors, std::move(element));
      DataCounter::init(*node, properties, factory);
      return node;
    }
    return std::unique_ptr<TbbContinueNode>{};
  }

  TbbContinueNode(TbbFlowGraphPtr graph, int number_of_predecessors, IElementPtr<ElementResultType()> element)
      : TbbContinueNodeBase<Element>(std::move(graph), number_of_predecessors, std::move(element)) {}

  bool tryPut() override { return TbbContinueNodeBase<Element>::node.try_put(::tbb::flow::continue_msg{}); }

  std::any receiver(std::size_t i) override {
    return i == 0 ? std::make_any<::tbb::flow::receiver<::tbb::flow::continue_msg>*>(&this->node) : std::any{};
  }
  std::any sender(std::size_t i) override {
    return i == 0 ? std::make_any<::tbb::flow::sender<NodeResultType>*>(&this->node) : std::any{};
  }

  bool connect(std::any sender, std::size_t i) override {
    if (i == 0) {
      if (typeid(::tbb::flow::sender<::tbb::flow::continue_msg>*) == sender.type()) {
        ::tbb::flow::sender<::tbb::flow::continue_msg>* s =
            std::any_cast<::tbb::flow::sender<::tbb::flow::continue_msg>*>(sender);
        ::tbb::flow::make_edge(*s, TbbContinueNodeBase<Element>::node);
        return true;
      }
    }
    return false;
  }
};

}  // namespace cvs::pipeline::tbb
