#pragma once

#include <cvs/common/configuration.hpp>
#include <cvs/common/factory.hpp>
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
void registrateElemetHelper(std::string key) {
  using ElementPtr = typename std::function<FactoryFunction>::result_type;
  using Element    = typename ElementPtr::element_type;
  using ElementFun = typename detail::RegistrationHelper<Element>::Fun;
  using Arg        = typename detail::RegistrationHelper<Element>::Arg;
  using Res        = typename detail::RegistrationHelper<Element>::Res;

  common::Factory::registrateIf<FactoryFunction>(key, Impl::make);

  common::Factory::registrateIf<IExecutionNodeUPtr(std::string, common::Configuration, IExecutionGraphPtr)>(
      key,
      [key](std::string node_name, common::Configuration cfg,
            pipeline::IExecutionGraphPtr graph) -> pipeline::IExecutionNodeUPtr {
        auto node_type = common::Factory::create<NodeType>(node_name);
        if (!node_type.has_value()) {
          std::cerr << "Node Type?" << std::endl;
          return {};
        }

        switch (node_type.value_or(NodeType::Unknown)) {
          case ServiceIn: break;
          case ServiceOut: break;
          case Functional: {
            auto element = common::Factory::create<std::unique_ptr<Element>>(key, cfg);
            if (!element) {
              std::cerr << "Null element" << std::endl;
              return {};
            }
            return common::Factory::create<cvs::pipeline::IExecutionNodeUPtr>(
                       node_name, cfg, graph, std::shared_ptr<Element>{std::move(*element)})
                .value_or(nullptr);
          } break;
          default: {
            std::cerr << "Node type" << std::endl;
          }
        }

        return {};
      });
}

}  // namespace cvs::pipeline
