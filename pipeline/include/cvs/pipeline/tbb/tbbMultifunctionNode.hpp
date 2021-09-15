#pragma once

#include <cvs/common/config.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/tbb/tbbflowgraph.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

namespace detail {

template <typename T>
struct MultifunctionResult;

template <typename T>
struct MultifunctionResult<std::optional<T>> {
  using Result = std::tuple<T>;

  using IsOptional       = std::true_type;
  using IsTupeOfOptional = std::false_type;
};

template <typename... T>
struct MultifunctionResult<std::tuple<std::optional<T>...>> {
  using Result = std::tuple<T...>;

  using IsOptional       = std::false_type;
  using IsTupeOfOptional = std::true_type;
};

}  // namespace detail

template <typename Element, typename Policy = ::tbb::flow::queueing>
class TbbMultifunctionNode;

template <typename Result, typename... Args, typename Policy>
class TbbMultifunctionNode<IElement<Result(Args...)>, Policy>
    : public IInputExecutionNode<NodeType::Functional, Args...> {
  using ArgumentsType  = std::tuple<Args...>;
  using ResultType     = typename detail::MultifunctionResult<Result>::Result;
  using ElementPtrType = IElementPtr<Result(Args...)>;
  using ElementType    = IElement<Result(Args...)>;
  using NodeType       = ::tbb::flow::multifunction_node<std::tuple<Args...>, ResultType, Policy>;

 public:
  static auto make(const common::Properties &cfg, IExecutionGraphPtr graph, ElementPtrType body) {
    auto params      = FunctionNodeConfig::make(cfg).value();
    auto node_params = NodeInfo::make(cfg).value();

    if (auto g = std::dynamic_pointer_cast<cvs::pipeline::tbb::TbbFlowGraph>(graph)) {
      auto node  = std::make_unique<TbbMultifunctionNode>(g, params.concurrency, std::move(body), params.priority);
      node->info = std::move(node_params);
      return node;
    }
    return std::unique_ptr<TbbMultifunctionNode>{};
  }

  TbbMultifunctionNode(TbbFlowGraphPtr              graph,
                       std::size_t                  concurrency,
                       ElementPtrType               element,
                       ::tbb::flow::node_priority_t priority)
      : node(
            graph->native(),
            concurrency,
            [e = std::move(element)](const typename NodeType::input_type & v,
                                     typename NodeType::output_ports_type &ports) {
              auto outputs = std::apply(&ElementType::process, std::tuple_cat(std::make_tuple(e), v));
              sendResult(outputs, ports);
            },
            priority) {}

  bool tryPut(const Args &...args) override { return node.try_put(std::tie(args...)); }

  std::any receiver(std::size_t index) override {
    if (index == 0)
      return std::make_any<::tbb::flow::receiver<ArgumentsType> *>(&node);
    return {};
  }

  std::any sender(std::size_t index) override { return getPort(index); }

  bool connect(std::any sender, std::size_t index) override {
    if (index == 0) {
      if (typeid(::tbb::flow::sender<ArgumentsType> *) == sender.type()) {
        ::tbb::flow::sender<ArgumentsType> *s = std::any_cast<::tbb::flow::sender<ArgumentsType> *>(sender);
        ::tbb::flow::make_edge(*s, node);
        return true;
      }
    }
    return false;
  }

  template <typename T>
  static void sendResult(std::optional<T> &result, typename NodeType::output_ports_type &ports) {
    if (result.has_value())
      std::get<0>(ports).try_put(*result);
  }

  template <std::size_t I = 0, typename... T>
  static void sendResult(std::tuple<std::optional<T>...> &result, typename NodeType::output_ports_type &ports) {
    if constexpr (I < std::tuple_size_v<std::tuple<std::optional<T>...>>) {
      if (std::get<I>(result).has_value())
        std::get<I>(ports).try_put(*std::get<I>(result));

      sendResult<I + 1, T...>(result, ports);
    }
  }

 private:
  template <std::size_t I = 0>
  std::any getPort(std::size_t i) {
    if constexpr (I < std::tuple_size_v<ResultType>) {
      if (I == i)
        return std::make_any<::tbb::flow::sender<std::tuple_element_t<I, ResultType>> *>(
            &::tbb::flow::output_port<I>(node));
      return getPort<I + 1>(i);
    }

    return {};
  }

 private:
  NodeType node;
};

}  // namespace cvs::pipeline::tbb
