#pragma once

#include <cvs/common/config.hpp>
#include <cvs/common/configbase.hpp>
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
  static type forward(const Args&... args) { return std::tie(args...); }
};

template <typename Arg>
struct Input<Arg> {
  using type = Arg;
  static const type& forward(const type& arg) { return arg; }
};

template <>
struct Input<void> {
  using type = ::tbb::flow::continue_msg;
  static type forward() { return {}; }
};

template <typename Res>
struct Output {
  using type = Res;
};

template <>
struct Output<void> {
  using type = ::tbb::flow::continue_msg;
};

CVSCFG_DECLARE_CONFIG(FunctionNodeConfig, CVSCFG_VALUE_DEFAULT(concurrency, std::size_t, 0))
}  // namespace detail

template <typename Element, typename Policy = ::tbb::flow::queueing>
class TbbFunctionNodeBase;

template <typename... Args, typename Policy>
class TbbFunctionNodeBase<IElement<void(Args...)>, Policy>
    : public IInputExecutionNode<NodeType::Functional, typename detail::Input<Args...>::type>,
      public IOutputExecutionNode<NodeType::Functional> {
 public:
  using ElementPtrType = IElementPtr<void(Args...)>;
  using ElementType    = IElement<void(Args...)>;
  using ResultType     = typename detail::Output<void>::type;
  using ArgumentsType  = typename detail::Input<Args...>::type;

  TbbFunctionNodeBase(TbbFlowGraphPtr graph, std::size_t concurrency, IElementPtr<void(Args...)> element)
      : node(graph->native(), concurrency, createExecuteFunction1(std::move(element))) {}

  bool tryGet() override {
    ::tbb::flow::continue_msg msg;
    return node.try_get(msg);
  }
  bool tryPut(const Args&... args) override { return node.try_put(detail::Input<Args>::forward(args)...); }

 protected:
  auto createExecuteFunction1(IElementPtr<void(Args...)> element) {
    if constexpr (1 < std::tuple_size_v<std::tuple<Args...>>) {
      return [e = std::move(element)](std::tuple<Args...> a) -> ::tbb::flow::continue_msg {
        std::apply(&IElement<void(Args...)>::process, std::tuple_cat(std::make_tuple(e), a));
        return ::tbb::flow::continue_msg{};
      };
    } else {
      return [e = std::move(element)](Args... a) -> ::tbb::flow::continue_msg {
        e->process(a...);
        return ::tbb::flow::continue_msg{};
      };
    }
  }

  ::tbb::flow::function_node<ArgumentsType, ResultType, Policy>* nodePtr() { return &node; }

 protected:
  ::tbb::flow::function_node<ArgumentsType, ResultType, Policy> node;
};

template <typename Result, typename... Args, typename Policy>
class TbbFunctionNodeBase<IElement<Result(Args...)>, Policy>
    : public IInputExecutionNode<NodeType::Functional, Args...>,
      public IOutputExecutionNode<NodeType::Functional, Result> {
 public:
  using ElementPtrType = IElementPtr<Result(Args...)>;
  using ElementType    = IElement<Result(Args...)>;
  using ResultType     = typename detail::Output<Result>::type;
  using ArgumentsType  = typename detail::Input<Args...>::type;

  TbbFunctionNodeBase(TbbFlowGraphPtr graph, std::size_t concurrency, ElementPtrType element)
      : node(graph->native(), concurrency, createExecuteFunction1(std::move(element))) {}

  bool tryGet(Result& val) override { return node.try_get(val); }

  bool tryPut(const Args&... args) override { return node.try_put(detail::Input<Args...>::forward(args...)); }

 protected:
  auto createExecuteFunction1(ElementPtrType element) {
    if constexpr (1 < std::tuple_size_v<std::tuple<Args...>>) {
      return [e = std::move(element)](std::tuple<Args...> a) -> Result {
        return std::apply(&ElementType::process, std::tuple_cat(std::make_tuple(e), a));
      };
    } else {
      return [e = std::move(element)](Args... a) -> Result { return e->process(a...); };
    }
  }

  ::tbb::flow::function_node<ArgumentsType, Result, Policy>* nodePtr() { return &node; }

 protected:
  ::tbb::flow::function_node<ArgumentsType, Result, Policy> node;
};

template <typename Element, typename Policy = ::tbb::flow::queueing>
class TbbFunctionNode : public TbbFunctionNodeBase<Element, Policy> {
 public:
  using ElementPtrType = typename TbbFunctionNodeBase<Element, Policy>::ElementPtrType;
  using ElementType    = typename TbbFunctionNodeBase<Element, Policy>::ElementType;
  using ResultType     = typename TbbFunctionNodeBase<Element, Policy>::ResultType;
  using ArgumentsType  = typename TbbFunctionNodeBase<Element, Policy>::ArgumentsType;

  static auto make(common::Config& cfg, IExecutionGraphPtr graph, ElementPtrType body) {
    auto params = cfg.parse<detail::FunctionNodeConfig>().value();

    if (auto g = std::dynamic_pointer_cast<cvs::pipeline::tbb::TbbFlowGraph>(graph))
      return std::make_unique<TbbFunctionNode>(g, params.concurrency, std::move(body));
    return std::unique_ptr<TbbFunctionNode>{};
  }

  TbbFunctionNode(TbbFlowGraphPtr graph, std::size_t concurrency, ElementPtrType element)
      : TbbFunctionNodeBase<Element, Policy>(std::move(graph), concurrency, std::move(element)) {}

  std::any receiver(std::size_t) override {
    return std::make_any<::tbb::flow::receiver<ArgumentsType>*>(TbbFunctionNodeBase<Element, Policy>::nodePtr());
  }
  std::any sender(std::size_t) override {
    return std::make_any<::tbb::flow::sender<ResultType>*>(TbbFunctionNodeBase<Element, Policy>::nodePtr());
  }

  bool connect(std::any sndr, std::size_t) override {
    ::tbb::flow::sender<ArgumentsType>* s = std::any_cast<::tbb::flow::sender<ArgumentsType>*>(sndr);
    ::tbb::flow::make_edge(*s, TbbFunctionNodeBase<Element, Policy>::node);
    return true;
  }
};

}  // namespace cvs::pipeline::tbb
