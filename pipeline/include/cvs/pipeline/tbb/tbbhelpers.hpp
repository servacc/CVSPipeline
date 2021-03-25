#pragma once

#include <boost/core/demangle.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutiongraph.hpp>
#include <cvs/pipeline/registrationhelper.hpp>
#include <cvs/pipeline/tbb/tbbbroadcastnode.hpp>
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

template <typename FactoryFunction>
struct RegistrationHelper;

template <typename R, typename... A>
struct RegistrationHelper<IElement<R(A...)>> {
  using Arg = std::tuple<A...>;
  using Res = R;
  using Fun = R(A...);
};

template <typename R, typename A>
struct RegistrationHelper<IElement<R(A)>> {
  using Arg = A;
  using Res = R;
  using Fun = R(A);
};

template <typename R>
struct RegistrationHelper<IElement<R()>> {
  using Arg = void;
  using Res = R;
  using Fun = R();
};

template <typename>
struct is_tuple : std::false_type {};

template <typename... T>
struct is_tuple<std::tuple<T...>> : std::true_type {};

template <typename, typename>
struct is_not_same : public std::true_type {};

template <typename _Tp>
struct is_not_same<_Tp, _Tp> : public std::false_type {};

}  // namespace detail

template <typename NType>
bool registrateNodeTypeFun(std::string key) {
  auto type_reg = common::Factory::registrateIf<NodeType()>(key, []() -> NodeType { return NType::node_type; });

  if (!type_reg) {
    auto registred_type = common::Factory::create<NodeType>(key).value();
    if (registred_type != NType::node_type) {
      auto logger = cvs::logger::createLogger("cvs.pipeline.tbb.helper");
      LOG_ERROR(logger, R"s(Can't registrate different types ({} != {}) for key ("{}"))s", key, int(registred_type),
                int(NType::node_type));
      return false;
    }
  }

  return true;
}

template <typename BaseElement, template <typename...> class Node, typename Enable = std::true_type>
void registrateNode(std::string key, typename std::enable_if<std::is_same_v<std::true_type, Enable>>::type* = 0) {
  if (!registrateNodeTypeFun<Node<BaseElement>>(key))
    return;

  auto logger = cvs::logger::createLogger("cvs.pipeline.tbb.helper");

  auto reg = common::Factory::registrateIf<IExecutionNodeUPtr(
      common::Config, IExecutionGraphPtr, std::shared_ptr<BaseElement>)>(key, Node<BaseElement>::make);
  if (reg) {
    LOG_DEBUG(logger, R"s(Register: node "{}" for element "{}")s", key,
              boost::core::demangle(typeid(BaseElement).name()));
  } else {
    LOG_DEBUG(logger, R"s(Duplicate: node "{}" for element "{}")s", key,
              boost::core::demangle(typeid(BaseElement).name()));
  }
}

template <typename BaseElement, template <typename...> class Node, typename Enable>
void registrateNode(std::string key, typename std::enable_if<std::is_same_v<std::false_type, Enable>>::type* = 0) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.tbb.helper");
  LOG_DEBUG(logger, R"s(Ignore: node "{}" for element "{}")s", key, boost::core::demangle(typeid(BaseElement).name()));
}

template <typename FactoryFunction, typename Impl>
void registrateElemetAndTbbHelper(std::string key) {
  using namespace cvs::pipeline::tbb;

  using ElementPtr  = typename std::function<FactoryFunction>::result_type;
  using Element     = typename ElementPtr::element_type;
  using ElementFun  = typename detail::RegistrationHelper<Element>::Fun;
  using Arg         = typename detail::RegistrationHelper<Element>::Arg;
  using Res         = typename detail::RegistrationHelper<Element>::Res;
  using BaseElement = IElement<ElementFun>;

  registrateElemetHelper<FactoryFunction, Impl>(key);

  // functional nodes
  registrateNode<BaseElement, TbbContinueNode, typename std::is_same<Arg, void>::type>(TbbDefaultName::continue_name);
  registrateNode<BaseElement, TbbSourceNode, typename std::is_same<Arg, bool*>::type>(TbbDefaultName::source);
  registrateNode<BaseElement, TbbFunctionNode, typename detail::is_not_same<Arg, void>::type>(TbbDefaultName::function);

  // service nodes
  registrateNode<Res, TbbBroadcastNode, typename detail::is_not_same<Res, void>::type>(TbbDefaultName::broadcast);
  registrateNode<Arg, TbbJoinNode, typename detail::is_tuple<Arg>::type>(TbbDefaultName::join);
  registrateNode<Res, TbbSplitNode, typename detail::is_tuple<Res>::type>(TbbDefaultName::split_name);
}

void registrateBase();

}  // namespace cvs::pipeline::tbb
