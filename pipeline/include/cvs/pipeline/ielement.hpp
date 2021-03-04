#pragma once

#include <memory>

namespace cvs::pipeline {

template <typename Signature>
class IElement;

template <typename Res, typename... Args>
class IElement<Res(Args...)> {
 public:
  using ArgsTuple = std::tuple<Args...>;
  using Result    = Res;

  virtual ~IElement() = default;

  virtual Result process(Args...) = 0;
};

template <typename T>
using IElementPtr = std::shared_ptr<IElement<T>>;
template <typename T>
using IElementUPtr = std::unique_ptr<IElement<T>>;

}  // namespace cvs::pipeline
