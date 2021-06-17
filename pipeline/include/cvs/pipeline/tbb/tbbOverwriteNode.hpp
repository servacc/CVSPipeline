#pragma once

#include <cvs/common/config.hpp>
#include <cvs/pipeline/tbb/tbbBufferTemplateNode.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {
template <NodeType Type, typename T>
class TbbOverwriteNode : public TbbBufferTemplateNode<Type, ::tbb::flow::buffer_node, T> {
 public:
  static auto make(common::Config&, IExecutionGraphPtr graph, std::shared_ptr<T>) {
    if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph))
      return std::make_unique<TbbOverwriteNode>(g);
    return std::unique_ptr<TbbOverwriteNode>{};
  }

  TbbOverwriteNode(TbbFlowGraphPtr graph)
      : TbbBufferTemplateNode<Type, ::tbb::flow::buffer_node, T>(graph) {}
};

template <typename T>
using TbbOverwriteNodeIn = TbbOverwriteNode<NodeType::ServiceIn, T>;
template <typename T>
using TbbOverwriteNodeOut = TbbOverwriteNode<NodeType::ServiceOut, T>;

}  // namespace cvs::pipeline::tbb
