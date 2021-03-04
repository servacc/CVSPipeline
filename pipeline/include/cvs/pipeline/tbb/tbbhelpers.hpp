#pragma once

#include <cvs/common/configuration.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutiongraph.hpp>
#include <cvs/pipeline/tbb/tbbbroadcastnode.hpp>
#include <cvs/pipeline/tbb/tbbcontinuenode.hpp>
#include <cvs/pipeline/tbb/tbbflowgraph.hpp>
#include <cvs/pipeline/tbb/tbbfunctionnode.hpp>
#include <cvs/pipeline/tbb/tbbjoinnode.hpp>
#include <cvs/pipeline/tbb/tbbsourcenode.hpp>
#include <cvs/pipeline/tbb/tbbsplitnode.hpp>

#include <type_traits>

namespace cvs::pipeline::tbb {

struct TbbDefaultName {
  static const std::string graph_name;

  static const std::string broadcast_name;
  static const std::string continue_name;
  static const std::string function_name;
  static const std::string join_name;
  static const std::string source_name;
  static const std::string split_name;
};

// default nodes:
// if(void()       ) continue
// if(void()       ) brodcast void aka continue_msg
//
// element
//
// if(R()          ) continue R +
// if(R(A)         ) continue A +
// if(R(...)       ) brodcast R +
// if(R(A)         ) brodcast A +
// if(R(A)         ) function +
// if(R(A...)      ) join A...
// if(R(A...)      ) split A...
// if(tuple<R>(...)) split

namespace details {

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

}  // namespace details

template <typename R>
void registrateBrodcastNode() {
  using namespace cvs::pipeline;
  using namespace cvs::pipeline::tbb;

  if constexpr (std::is_same_v<R, void>) {
    common::Factory::registrateIf<IExecutionNodeUPtr(common::Configuration, IExecutionGraphPtr)>(
        TbbDefaultName::broadcast_name, TbbBroadcastNode<::tbb::flow::continue_msg>::make);
  } else {
    common::Factory::registrateIf<IExecutionNodeUPtr(common::Configuration, IExecutionGraphPtr)>(
        TbbDefaultName::broadcast_name, TbbBroadcastNode<R>::make);
  }
}

template <typename T>
using NodeFactoryFunction = IExecutionNodeUPtr(common::Configuration, IExecutionGraphPtr, IElementPtr<T>);

template <typename ElementFunc, template <typename...> class Node, typename Enable = std::true_type>
void registrateNode(std::string key, typename std::enable_if<std::is_same_v<std::true_type, Enable>>::type* = 0) {
  auto reg = common::Factory::registrateIf<IExecutionNodeUPtr(
      common::Configuration, IExecutionGraphPtr, IElementPtr<ElementFunc>)>(key, Node<IElement<ElementFunc>>::make);
  //  if (reg)
  //    std::cout << "Node \"" << key << "\" has registered for element \""
  //              << boost::core::demangle(typeid(IElement<ElementFunc>).name()) << "\"" << std::endl;
  //  else
  //    std::cout << "Duplicate node \"" << key << "\" has registered for element \""
  //              << boost::core::demangle(typeid(IElement<ElementFunc>).name()) << "\"" << std::endl;
}

template <typename ElementFunc, template <typename...> class Node, typename Enable>
void registrateNode(std::string key, typename std::enable_if<std::is_same_v<std::false_type, Enable>>::type* = 0) {
  //  std::cout << "Node " << key << " NOT registered for element \""
  //            << boost::core::demangle(typeid(IElement<ElementFunc>).name()) << "\"" << std::endl;
}

template <typename, typename>
struct is_not_same : public std::true_type {};

template <typename _Tp>
struct is_not_same<_Tp, _Tp> : public std::false_type {};

template <typename FactoryFunction, typename Impl>
void registrateElemetHelper(std::string key) {
  using namespace cvs::pipeline::tbb;

  using ElementPtr = typename std::function<FactoryFunction>::result_type;
  using Element    = typename ElementPtr::element_type;
  using ElementFun = typename details::RegistrationHelper<Element>::Fun;
  using Arg        = typename details::RegistrationHelper<Element>::Arg;
  using Res        = typename details::RegistrationHelper<Element>::Res;

  common::Factory::registrateIf<FactoryFunction>(key, Impl::make);

  registrateBrodcastNode<Arg>();
  registrateBrodcastNode<Res>();

  //  common::Factory::registrateIf<IExecutionNodeUPtr(common::Configuration, IExecutionGraphPtr, IElementPtr<Arg()>)>(
  //      "ContinueDefault"s, TbbContinueNode<IElement<Arg()>>::make);  // TODO: Is it necessary???
  //  common::Factory::registrateIf<IExecutionNodeUPtr(common::Configuration, IExecutionGraphPtr, IElementPtr<Res()>)>(
  //      "ContinueDefault"s, TbbContinueNode<IElement<Res()>>::make);

  registrateNode<Arg(), TbbContinueNode>(TbbDefaultName::continue_name);
  registrateNode<Res(), TbbContinueNode>(TbbDefaultName::continue_name);

  //  if constexpr (not std::is_same_v<Res, void>) {
  //    common::Factory::registrateIf<IExecutionNodeUPtr(common::Configuration, IExecutionGraphPtr,
  //    IElementPtr<Res()>)>(
  //        "SourceDefault"s, TbbSourceNode<IElement<Res()>>::make);
  //  }
  //  if constexpr (not std::is_same_v<Arg, void>) {
  //    common::Factory::registrateIf<IExecutionNodeUPtr(common::Configuration, IExecutionGraphPtr,
  //    IElementPtr<Arg()>)>(
  //        "SourceDefault"s, TbbSourceNode<IElement<Arg()>>::make);
  //  }

  // TODO: only if constexpr (not std::is_same_v<Res, void>) and std::is_same_v<Arg, void>)) ????
  //  registrateNode<Res(), TbbSourceNode, typename is_not_same<Res, void>::type>("SourceDefault"s);
  //  registrateNode<Arg(), TbbSourceNode, typename is_not_same<Arg, void>::type>("SourceDefault"s);
  registrateNode<ElementFun, TbbSourceNode, typename std::is_same<Arg, bool*>::type>(TbbDefaultName::source_name);

  //  if constexpr (not std::is_same_v<Arg, void>) {
  //    common::Factory::registrateIf<IExecutionNodeUPtr(common::Configuration, IExecutionGraphPtr, ElementPtr)>(
  //        "FunctionNodeDefault"s, TbbFunctionNode<Element>::make);
  //  }

  registrateNode<ElementFun, TbbFunctionNode, typename is_not_same<Arg, void>::type>(TbbDefaultName::function_name);

  //  if constexpr (is_tuple<Arg>::value) {
  //    common::Factory::registrateIf<IExecutionNodeUPtr(common::Configuration, IExecutionGraphPtr, ElementPtr)>(
  //        "JoinDefault"s, TbbJoinNode<Element>::make);
  //  }

  //  if constexpr (is_tuple<Res>::value) {
  //    common::Factory::registrateIf<IExecutionNodeUPtr(common::Configuration, IExecutionGraphPtr, ElementPtr)>(
  //        "SplitDefault"s, TbbSplitNode<Element>::make);
  //  }

  common::Factory::registrateIf<IExecutionNodeUPtr(std::string, common::Configuration, IExecutionGraphPtr)>(
      key,
      [key](std::string node_name, common::Configuration cfg,
            pipeline::IExecutionGraphPtr graph) -> pipeline::IExecutionNodeUPtr {
        std::shared_ptr<Element> element;
        if (node_name != TbbDefaultName::split_name && node_name != TbbDefaultName::join_name)
          element = common::Factory::create<std::unique_ptr<Element>>(key, cfg);
        return common::Factory::create<cvs::pipeline::IExecutionNodeUPtr>(node_name, cfg, graph, std::move(element));
      });
}

void registrateBase();

}  // namespace cvs::pipeline::tbb
