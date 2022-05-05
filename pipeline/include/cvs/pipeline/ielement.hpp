#pragma once

#include <cvs/common/config.hpp>

#include <memory>

namespace cvs::pipeline {

template <typename Signature>
class IElement {
  static_assert(sizeof(Signature) < 0, "IElement template argument must be function type");
};

template <typename Res, typename... Args>
class IElement<Res(Args...)> {
 public:
  using ArgsTuple = std::tuple<Args...>;
  using Result    = Res;

  virtual ~IElement() = default;

  virtual Result process(Args...) = 0;

  virtual bool isStopped() const { return true; }
};

template <typename Signature>
struct IElementPointerProxy {
  static_assert(sizeof(Signature) < 0, "IElementPtr (IElementUPtr) template argument must be function type");
};

template <typename Res, typename... Args>
struct IElementPointerProxy<Res(Args...)> {
  using SharedPointer = std::shared_ptr<IElement<Res(Args...)>>;
  using UniquePointer = std::unique_ptr<IElement<Res(Args...)>>;
};

template <typename T>
using IElementPtr = typename IElementPointerProxy<T>::SharedPointer;
template <typename T>
using IElementUPtr = typename IElementPointerProxy<T>::UniquePointer;

}  // namespace cvs::pipeline

namespace cvs::pipeline {

template <typename T, typename Description>
struct ArgHelper {
  using type        = T;
  using description = Description;
};

template <typename Result, typename... Args>
struct SignatureHelper {
  using type      = Result(typename Args::type...);
  using result    = Result;
  using arguments = std::tuple<Args...>;
};

template <typename Type, typename Description, typename... Signature>
class ElementHelper : public common::CVSConfig<Type, Description>, public IElement<typename Signature::type>... {
 public:
  using common::CVSConfig<Type, Description>::makeUPtr;

  static std::array<std::string, 0> describeArguments(std::tuple<>*) { return {}; }

  template <typename T, typename... Args>
  static std::array<std::string, std::tuple_size_v<std::tuple<T, Args...>>> describeArguments(std::tuple<T, Args...>*) {
    std::array<std::string, std::tuple_size_v<std::tuple<T, Args...>>> result;
    using ArgDesc = typename T::description;
    result[0]     = fmt::format("{: <10} {}", boost::core::demangle(typeid(typename T::type).name()),
                            std::string{ArgDesc::value, ArgDesc::size});

    auto tail = describeArguments((std::tuple<Args...>*)nullptr);
    std::move(tail.begin(), tail.end(), std::next(result.begin()));

    return result;
  }

  template <typename T, typename... TS>
  static std::vector<std::string> describeSignature() {
    auto args = describeArguments((typename T::arguments*)nullptr);

    std::vector<std::string> result;
    std::move(args.begin(), args.end(), std::back_inserter(result));

    std::string output = fmt::format("#{} {} (", sizeof...(Signature) - sizeof...(TS),
                                     boost::core::demangle(typeid(typename T::result).name()));
    if (result.empty())
      result.push_back(output + ')');
    else {
      result.front() = output + result.front();
      std::string indent(output.size(), ' ');
      for (auto iter = std::next(result.begin()); iter != result.end(); ++iter)
        *iter = indent + *iter;
      result.back() += ')';
    }

    if constexpr (sizeof...(TS) > 0) {
      auto another = describeSignature<TS...>();
      std::move(another.begin(), another.end(), std::back_inserter(result));
    }

    return result;
  }

  static std::vector<std::string> describeElement() {
    std::vector<std::string> result;

    auto signature = describeSignature<Signature...>();
    auto settings  = cvs::common::CVSConfig<Type, Description>::fields();

    result.push_back("Description: " + std::string{Description::value, Description::size});
    result.push_back("Signatures: " + signature.front());
    std::transform(std::next(signature.begin()), signature.end(), std::back_inserter(result),
                   [](const std::string& str) { return std::string(12, ' ') + str; });

    if (!settings.empty()) {
      result.push_back("Settings: " + settings.front()->descriptionString());
      std::transform(std::next(settings.begin()), settings.end(), std::back_inserter(result),
                     [](const common::ICVSField* field) { return std::string(10, ' ') + field->descriptionString(); });
    }

    return result;
  }
};

}  // namespace cvs::pipeline

#define Arg(type, description) cvs::pipeline::ArgHelper<type, CVS_CONSTEXPRSTRING(description)>
#define Fun(...)               cvs::pipeline::SignatureHelper<__VA_ARGS__>

#define CVS_ELEMENT(name, description, ...) \
  class name : public cvs::pipeline::ElementHelper<name, CVS_CONSTEXPRSTRING(description), __VA_ARGS__>
