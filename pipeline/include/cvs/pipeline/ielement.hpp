#pragma once

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
