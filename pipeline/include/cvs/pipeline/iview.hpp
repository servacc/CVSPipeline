#pragma once

#include <any>
#include <memory>

namespace cvs::pipeline {

class IView {
 public:
  virtual ~IView() = default;

  virtual int exec() = 0;

  virtual bool addSender(std::size_t, std::any)   = 0;
  virtual bool addReceiver(std::size_t, std::any) = 0;
};

using IViewUPtr = std::unique_ptr<IView>;
using IViewPtr  = std::shared_ptr<IView>;

}  // namespace cvs::pipeline
