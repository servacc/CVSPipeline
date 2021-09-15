#pragma once

#include <boost/core/demangle.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutiongraph.hpp>
#include <cvs/pipeline/registrationhelper.hpp>
#include <cvs/pipeline/tbb/tbbBroadcastNode.hpp>
#include <cvs/pipeline/tbb/tbbBufferNode.hpp>
#include <cvs/pipeline/tbb/tbbMultifunctionNode.hpp>
#include <cvs/pipeline/tbb/tbbOverwriteNode.hpp>
#include <cvs/pipeline/tbb/tbbcontinuenode.hpp>
#include <cvs/pipeline/tbb/tbbdefinitions.hpp>
#include <cvs/pipeline/tbb/tbbflowgraph.hpp>
#include <cvs/pipeline/tbb/tbbfunctionnode.hpp>
#include <cvs/pipeline/tbb/tbbjoinnode.hpp>
#include <cvs/pipeline/tbb/tbbsourcenode.hpp>
#include <cvs/pipeline/tbb/tbbsplitnode.hpp>

#include <iostream>
#include <type_traits>

namespace cvs::pipeline::tbb {

namespace detail {

template <typename T>
struct remove_optional {
  using Type = T;
};

template <typename T>
struct remove_optional<std::optional<T>> {
  using Type = T;
};

template <typename T>
using remove_optional_t = typename remove_optional<T>::Type;

template <typename>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <typename>
struct is_tuple_of_optional : std::false_type {};

template <typename... T>
struct is_tuple_of_optional<std::tuple<std::optional<T>...>> : std::true_type {};

template <typename>
struct is_tuple : std::false_type {};

template <typename... T>
struct is_tuple<std::tuple<T...>> : std::true_type {};

template <typename T>
class HelperElement : public cvs::pipeline::IElement<void(T)> {
 public:
  static std::unique_ptr<HelperElement> make(const common::Properties&) { return std::make_unique<HelperElement>(); }

  void process(T) override {}
};

}  // namespace detail

template <typename NType>
bool registerNodeTypeFun(const std::string& key, const cvs::common::FactoryPtr<std::string>& factory) {
  auto type_reg = factory->tryRegisterType<NodeType()>(key, []() -> NodeType { return NType::node_type; });

  auto logger = cvs::logger::createLogger("cvs.pipeline.tbb.helper").value();

  if (!type_reg) {
    auto registred_type = factory->create<NodeType>(key).value();
    if (registred_type != NType::node_type) {
      LOG_ERROR(logger, R"s(Can't register different types ({} != {}) for key ("{}"))s", key, int(registred_type),
                int(NType::node_type));
      return false;
    }
  }

  LOG_DEBUG(logger, R"s(Type registered: type {} for key "{}")s", int(NType::node_type), key);

  return true;
}

template <typename BaseElement, template <typename...> class Node, bool Enable>
void registerNode(const std::string& key, const cvs::common::FactoryPtr<std::string>& factory) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.tbb.helper").value();
  if constexpr (Enable) {
    if (!registerNodeTypeFun<Node<BaseElement>>(key, factory))
      return;

    auto reg = factory->tryRegisterType<IExecutionNodeUPtr(
        const common::Properties&, IExecutionGraphPtr&, std::shared_ptr<BaseElement>&)>(key, Node<BaseElement>::make);
    if (reg) {
      LOG_DEBUG(logger, R"s(Register: node "{}" for element "{}")s", key,
                boost::core::demangle(typeid(BaseElement).name()));
    } else {
      LOG_DEBUG(logger, R"s(Duplicate: node "{}" for element "{}")s", key,
                boost::core::demangle(typeid(BaseElement).name()));
    }
  } else {
    LOG_TRACE(logger, R"s(Ignore: node "{}" for element "{}")s", key,
              boost::core::demangle(typeid(BaseElement).name()));
  }
}

template <typename T>
void registerServiceNodes(const std::string& key, const cvs::common::FactoryPtr<std::string>& factory) {
  registerNode<T, TbbOverwriteNodeOut, !std::is_same<T, void>::value>(TbbDefaultName::overwrite_out, factory);
  registerNode<T, TbbOverwriteNodeIn, !std::is_same<T, void>::value>(TbbDefaultName::overwrite_in, factory);
  registerNode<T, TbbBroadcastNodeOut, !std::is_same<T, void>::value>(TbbDefaultName::broadcast_out, factory);
  registerNode<T, TbbBroadcastNodeIn, !std::is_same<T, void>::value>(TbbDefaultName::broadcast_in, factory);
  registerNode<T, TbbBufferNodeOut, !std::is_same<T, void>::value>(TbbDefaultName::buffer_out, factory);
  registerNode<T, TbbBufferNodeIn, !std::is_same<T, void>::value>(TbbDefaultName::buffer_in, factory);
  registerNode<T, TbbJoinNode, detail::is_tuple<T>::value>(TbbDefaultName::join, factory);
  registerNode<T, TbbSplitNode, detail::is_tuple<T>::value>(TbbDefaultName::split, factory);
  if constexpr (detail::is_optional<T>::value) {
    auto helper_key = "*" + key;
    registerElemetHelper<IElementUPtr<void(detail::remove_optional_t<T>)>(common::Properties&),
                         detail::HelperElement<detail::remove_optional_t<T>>>(helper_key, factory);
    registerServiceNodes<detail::remove_optional_t<T>>(helper_key, factory);
  }
}

template <std::size_t I = 0, typename T>
void registerServiceNodesForTupleElements(const std::string&, const cvs::common::FactoryPtr<std::string>&, T*) {}

template <std::size_t I = 0, typename T, typename... Args>
void registerServiceNodesForTupleElements(const std::string&                          root_key,
                                          const cvs::common::FactoryPtr<std::string>& factory,
                                          std::tuple<T, Args...>*) {
  std::string key = fmt::format("{}[{}]", root_key, I);
  registerElemetHelper<IElementUPtr<void(T)>(common::Properties&), detail::HelperElement<T>>(key, factory);

  registerNode<IElement<void(T)>, TbbFunctionNode, !std::is_same<T, void>::value>(TbbDefaultName::function, factory);

  registerServiceNodes<T>(root_key, factory);

  registerServiceNodesForTupleElements(key, factory, (T*)nullptr);

  registerServiceNodesForTupleElements<I + 1>(root_key, factory, (std::tuple<Args...>*)nullptr);
}

template <typename FactoryFunction, typename Impl>
void registerElemetAndTbbHelper(const std::string& key, const cvs::common::FactoryPtr<std::string>& factory) {
  using namespace cvs::pipeline::tbb;

  using ElementPtr  = typename std::function<FactoryFunction>::result_type;
  using Element     = typename ElementPtr::element_type;
  using ElementFun  = typename cvs::pipeline::detail::RegistrationHelper<Element>::Fun;
  using Arg         = typename cvs::pipeline::detail::RegistrationHelper<Element>::Arg;
  using Res         = typename cvs::pipeline::detail::RegistrationHelper<Element>::Res;
  using BaseElement = IElement<ElementFun>;

  registerElemetHelper<FactoryFunction, Impl>(key, factory);

  // functional nodes
  registerNode<BaseElement, TbbContinueNode, std::is_same<Arg, void>::value>(TbbDefaultName::continue_name, factory);
  registerNode<BaseElement, TbbSourceNode, std::is_same<Arg, void>::value>(TbbDefaultName::source, factory);
  registerNode<BaseElement, TbbFunctionNode, !std::is_same<Arg, void>::value>(TbbDefaultName::function, factory);
  registerNode<BaseElement, TbbMultifunctionNode,
               !std::is_same<Arg, void>::value &&
                   (detail::is_optional<Res>::value || detail::is_tuple_of_optional<Res>::value)>(
      TbbDefaultName::multifunction, factory);

  // service nodes
  registerServiceNodes<Arg>(key + ".in", factory);
  registerServiceNodes<Res>(key + ".out", factory);

  registerServiceNodesForTupleElements(key + ".in", factory, (Arg*)nullptr);
  registerServiceNodesForTupleElements(key + ".out", factory, (Res*)nullptr);
}

void registerBase(const cvs::common::FactoryPtr<std::string>& factory);

}  // namespace cvs::pipeline::tbb
