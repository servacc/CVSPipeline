#pragma once

#include <cvs/common/configuration.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/tbb/tbbflowgraph.hpp>

namespace cvs::pipeline::tbb {

template <typename>
class TbbContinueNodeBase;

template <typename Result>
class TbbContinueNodeBase<IElement<Result()>> : public IExecutionNode {
 public:
  using ElementResultType = Result;
  using NodeResultType    = Result;

 protected:
  auto createBodyFunction(IElementPtr<Result()> element) {
    return [e = std::move(element)](::tbb::flow::continue_msg) -> Result { return e->process(); };
  }
};

template <>
class TbbContinueNodeBase<IElement<void()>> : public IExecutionNode {
 public:
  using ElementResultType = void;
  using NodeResultType    = ::tbb::flow::continue_msg;

 protected:
  auto createBodyFunction(IElementPtr<void()> element) {
    return [e = std::move(element)](::tbb::flow::continue_msg) -> NodeResultType {
      e->process();
      return NodeResultType{};
    };
  }
};

}  // namespace cvs::pipeline::tbb

namespace cvs::pipeline::tbb {

template <typename Element>
class TbbContinueNode : public TbbContinueNodeBase<Element> {
 public:
  using ElementResultType = typename TbbContinueNodeBase<Element>::ElementResultType;
  using NodeResultType    = typename TbbContinueNodeBase<Element>::NodeResultType;

  static auto make(common::Configuration, IExecutionGraphPtr graph, IElementPtr<ElementResultType()> element) {
    int number_of_predecessors = 0;
    if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph))
      return std::make_unique<TbbContinueNode>(g, number_of_predecessors, std::move(element));
    return std::unique_ptr<TbbContinueNode>{};
  }

  TbbContinueNode(TbbFlowGraphPtr graph, int number_of_predecessors, IElementPtr<ElementResultType()> element)
      : node(graph->native(),
             number_of_predecessors,
             TbbContinueNodeBase<Element>::createBodyFunction(std::move(element))) {}

  std::any receiver(std::size_t i) override {
    return i == 0 ? std::make_any<::tbb::flow::receiver<::tbb::flow::continue_msg>*>(&node) : std::any{};
  }
  std::any sender(std::size_t i) override {
    return i == 0 ? std::make_any<::tbb::flow::sender<NodeResultType>*>(&node) : std::any{};
  }

  bool connect(std::any sndr, std::size_t i) override {
    if (i == 0) {
      ::tbb::flow::sender<::tbb::flow::continue_msg>* s =
          std::any_cast<::tbb::flow::sender<::tbb::flow::continue_msg>*>(sndr);
      ::tbb::flow::make_edge(*s, node);
      return true;
    }
    return false;
  }

 private:
  ::tbb::flow::continue_node<NodeResultType> node;
};

}  // namespace cvs::pipeline::tbb
