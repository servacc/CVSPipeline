#pragma once

#include <cvs/common/config.hpp>
#include <cvs/pipeline/tbb/tbbBufferTemplateNode.hpp>
#include <tbb/flow_graph.h>

namespace cvs::pipeline::tbb {

template <NodeType Type, typename T>
class TbbQueueNode : public TbbBufferTemplateNode<Type, ::tbb::flow::queue_node, T> {
 public:
  static auto make(const common::Properties& properties,
                   IExecutionGraphPtr        graph,
                   const common::FactoryPtr<std::string>&,
                   std::shared_ptr<T>) {
    return createNode<TbbQueueNode<Type, T>, TbbFlowGraph>(properties, std::move(graph));
  }

  TbbQueueNode(TbbFlowGraphPtr graph)
      : TbbBufferTemplateNode<Type, ::tbb::flow::queue_node, T>(graph) {}
};

template <typename T>
using TbbQueueNodeIn = TbbQueueNode<NodeType::ServiceIn, T>;
template <typename T>
using TbbQueueNodeOut = TbbQueueNode<NodeType::ServiceOut, T>;

}  // namespace cvs::pipeline::tbb
