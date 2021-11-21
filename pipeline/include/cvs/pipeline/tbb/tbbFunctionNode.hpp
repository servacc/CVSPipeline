#pragma once

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/pipeline/dataCounter.hpp>
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
                                                             public IOutputExecutionNode<NodeType::Functional>,
                                                             public DataCounter {
 public:
  using ElementPtrType = IElementPtr<void(Args...)>;
  using ElementType    = IElement<void(Args...)>;
  using ResultType     = typename detail::Output<void>::type;
  using ArgumentsType  = typename detail::Input<Args...>::type;

  TbbFunctionNodeBase(TbbFlowGraphPtr            graph,
                      std::size_t                concurrency,
                      unsigned int               priority,
                      IElementPtr<void(Args...)> element)
      : node(graph->native(), concurrency, createExecuteFunction(std::move(element)), priority) {}

  bool tryGet() override {
    ::tbb::flow::continue_msg msg;
    return node.try_get(msg);
  }
  bool tryPut(const Args&... args) override { return node.try_put(detail::Input<Args...>::forward(args...)); }

 protected:
  auto createExecuteFunction(IElementPtr<void(Args...)> element) {
    if constexpr (1 < std::tuple_size_v<std::tuple<Args...>>) {
      return [this, e = std::move(element)](std::tuple<Args...> a) -> ::tbb::flow::continue_msg {
        try {
          LOG_TRACE(IExecutionNode::logger(), "Begin processing function node {}", IExecutionNode::info.name);
          beforeProcessing();
          std::apply(&IElement<void(Args...)>::process, std::tuple_cat(std::make_tuple(e), a));
          afterProcessing();
          LOG_TRACE(IExecutionNode::logger(), "End processing function node {}", IExecutionNode::info.name);
          return ::tbb::flow::continue_msg{};
        }
        catch (...) {
          cvs::common::throwWithNested<std::runtime_error>("Exception in {}", IExecutionNode::info.name);
        }

        cvs::common::throwException<std::runtime_error>(
            R"(Someone removed "return" from the method body of FunctionNode. )");
      };
    } else {
      return [this, e = std::move(element)](Args... a) -> ::tbb::flow::continue_msg {
        try {
          LOG_TRACE(IExecutionNode::logger(), "Begin processing function node {}", IExecutionNode::info.name);
          beforeProcessing();
          e->process(a...);
          afterProcessing();
          LOG_TRACE(IExecutionNode::logger(), "End processing function node {}", IExecutionNode::info.name);
          return ::tbb::flow::continue_msg{};
        }
        catch (...) {
          cvs::common::throwWithNested<std::runtime_error>("Exception in {}", IExecutionNode::info.name);
        }

        cvs::common::throwException<std::runtime_error>(
            R"(Someone removed "return" from the method body of FunctionNode. )");
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
      public IOutputExecutionNode<NodeType::Functional, Result>,
      public DataCounter {
 public:
  using ElementPtrType = IElementPtr<Result(Args...)>;
  using ElementType    = IElement<Result(Args...)>;
  using ResultType     = typename detail::Output<Result>::type;
  using ArgumentsType  = typename detail::Input<Args...>::type;

  TbbFunctionNodeBase(TbbFlowGraphPtr graph, std::size_t concurrency, unsigned int priority, ElementPtrType element)
      : node(graph->native(), concurrency, createExecuteFunction(std::move(element)), priority) {}

  bool tryGet(Result& val) override { return node.try_get(val); }

  bool tryPut(const Args&... args) override { return node.try_put(detail::Input<Args...>::forward(args...)); }

 protected:
  auto createExecuteFunction(ElementPtrType element) {
    if constexpr (1 < std::tuple_size_v<std::tuple<Args...>>) {
      return [this, e = std::move(element)](std::tuple<Args...> a) -> Result {
        try {
          LOG_TRACE(IExecutionNode::logger(), "Begin processing function node {}", IExecutionNode::info.name);
          beforeProcessing();
          auto result = std::apply(&ElementType::process, std::tuple_cat(std::make_tuple(e), a));
          afterProcessing();
          LOG_TRACE(IExecutionNode::logger(), "End processing function node {}", IExecutionNode::info.name);
          return result;
        }
        catch (...) {
          cvs::common::throwWithNested<std::runtime_error>("Exception in {}", IExecutionNode::info.name);
        }

        cvs::common::throwException<std::runtime_error>(
            R"(Someone removed "return" from the method body of FunctionNode. )");
      };
    } else {
      return [this, e = std::move(element)](Args... a) -> Result {
        try {
          LOG_TRACE(IExecutionNode::logger(), "Begin processing function node {}", IExecutionNode::info.name);
          beforeProcessing();
          auto result = e->process(a...);
          afterProcessing();
          LOG_TRACE(IExecutionNode::logger(), "End processing function node {}", IExecutionNode::info.name);
          return result;
        }
        catch (...) {
          cvs::common::throwWithNested<std::runtime_error>("Exception in {}", IExecutionNode::info.name);
        }

        cvs::common::throwException<std::runtime_error>(
            R"(Someone removed "return" from the method body of FunctionNode. )");
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

  static auto make(const common::Properties&              cfg,
                   IExecutionGraphPtr                     graph,
                   const common::FactoryPtr<std::string>& factory,
                   ElementPtrType                         body) {
    auto params      = FunctionNodeConfig::make(cfg).value();
    auto node_params = NodeInfo::make(cfg).value();

    if (auto g = std::dynamic_pointer_cast<cvs::pipeline::tbb::TbbFlowGraph>(graph)) {
      auto node = std::make_unique<TbbFunctionNode>(g, params.concurrency, params.priority, std::move(body));
      DataCounter::init(*node, cfg, factory);
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
