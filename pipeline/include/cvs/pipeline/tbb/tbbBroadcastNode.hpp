#pragma once

#include <cvs/common/config.hpp>
#include <cvs/pipeline/tbb/tbbBufferTemplateNode.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

template <NodeType Type, typename T>
class TbbBroadcastNode : public TbbBufferTemplateNode<Type, ::tbb::flow::broadcast_node, T> {
 public:
  static auto make(const common::Properties& properties,
                   IExecutionGraphPtr        graph,
                   const common::FactoryPtr<std::string>&,
                   std::shared_ptr<T>) {
    return createNode<TbbBroadcastNode<Type, T>, TbbFlowGraph>(properties, std::move(graph));
  }

  TbbBroadcastNode(const TbbFlowGraphPtr& graph)
      : TbbBufferTemplateNode<Type, ::tbb::flow::broadcast_node, T>(graph) {}
};

template <typename T>
using TbbBroadcastNodeIn = TbbBroadcastNode<NodeType::ServiceIn, T>;
template <typename T>
using TbbBroadcastNodeOut = TbbBroadcastNode<NodeType::ServiceOut, T>;

}  // namespace cvs::pipeline::tbb
