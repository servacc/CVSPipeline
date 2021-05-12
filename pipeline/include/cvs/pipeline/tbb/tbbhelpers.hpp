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

template <typename>
struct is_tuple : std::false_type {};

template <typename... T>
struct is_tuple<std::tuple<T...>> : std::true_type {};

}  // namespace detail

template <typename NType>
bool registerNodeTypeFun(const std::string& key, const cvs::common::FactoryPtr<std::string>& factory) {
  auto type_reg = factory->tryRegisterType<NodeType()>(key, []() -> NodeType { return NType::node_type; });

  auto logger = cvs::logger::createLogger("cvs.pipeline.tbb.helper");

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
  if constexpr (Enable) {
    if (!registerNodeTypeFun<Node<BaseElement>>(key, factory))
      return;

    auto logger = cvs::logger::createLogger("cvs.pipeline.tbb.helper");

    auto reg =
        factory
            ->tryRegisterType<IExecutionNodeUPtr(common::Config&, IExecutionGraphPtr&, std::shared_ptr<BaseElement>&)>(
                key, Node<BaseElement>::make);
    if (reg) {
      LOG_DEBUG(logger, R"s(Register: node "{}" for element "{}")s", key,
                boost::core::demangle(typeid(BaseElement).name()));
    } else {
      LOG_DEBUG(logger, R"s(Duplicate: node "{}" for element "{}")s", key,
                boost::core::demangle(typeid(BaseElement).name()));
    }
  } else {
    auto logger = cvs::logger::createLogger("cvs.pipeline.tbb.helper");
    LOG_TRACE(logger, R"s(Ignore: node "{}" for element "{}")s", key,
              boost::core::demangle(typeid(BaseElement).name()));
  }
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

  // service nodes
  registerNode<Res, TbbBroadcastNode, !std::is_same<Res, void>::value>(TbbDefaultName::broadcast, factory);
  registerNode<Arg, TbbJoinNode, detail::is_tuple<Arg>::value>(TbbDefaultName::join, factory);
  registerNode<Res, TbbSplitNode, detail::is_tuple<Res>::value>(TbbDefaultName::split_name, factory);
}

void registerBase(const cvs::common::FactoryPtr<std::string>& factory);

}  // namespace cvs::pipeline::tbb
