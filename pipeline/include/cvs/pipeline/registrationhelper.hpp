#pragma once

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutiongraph.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>

#include <functional>

namespace cvs::pipeline {

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

template <typename FactoryFunction, typename Impl>
void registerElemetHelper(const std::string key, const cvs::common::FactoryPtr<std::string>& factory) {
  using ElementPtr = typename std::function<FactoryFunction>::result_type;
  using Element    = typename ElementPtr::element_type;
  using Arg        = typename detail::RegistrationHelper<Element>::Arg;
  using Res        = typename detail::RegistrationHelper<Element>::Res;

  factory->tryRegisterType<FactoryFunction>(key, Impl::make);

  factory->tryRegisterType<IExecutionNodeUPtr(const std::string&, common::Config&, IExecutionGraphPtr&)>(
      key,
      [key, factory](const std::string& node_name, common::Config& cfg,
                     IExecutionGraphPtr& graph) -> IExecutionNodeUPtr {
        auto logger = cvs::logger::createLogger("cvs.pipeline.helper");

        auto node_type = factory->create<NodeType>(node_name);
        if (!node_type.has_value()) {
          LOG_ERROR(logger, R"s(Unable to create node "{}" for key "{}")s",
                    boost::core::demangle(typeid(NodeType).name()), node_name);
          return {};
        }

        switch (node_type.value_or(NodeType::Unknown)) {
          case ServiceIn: {
            std::shared_ptr<Arg> type;
            return factory->create<IExecutionNodeUPtr>(node_name, cfg, graph, type).value_or(nullptr);
          }
          case ServiceOut: {
            std::shared_ptr<Res> type;
            return factory->create<IExecutionNodeUPtr>(node_name, cfg, graph, type).value_or(nullptr);
          }
          case Functional: {
            auto element = factory->create<std::unique_ptr<Element>>(key, cfg);
            if (!element) {
              LOG_ERROR(logger, R"s(Unable to create element "{}" for key "{}")s",
                        boost::core::demangle(typeid(Element).name()), key);
              return {};
            }
            std::shared_ptr<Element> element_ptr{std::move(*element)};
            return factory->create<IExecutionNodeUPtr>(node_name, cfg, graph, element_ptr).value_or(nullptr);
          }
          default: {
            LOG_ERROR(logger, "Unknown node type");
          } break;
        }

        return {};
      });
}

void registerDefault(const cvs::common::FactoryPtr<std::string>& factory);

}  // namespace cvs::pipeline
