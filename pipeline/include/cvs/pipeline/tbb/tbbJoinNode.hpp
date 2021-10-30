#pragma once

#include <cvs/common/config.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionNode.hpp>
#include <cvs/pipeline/tbb/tbbFlowGraph.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

template <typename Tuple>
class TbbJoinNode;

template <typename... Args>
class TbbJoinNode<std::tuple<Args...>> : public IInputExecutionNode<NodeType::ServiceIn, Args...>,
                                         public IOutputExecutionNode<NodeType::ServiceIn, std::tuple<Args...>> {
 public:
  using ArgumentsType = std::tuple<Args...>;

  static_assert(sizeof...(Args) > 1, "The number of arguments must be equal to or more than two.");

  static std::unique_ptr<TbbJoinNode> make(const common::Properties& config,
                                           IExecutionGraphPtr        graph,
                                           const common::FactoryPtr<std::string>&,
                                           std::shared_ptr<ArgumentsType>);

  virtual ~TbbJoinNode() = default;
};

class TbbJoinNodeConstructor {};

}  // namespace cvs::pipeline::tbb

namespace cvs::pipeline::tbb {

template <typename Tuple, typename Policy>
class TbbJoinNodePolicy;

template <typename Policy, typename... Args>
class TbbJoinNodePolicy<std::tuple<Args...>, Policy> : public TbbJoinNode<std::tuple<Args...>> {
 public:
  using ArgumentsType = std::tuple<Args...>;

  TbbJoinNodePolicy(TbbFlowGraphPtr g)
      : node(g->native()) {}

  bool tryPut(const Args&...) override { return false; }
  bool tryGet(ArgumentsType&) override { return false; }

  std::any receiver(std::size_t i) override { return getPort<0, Args...>(i); }
  std::any sender(std::size_t) override { return std::make_any<::tbb::flow::sender<ArgumentsType>*>(&node); }

  bool connect(std::any sender, std::size_t i) override { return connectToPort<0, Args...>(std::move(sender), i); }

 private:
  template <std::size_t I>
  std::any getPort(std::size_t) {
    return {};
  }

  template <std::size_t I, typename A0, typename... AN>
  std::any getPort(std::size_t i) {
    if (I == i)
      return std::make_any<::tbb::flow::receiver<A0>*>(&::tbb::flow::input_port<I>(node));
    return getPort<I + 1, AN...>(i);
  }

  template <std::size_t I>
  bool connectToPort(std::any, std::size_t) {
    return false;
  }

  template <std::size_t I, typename A0, typename... AN>
  bool connectToPort(std::any sender, std::size_t i) {
    if (I == i) {
      if (typeid(::tbb::flow::sender<A0>*) != sender.type())
        return false;

      ::tbb::flow::sender<A0>* s = std::any_cast<::tbb::flow::sender<A0>*>(sender);
      ::tbb::flow::make_edge(*s, ::tbb::flow::input_port<I>(node));

      return true;
    }

    return connectToPort<I + 1, AN...>(std::move(sender), i);
  }

 private:
  ::tbb::flow::join_node<ArgumentsType, Policy> node;
};

}  // namespace cvs::pipeline::tbb

namespace cvs::pipeline::tbb {

enum class JoinPolicy {
  queueing,
  reserving,
  // TODO:
  //  key_matching,
  //  tag_matching,
};

}

namespace boost::property_tree {

template <>
struct translator_between<std::string, cvs::pipeline::tbb::JoinPolicy> {
  class Translator {
   public:
    using internal_type = std::string;
    using external_type = cvs::pipeline::tbb::JoinPolicy;

    inline static const std::map<external_type, std::string> names = {
        {cvs::pipeline::tbb::JoinPolicy::queueing, "QUEUEING"},
        {cvs::pipeline::tbb::JoinPolicy::reserving, "RESERVING"},
    };

    boost::optional<external_type> get_value(const internal_type& v) {
      auto iter = std::find_if(names.begin(), names.end(), [&](auto& pair) { return pair.second == v; });
      if (iter != names.end())
        return iter->first;
      return {};
    }
    boost::optional<internal_type> put_value(const external_type& v) {
      if (auto iter = names.find(v); iter != names.end())
        return iter->second;
      return {};
    }
  };

  using type = Translator;
};
}  // namespace boost::property_tree

namespace cvs::pipeline::tbb {

CVS_CONFIG(JoinNodeConfig, "") { CVS_FIELD_DEF(policy, JoinPolicy, JoinPolicy::queueing, ""); };

template <typename... Args>
std::unique_ptr<TbbJoinNode<std::tuple<Args...>>> TbbJoinNode<std::tuple<Args...>>::make(
    const common::Properties& config,
    IExecutionGraphPtr        graph,
    const common::FactoryPtr<std::string>&,
    std::shared_ptr<ArgumentsType>) {
  if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph)) {
    auto params = JoinNodeConfig::make(config).value();
    switch (params.policy) {
      case cvs::pipeline::tbb::JoinPolicy::queueing:
        return std::make_unique<TbbJoinNodePolicy<std::tuple<Args...>, ::tbb::flow::queueing>>(g);
      case cvs::pipeline::tbb::JoinPolicy::reserving:
        return std::make_unique<TbbJoinNodePolicy<std::tuple<Args...>, ::tbb::flow::reserving>>(g);
    }
  }
  return std::unique_ptr<TbbJoinNode>{};
}

}  // namespace cvs::pipeline::tbb
