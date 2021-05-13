#pragma once

#include <any>
#include <memory>

namespace cvs::pipeline {

enum NodeType {
  ServiceIn,
  ServiceOut,
  Functional,

  Unknown,

  Count
};

class IExecutionNode {
 public:
  virtual ~IExecutionNode() = default;

  virtual NodeType type() const = 0;

  virtual std::any receiver(std::size_t index) = 0;
  virtual std::any sender(std::size_t index)   = 0;

  virtual bool connect(std::any, std::size_t index) = 0;
};

using IExecutionNodePtr  = std::shared_ptr<IExecutionNode>;
using IExecutionNodeUPtr = std::unique_ptr<IExecutionNode>;

}  // namespace cvs::pipeline

namespace cvs::pipeline {

template <NodeType NodeT>
class IExecutionTypedNode : public IExecutionNode {
 public:
  static constexpr NodeType node_type = NodeT;

  NodeType type() const final { return node_type; }
};

template <NodeType Type, typename... T>
class IOutputExecutionNode : public virtual IExecutionTypedNode<Type> {
 public:
  virtual bool tryGet(T &...) = 0;
};

template <NodeType Type, typename T>
using IOutputExecutionNodePtr = std::shared_ptr<IOutputExecutionNode<Type, T>>;
template <NodeType Type, typename T>
using IOutputExecutionNodeUPtr = std::unique_ptr<IOutputExecutionNode<Type, T>>;

}  // namespace cvs::pipeline

namespace cvs::pipeline {

template <NodeType Type, typename... T>
class IInputExecutionNode : public virtual IExecutionTypedNode<Type> {
 public:
  virtual bool tryPut(const T &...) = 0;
};

template <NodeType Type, typename... T>
using IInputExecutionNodePtr = std::shared_ptr<IInputExecutionNode<Type, T...>>;
template <NodeType Type, typename... T>
using IInputExecutionNodeUPtr = std::unique_ptr<IInputExecutionNode<Type, T...>>;

}  // namespace cvs::pipeline

namespace cvs::pipeline {

template <NodeType Type>
class ISourceExecutionNode : public virtual IExecutionTypedNode<Type> {
 public:
  virtual void activate() = 0;
};

template <NodeType Type>
using ISourceExecutionNodePtr = std::shared_ptr<ISourceExecutionNode<Type>>;
template <NodeType Type>
using ISourceExecutionNodeUPtr = std::unique_ptr<ISourceExecutionNode<Type>>;

}  // namespace cvs::pipeline
