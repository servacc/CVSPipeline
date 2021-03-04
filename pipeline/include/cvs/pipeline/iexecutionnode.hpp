#pragma once

#include <any>
#include <memory>

namespace cvs::pipeline {

class IExecutionNode {
 public:
  virtual ~IExecutionNode() = default;

  virtual std::any receiver(std::size_t index) = 0;
  virtual std::any sender(std::size_t index)   = 0;

  virtual bool connect(std::any, std::size_t index) = 0;
};

using IExecutionNodePtr  = std::shared_ptr<IExecutionNode>;
using IExecutionNodeUPtr = std::unique_ptr<IExecutionNode>;

}  // namespace cvs::pipeline

namespace cvs::pipeline {

template <typename T>
class IOutputExecutionNode : public virtual IExecutionNode {
 public:
  virtual bool tryGet(T &v) = 0;
};

template <typename T>
using IOutputExecutionNodePtr = std::shared_ptr<IOutputExecutionNode<T>>;
template <typename T>
using IOutputExecutionNodeUPtr = std::unique_ptr<IOutputExecutionNode<T>>;

}  // namespace cvs::pipeline

namespace cvs::pipeline {

template <typename... T>
class IInputExecutionNode : public virtual IExecutionNode {
 public:
  virtual bool tryPut(const T &...) = 0;
};

template <typename... T>
using IInputExecutionNodePtr = std::shared_ptr<IInputExecutionNode<T...>>;
template <typename... T>
using IInputExecutionNodeUPtr = std::unique_ptr<IInputExecutionNode<T...>>;

}  // namespace cvs::pipeline

namespace cvs::pipeline {

class ISourceExecutionNode : public virtual IExecutionNode {
 public:
  virtual void activate() = 0;
};

using ISourceExecutionNodePtr  = std::shared_ptr<ISourceExecutionNode>;
using ISourceExecutionNodeUPtr = std::unique_ptr<ISourceExecutionNode>;

}  // namespace cvs::pipeline
