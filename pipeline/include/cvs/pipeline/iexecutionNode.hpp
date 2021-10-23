#pragma once

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/loggable.hpp>
#include <cvs/logger/tools/fpsCounter.hpp>
#include <cvs/pipeline/iexecutionGraph.hpp>

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

CVS_CONFIG(NodeInfo, "") {
  CVS_FIELD(name, std::string, "Node name.");
  CVS_FIELD(element, std::string, "Element type.");
  CVS_FIELD(node, std::string, "Node type.");
};

CVS_CONFIG(FunctionNodeConfig, "") {
  CVS_FIELD_DEF(concurrency, std::size_t, 0, "");
  CVS_FIELD_DEF(priority, unsigned int, 0, "");
};

class IExecutionNode : public cvs::logger::Loggable<IExecutionNode> {
 public:
  IExecutionNode()
      : cvs::logger::Loggable<IExecutionNode>("cvs.pipeline.ExecutionNode") {}
  virtual ~IExecutionNode() = default;

  virtual NodeType type() const = 0;

  const std::string &name() const { return info.name; }
  const std::string &element() const { return info.element; }
  const std::string &nodeName() const { return info.node; }

  virtual std::any receiver(std::size_t index) = 0;
  virtual std::any sender(std::size_t index)   = 0;

  virtual bool connect(std::any, std::size_t index) = 0;

 protected:
  template <typename Node, typename Graph>
  static std::unique_ptr<Node> createNode(const common::Properties &properties, IExecutionGraphPtr graph) {
    auto g = std::dynamic_pointer_cast<Graph>(graph);
    if (!g)
      cvs::common::throwException<std::runtime_error>(R"(The type of the graph must be "{}".)",
                                                      boost::core::demangle(typeid(Graph).name()));

    auto info = NodeInfo::make(properties).value();

    auto node  = std::make_unique<Node>(g);
    node->info = std::move(info);

    return node;
  }

 protected:
  NodeInfo info;
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
