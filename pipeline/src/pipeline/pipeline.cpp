#include "../../include/cvs/pipeline/impl/pipeline.hpp"

#include <cvs/logger/logging.hpp>
#include <fmt/format.h>

#include <exception>

namespace {

CVS_CONFIG(PipelineConfig, "") { CVS_FIELD_DEF(autostart, bool, false, ""); };

CVS_CONFIG(GraphConfig, "") { CVS_FIELD(type, std::string, ""); };

CVS_CONFIG(ConnectionConfig, "") {
  CVS_FIELD(from, std::string, "");
  CVS_FIELD(to, std::string, "");
  CVS_FIELD_DEF(output, std::size_t, 0, "");
  CVS_FIELD_DEF(input, std::size_t, 0, "");
};

}  // namespace

namespace cvs::pipeline::impl {

IPipelineUPtr Pipeline::make(const common::Properties &                  pipeline_cfg,
                             const cvs::common::FactoryPtr<std::string> &factory) {
  try {
    std::unique_ptr<Pipeline> pipeline{new Pipeline};
    initPipeline(*pipeline, pipeline_cfg, factory);

    return pipeline;
  }
  catch (...) {
    cvs::common::throwWithNested<std::runtime_error>("Can't init Pipeline.");
  }
}

void Pipeline::initPipeline(Pipeline &                                  pipeline,
                            const common::Properties &                  cfg,
                            const cvs::common::FactoryPtr<std::string> &factory) {
  auto graph = parseGraph(cfg.get_child("graph"), factory);
  auto nodes = parseNodes(cfg.get_child("nodes"), factory, graph);

  parseConnections(cfg.get_child("connections"), nodes);

  auto params = PipelineConfig::make(cfg);

  pipeline.autostart = params->autostart;
  pipeline.graph     = std::move(graph);
  pipeline.nodes     = std::move(nodes);
}

IExecutionGraphPtr Pipeline::parseGraph(const common::Properties &                  cfg,
                                        const cvs::common::FactoryPtr<std::string> &factory) {
  auto graph_cfg = GraphConfig::make(cfg);
  return factory->create<IExecutionGraphUPtr>(graph_cfg->type).value();
}

Pipeline::NodesMap Pipeline::parseNodes(const common::Properties &                  nodes_list,
                                        const cvs::common::FactoryPtr<std::string> &factory,
                                        IExecutionGraphPtr &                        graph) {
  auto logger = *cvs::logger::createLogger("cvs.pipeline.Pipeline");

  std::map<std::string, IExecutionNodePtr> nodes;
  try {
    for (const auto &node_cfg : nodes_list) {
      const auto node_params = NodeInfo::make(node_cfg.second).value();

      LOG_TRACE(logger, R"s(Try create node "{}" with element "{}" and type "{}".)s", node_params.name,
                node_params.node, node_params.element);

      auto node = factory->create<IExecutionNodeUPtr>(node_params.element, node_params.node, node_cfg.second, graph);

      nodes.emplace(node_params.name, std::move(node.value()));

      LOG_DEBUG(logger, R"s(Node "{}" with element "{}" and type "{}" created.)s", node_params.name, node_params.node,
                node_params.element);
    }
  }
  catch (...) {
    cvs::common::throwWithNested<std::runtime_error>("Can't create pipeline nodes.");
  }

  return nodes;
}

void Pipeline::parseConnections(const common::Properties &connection_list, const NodesMap &nodes) {
  auto logger = *cvs::logger::createLogger("cvs.pipeline.Pipeline");

  for (auto &connection_cfg : connection_list) {
    auto connection_params = ConnectionConfig::make(connection_cfg.second).value();

    LOG_TRACE(logger, R"(Try connect node "{}:{}"-"{}:{}")", connection_params.from, connection_params.output,
              connection_params.to, connection_params.input);

    auto from_iter = nodes.find(connection_params.from);
    if (from_iter == nodes.end())
      throw std::runtime_error("Can't find node " + connection_params.from);
    auto from = from_iter->second;

    auto to_iter = nodes.find(connection_params.to);
    if (to_iter == nodes.end())
      throw std::runtime_error("Can't find node " + connection_params.to);
    auto to = to_iter->second;

    auto sender = from->sender(connection_params.output);
    if (!sender.has_value())
      throw std::runtime_error("No sender with index " + std::to_string(connection_params.output));
    if (!to->connect(sender, connection_params.input))
      throw std::runtime_error(fmt::format(R"(Can't connect "{}" output {} with "{}" input {})", connection_params.from,
                                           connection_params.output, connection_params.to, connection_params.input));

    LOG_DEBUG(logger, R"(Nodes "{}" and "{}" connected ({}-{}).)", connection_params.from, connection_params.to,
              connection_params.output, connection_params.input);
  }
}

Pipeline::Pipeline()
    : cvs::logger::Loggable<Pipeline>("cvs.pipeline.Pipeline") {}

IExecutionNodePtr Pipeline::getNode(std::string_view name) const {
  auto iter = nodes.find(std::string{name});
  if (iter != nodes.end())
    return iter->second;
  return {};
}

void Pipeline::waitForAll() { graph->waitForAll(); }

int Pipeline::exec() {
  LOG_DEBUG(logger(), "Starting...");
  onStarted();

  LOG_INFO(logger(), "Started");

  waitForAll();

  LOG_DEBUG(logger(), "Stopping...");
  onStopped();

  LOG_INFO(logger(), "Stopped");

  return 0;
}

void Pipeline::onStarted() {
  if (!autostart) {
    LOG_TRACE(logger(), "Autostart disabled");
    return;
  }

  for (auto &n : nodes) {
    LOG_TRACE(logger(), R"s(Try activate node "{}...")s", n.first);
    if (n.second->type() != NodeType::Functional)
      continue;

    auto src = std::dynamic_pointer_cast<cvs::pipeline::ISourceExecutionNode<NodeType::Functional>>(n.second);
    if (!src)
      continue;

    src->activate();
    LOG_TRACE(logger(), R"s(Node "{}" activated)s", n.first);
  }
}

void Pipeline::onStopped() {}

}  // namespace cvs::pipeline::impl
