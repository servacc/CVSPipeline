#pragma once

#include <cvs/common/config.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionNode.hpp>
#include <cvs/pipeline/tbb/tbbFlowGraph.hpp>
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

}  // namespace detail

template <typename Element, typename Policy = ::tbb::flow::queueing>
class TbbFunctionNodeBase;

template <typename... Args, typename Policy>
class TbbFunctionNodeBase<IElement<void(Args...)>, Policy> : public IInputExecutionNode<NodeType::Functional, Args...>,
                                                             public IOutputExecutionNode<NodeType::Functional> {
 public:
  using ElementPtrType = IElementPtr<void(Args...)>;
  using ElementType    = IElement<void(Args...)>;
  using ResultType     = typename detail::Output<void>::type;
  using ArgumentsType  = typename detail::Input<Args...>::type;

  TbbFunctionNodeBase(TbbFlowGraphPtr            graph,
                      std::size_t                concurrency,
                      unsigned int               priority,
                      IElementPtr<void(Args...)> element)
      : node(graph->native(), concurrency, createExecuteFunction1(std::move(element)), priority) {}

  bool tryGet() override {
    ::tbb::flow::continue_msg msg;
    return node.try_get(msg);
  }
  bool tryPut(const Args&... args) override { return node.try_put(detail::Input<Args...>::forward(args...)); }

 protected:
  auto createExecuteFunction1(IElementPtr<void(Args...)> element) {
    if constexpr (1 < std::tuple_size_v<std::tuple<Args...>>) {
      return [this, e = std::move(element)](std::tuple<Args...> a) -> ::tbb::flow::continue_msg {
        try {
          std::apply(&IElement<void(Args...)>::process, std::tuple_cat(std::make_tuple(e), a));
          return ::tbb::flow::continue_msg{};
        }
        catch (std::exception& e) {
          LOG_ERROR(logger(), R"(Node "{}". Exception: {})", info.name, e.what());
          throw;
        }
      };
    } else {
      return [this, e = std::move(element)](Args... a) -> ::tbb::flow::continue_msg {
        try {
          e->process(a...);
          return ::tbb::flow::continue_msg{};
        }
        catch (std::exception& e) {
          LOG_ERROR(logger(), R"(Node "{}". Exception: {})", info.name, e.what());
          throw;
        }
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

  TbbFunctionNodeBase(TbbFlowGraphPtr graph, std::size_t concurrency, unsigned int priority, ElementPtrType element)
      : node(graph->native(), concurrency, createExecuteFunction1(std::move(element)), priority) {}

  bool tryGet(Result& val) override { return node.try_get(val); }

  bool tryPut(const Args&... args) override { return node.try_put(detail::Input<Args...>::forward(args...)); }

 protected:
  auto createExecuteFunction1(ElementPtrType element) {
    if constexpr (1 < std::tuple_size_v<std::tuple<Args...>>) {
      return [this, e = std::move(element)](std::tuple<Args...> a) -> Result {
        try {
          return std::apply(&ElementType::process, std::tuple_cat(std::make_tuple(e), a));
        }
        catch (std::exception& e) {
          LOG_ERROR(IExecutionNode::logger(), R"(Node "{}". Exception: {})", IExecutionNode::info.name, e.what());
          throw;
        }
      };
    } else {
      return [this, e = std::move(element)](Args... a) -> Result {
        try {
          return e->process(a...);
        }
        catch (std::exception& e) {
          LOG_ERROR(IExecutionNode::logger(), R"(Node "{}". Exception: {})", IExecutionNode::info.name, e.what());
          throw;
        }
      };
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

  static auto make(const common::Properties& cfg, IExecutionGraphPtr graph, ElementPtrType body) {
    auto params      = FunctionNodeConfig::make(cfg).value();
    auto node_params = NodeInfo::make(cfg).value();

    if (auto g = std::dynamic_pointer_cast<cvs::pipeline::tbb::TbbFlowGraph>(graph)) {
      auto node  = std::make_unique<TbbFunctionNode>(g, params.concurrency, params.priority, std::move(body));
      node->info = std::move(node_params);
      return node;
    }
    return std::unique_ptr<TbbFunctionNode>{};
  }

  TbbFunctionNode(TbbFlowGraphPtr graph, std::size_t concurrency, unsigned int priority, ElementPtrType element)
      : TbbFunctionNodeBase<Element, Policy>(std::move(graph), concurrency, priority, std::move(element)) {}

  std::any receiver(std::size_t) override {
    return std::make_any<::tbb::flow::receiver<ArgumentsType>*>(TbbFunctionNodeBase<Element, Policy>::nodePtr());
  }
  std::any sender(std::size_t) override {
    return std::make_any<::tbb::flow::sender<ResultType>*>(TbbFunctionNodeBase<Element, Policy>::nodePtr());
  }

  bool connect(std::any sender, std::size_t i) override {
    if (i == 0) {
      if (typeid(::tbb::flow::sender<ArgumentsType>*) == sender.type()) {
        ::tbb::flow::sender<ArgumentsType>* s = std::any_cast<::tbb::flow::sender<ArgumentsType>*>(sender);
        ::tbb::flow::make_edge(*s, TbbFunctionNodeBase<Element, Policy>::node);
        return true;
      }
    }
    return false;
  }
};

}  // namespace cvs::pipeline::tbb
