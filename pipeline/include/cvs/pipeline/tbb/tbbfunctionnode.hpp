#pragma once

#include <cvs/common/configuration.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/tbb/tbbflowgraph.hpp>
#include <tbb/flow_graph.h>

// TODO: void(Args...) && Res() => ::tbb::flow::continue_msg(Args...) && Res(::tbb::flow::continue_msg) ????

namespace cvs::pipeline::tbb {

namespace detail {

template <typename... Args>
struct Input {
  using type = std::tuple<Args...>;
};

template <typename Arg>
struct Input<Arg> {
  using type = Arg;
};

template <typename Res>
struct Output {
  using type = Res;
};

template <>
struct Output<void> {
  using type = ::tbb::flow::continue_msg;
};

}  // namespace detail
/*
template <typename>
class ITbbFunctionNode;

template <typename Result, typename Arg>
class ITbbFunctionNode<IElement<Result(Arg)>> : public IExecutionNode {
 public:
  using ElementType   = IElementPtr<Result(Arg)>;
  using ResultType    = Result;
  using ArgumentsType = Arg;

 protected:
  auto createExecuteFunction(ElementType element, typename std::enable_if<std::is_same_v<Result, void>>::type* = 0) {
    return [element](ArgumentsType a) -> void { element->process(std::forward<Arg>(a)); };
  }

  auto createExecuteFunction(ElementType element, typename std::enable_if<!std::is_same_v<Result, void>>::type* = 0) {
    return [element](ArgumentsType a) -> Result { return element->process(std::forward<Arg>(a)); };
  }
};

//*
template <typename... Args>
class ITbbFunctionNode<IElement<void(Args...)>> : public IExecutionNode {
 public:
  using ElementType   = IElementPtr<void(Args...)>;
  using ResultType    = void;
  using ArgumentsType = std::tuple<Args...>;

 protected:
  auto createExecuteFunction(ElementType element) {
    return [element](ArgumentsType a) {
      std::apply(&ElementType::process, std::tuple_cat(std::make_tuple(element), a));
      return ::tbb::flow::continue_msg{};
    };
  }
};

template <typename Arg>
class ITbbFunctionNode<IElement<void(Arg)>> : public IExecutionNode {
 public:
  using ElementType   = IElementPtr<void(Arg)>;
  using ResultType    = void;
  using ArgumentsType = Arg;

 protected:
  auto createExecuteFunction(ElementType element) {
    return [element](ArgumentsType a) {
      element->process(a);
      return ::tbb::flow::continue_msg{};
    };
  }
};
//* /

template <typename Result, typename... Args>
class ITbbFunctionNode<IElement<Result(Args...)>> : public IExecutionNode {
 public:
  using ElementType   = IElementPtr<Result(Args...)>;
  using ResultType    = Result;
  using ArgumentsType = std::tuple<Args...>;

 protected:
  auto createExecuteFunction(ElementType element) {
    return [element](ArgumentsType a) -> Result {
      return std::apply(&ElementType::process, std::tuple_cat(std::make_tuple(element), a));
    };
  }
};
//*/
}  // namespace cvs::pipeline::tbb

namespace cvs::pipeline::tbb {

template <typename Element, typename Policy = ::tbb::flow::queueing>
class TbbFunctionNode;

template <typename Result, typename... Args, typename Policy>
class TbbFunctionNode<IElement<Result(Args...)>, Policy> : public IExecutionNode {
 public:
  using ElementType   = IElementPtr<Result(Args...)>;
  using ResultType    = typename detail::Output<Result>::type;
  using ArgumentsType = typename detail::Input<Args...>::type;

  static auto make(common::Configuration cfg, IExecutionGraphPtr graph, ElementType body) {
    std::size_t concurrency = 0;

    if (auto g = std::dynamic_pointer_cast<cvs::pipeline::tbb::TbbFlowGraph>(graph))
      return std::make_unique<TbbFunctionNode>(g, concurrency, std::move(body));
    return std::unique_ptr<TbbFunctionNode>{};
  }

  TbbFunctionNode(TbbFlowGraphPtr graph, std::size_t concurrency, ElementType element)
      : node(graph->native(), concurrency, createExecuteFunction1(std::move(element))) {}

  template <typename Body>
  TbbFunctionNode(TbbFlowGraphPtr graph, std::size_t concurrency, Body body)
      : node(graph->native(), concurrency, std::move(body)) {}

  std::any receiver(std::size_t) override { return std::make_any<::tbb::flow::receiver<ArgumentsType>*>(&node); }
  std::any sender(std::size_t) override { return std::make_any<::tbb::flow::sender<ResultType>*>(&node); }

  bool connect(std::any sndr, std::size_t) override {
    ::tbb::flow::sender<ArgumentsType>* s = std::any_cast<::tbb::flow::sender<ArgumentsType>*>(sndr);
    ::tbb::flow::make_edge(*s, node);
    return true;
  }

 protected:
  auto createExecuteFunction1(ElementType element) {
    if constexpr (std::is_same_v<ResultType, ::tbb::flow::continue_msg>) {
      if constexpr (std::tuple_size_v<std::tuple<Args...>> >= 2) {
        return [e = std::move(element)](ArgumentsType a) -> ResultType {
          std::apply(&IElement<Result(Args...)>::process, std::tuple_cat(std::make_tuple(e), a));
          return ResultType{};
        };
      } else {
        return [e = std::move(element)](ArgumentsType a) -> ResultType {
          e->process(a);
          return ResultType{};
        };
      }
    } else {
      if constexpr (std::tuple_size_v<std::tuple<Args...>> >= 2) {
        return [e = std::move(element)](ArgumentsType a) -> ResultType {
          return std::apply(&IElement<Result(Args...)>::process, std::tuple_cat(std::make_tuple(e), a));
        };
      } else {
        return [e = std::move(element)](ArgumentsType a) -> ResultType { return e->process(a); };
      }
    }
  }

 private:
  ::tbb::flow::function_node<ArgumentsType, ResultType, Policy> node;
};

}  // namespace cvs::pipeline::tbb

/*
namespace cvs::pipeline::tbb::_ {

template <typename Element, typename Policy = ::tbb::flow::queueing>
class TbbFunctionNode : public ITbbFunctionNode<Element> {
 public:
  using ElementType   = typename ITbbFunctionNode<Element>::ElementType;
  using ResultType    = typename ITbbFunctionNode<Element>::ResultType;
  using ArgumentsType = typename ITbbFunctionNode<Element>::ArgumentsType;

  static auto make(common::Configuration cfg, IExecutionGraphPtr graph, ElementType body) {
    std::size_t concurrency = 0;

    if (auto g = std::dynamic_pointer_cast<cvs::pipeline::tbb::TbbFlowGraph>(graph))
      return std::make_unique<TbbFunctionNode>(g, concurrency, std::move(body));
    return std::unique_ptr<TbbFunctionNode>{};
  }

  TbbFunctionNode(TbbFlowGraphPtr graph, std::size_t concurrency, ElementType element)
      : node(graph->native(), concurrency, ITbbFunctionNode<Element>::createExecuteFunction(std::move(element))) {}

  template <typename Body>
  TbbFunctionNode(TbbFlowGraphPtr graph, std::size_t concurrency, Body body)
      : node(graph->native(), concurrency, std::move(body)) {}

  std::any receiver(std::size_t) override { return std::make_any<::tbb::flow::receiver<ArgumentsType>*>(&node); }
  std::any sender(std::size_t) override { return std::make_any<::tbb::flow::sender<ResultType>*>(&node); }

  bool connect(std::any sndr, std::size_t) override {
    ::tbb::flow::sender<ArgumentsType>* s = std::any_cast<::tbb::flow::sender<ArgumentsType>*>(sndr);
    ::tbb::flow::make_edge(*s, node);
    return true;
  }

 private:
  ::tbb::flow::function_node<ArgumentsType, ResultType, Policy> node;
};

}  // namespace cvs::pipeline::tbb::_
//*/
