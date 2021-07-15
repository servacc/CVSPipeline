#pragma once

#include <cvs/common/config.hpp>
#include <cvs/pipeline/tbb/tbbBufferTemplateNode.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

template <NodeType Type, typename T>
class TbbBroadcastNode : public TbbBufferTemplateNode<Type, ::tbb::flow::broadcast_node, T> {
 public:
  static auto make(common::Config&, IExecutionGraphPtr graph, std::shared_ptr<T>) {
    if (auto g = std::dynamic_pointer_cast<TbbFlowGraph>(graph))
      return std::make_unique<TbbBroadcastNode>(g);
    return std::unique_ptr<TbbBroadcastNode>{};
  }

  TbbBroadcastNode(const TbbFlowGraphPtr& graph)
      : TbbBufferTemplateNode<Type, ::tbb::flow::broadcast_node, T>(graph) {}
};

template <typename T>
using TbbBroadcastNodeIn = TbbBroadcastNode<NodeType::ServiceIn, T>;
template <typename T>
using TbbBroadcastNodeOut = TbbBroadcastNode<NodeType::ServiceOut, T>;

}  // namespace cvs::pipeline::tbb
