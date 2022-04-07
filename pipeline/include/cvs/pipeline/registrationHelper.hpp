#pragma once

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/iexecutionGraph.hpp>
#include <cvs/pipeline/iexecutionNode.hpp>

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

// Primary template with a static assertion
// for a meaningful error message
// if it ever gets instantiated.
// We could leave it undefined if we didn't care.

template <typename, typename T>
struct hasDescribeElement {
  static_assert(std::integral_constant<T, false>::value, "Second template parameter needs to be of function type.");
};

// specialization that does the checking

template <typename C, typename Ret, typename... Args>
struct hasDescribeElement<C, Ret(Args...)> {
 private:
  template <typename T>
  static constexpr auto check(T*) ->
      typename std::is_same<decltype(std::declval<T>().describeElement(std::declval<Args>()...)),
                            Ret       // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                            >::type;  // attempt to call it and see if the return type is correct

  template <typename>
  static constexpr std::false_type check(...);

  typedef decltype(check<C>(0)) type;

 public:
  static constexpr bool value = type::value;
};

}  // namespace detail

template <typename FactoryFunction, typename Impl>
void registerElemetHelper(const std::string key, const cvs::common::FactoryPtr<std::string>& factory) {
  using ElementPtr = typename std::function<FactoryFunction>::result_type;
  using Element    = typename ElementPtr::element_type;
  using Arg        = typename detail::RegistrationHelper<Element>::Arg;
  using Res        = typename detail::RegistrationHelper<Element>::Res;

  auto logger = cvs::logger::createLogger("cvs.pipeline.helper").value();
  LOG_TRACE(logger, R"(Try register element with type "{}" for key "{}")",
            boost::core::demangle(typeid(Element).name()), key);

  factory->tryRegisterType<FactoryFunction>(key, Impl::make);

  if constexpr (detail::hasDescribeElement<Impl, std::vector<std::string>()>::value) {
    using InfoType = std::map<std::string, std::vector<std::string>>;
    factory->tryRegisterType<InfoType*()>("info", []() -> InfoType* {
      static InfoType descriptions;
      return &descriptions;
    });

    auto description_map = factory->create<std::map<std::string, std::vector<std::string>>*>("info").value();
    description_map->emplace(key, Impl::describeElement());
  }

  factory->tryRegisterType<IExecutionNodeUPtr(const std::string&, const common::Properties&, IExecutionGraphPtr&)>(
      key,
      [key, factory](const std::string& node_name, const common::Properties& cfg,
                     IExecutionGraphPtr& graph) -> IExecutionNodeUPtr {
        try {
          auto logger = cvs::logger::createLogger("cvs.pipeline.helper").value();
          LOG_TRACE(logger, R"(Trying to create a node "{}"...)", node_name);

          auto node_type = factory->create<NodeType>(node_name).value();

          switch (node_type) {
            case ServiceIn: {
              std::shared_ptr<Arg> type;
              return factory->create<IExecutionNodeUPtr>(node_name, cfg, graph, factory, type).value();
            }
            case ServiceOut: {
              std::shared_ptr<Res> type;
              return factory->create<IExecutionNodeUPtr>(node_name, cfg, graph, factory, type).value();
            }
            case Functional: {
              std::shared_ptr<Element> element = factory->create<std::unique_ptr<Element>>(key, cfg).value();
              return factory->create<IExecutionNodeUPtr>(node_name, cfg, graph, factory, element).value();
            }
            default: {
              throw std::runtime_error("Unknown node type");
            } break;
          }
        }
        catch (...) {
          common::throwWithNested<std::runtime_error>(R"(Unable to create node for key "{}")", node_name);
        }

        // fix Warning: No return, in function returning non-void
        return {};
      });
}

void registerDefault(const cvs::common::FactoryPtr<std::string>& factory);

}  // namespace cvs::pipeline
