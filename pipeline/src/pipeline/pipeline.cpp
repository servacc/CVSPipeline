#include "../../include/cvs/pipeline/impl/pipeline.hpp"

#include <cvs/common/configbase.hpp>
#include <cvs/logger/logging.hpp>

#include <exception>

namespace {

CVSCFG_DECLARE_CONFIG(PipelineConfig, CVSCFG_VALUE_DEFAULT(autostart, bool, false))

CVSCFG_DECLARE_CONFIG(GraphConfig, CVSCFG_VALUE(type, std::string))
CVSCFG_DECLARE_CONFIG(NodeConfig,
                      CVSCFG_VALUE(node, std::string),
                      CVSCFG_VALUE(element, std::string),
                      CVSCFG_VALUE(name, std::string))

CVSCFG_DECLARE_CONFIG(ConnectionConfig,
                      CVSCFG_VALUE(from, std::string),
                      CVSCFG_VALUE(to, std::string),
                      CVSCFG_VALUE(output, std::size_t),
                      CVSCFG_VALUE(input, std::size_t))

}  // namespace

namespace cvs::pipeline::impl {

IPipelineUPtr Pipeline::make(common::Config &pipeline_cfg, const cvs::common::FactoryPtr<std::string> &factory) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.Pipeline");

  std::unique_ptr<Pipeline> pipeline{new Pipeline};
  initPipeline(*pipeline, pipeline_cfg, factory);

  return pipeline;
}

void Pipeline::initPipeline(Pipeline &                                  pipeline,
                            common::Config &                            cfg,
                            const cvs::common::FactoryPtr<std::string> &factory) {
  auto graph_cfg = cfg.getFirstChild("graph").value();
  auto graph     = parseGraph(graph_cfg, factory);

  auto nodes_list = cfg.getFirstChild("nodes").value();
  auto nodes      = parseNodes(nodes_list, factory, graph);

  auto connection_list = cfg.getFirstChild("connections").value();
  parseConnections(connection_list, nodes);

  auto params = cfg.parse<PipelineConfig>();

  pipeline.autostart = params->autostart;
  pipeline.graph     = std::move(graph);
  pipeline.nodes     = std::move(nodes);
}

IExecutionGraphPtr Pipeline::parseGraph(common::Config &cfg, const cvs::common::FactoryPtr<std::string> &factory) {
  auto graph_cfg = cfg.parse<GraphConfig>().value();
  return factory->create<IExecutionGraphUPtr>(graph_cfg.type).value();
}

Pipeline::NodesMap Pipeline::parseNodes(common::Config &                            nodes_list,
                                        const cvs::common::FactoryPtr<std::string> &factory,
                                        IExecutionGraphPtr &                        graph) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.Pipeline");

  std::map<std::string, IExecutionNodePtr> nodes;
  for (auto &node_cfg : nodes_list.getChildren()) {
    auto node_params = node_cfg.parse<NodeConfig>().value();

    LOG_TRACE(logger, R"s(Try create node "{}" with element "{}" and type "{}".)s", node_params.name, node_params.node,
              node_params.element);

    auto node = factory
                    ->create<IExecutionNodeUPtr, const std::string &, common::Config &, IExecutionGraphPtr &>(
                        node_params.element, node_params.node, node_cfg, graph)
                    .value();

    if (!node)
      throw std::runtime_error(fmt::format(R"(Can't create node "{}" with element "{}" and type "{}".)",
                                           node_params.name, node_params.node, node_params.element));

    nodes.emplace(node_params.name, std::move(node));

    LOG_DEBUG(logger, R"s(Node "{}" with element "{}" and type "{}" created.)s", node_params.name, node_params.node,
              node_params.element);
  }

  return nodes;
}

void Pipeline::parseConnections(common::Config &connection_list, const NodesMap &nodes) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.Pipeline");

  for (auto &connection_cfg : connection_list.getChildren()) {
    auto connection_params = connection_cfg.parse<ConnectionConfig>().value();

    LOG_TRACE(logger, R"(Try connect node "{}:{}"-"{}:{}")", connection_params.from, connection_params.output,
              connection_params.to, connection_params.input);

    auto from = nodes.at(connection_params.from).get();
    if (!from)
      throw std::runtime_error("Can't find node " + connection_params.from);

    auto to = nodes.at(connection_params.to).get();
    if (!to)
      throw std::runtime_error("Can't find node " + connection_params.to);

    auto sender = from->sender(connection_params.output);
    if (!sender.has_value())
      throw std::runtime_error("No sender with index " + std::to_string(connection_params.output));
    if (!to->connect(sender, connection_params.input))
      throw std::runtime_error("Can't connect output " + std::to_string(connection_params.output) + " with input " +
                               std::to_string(connection_params.input));

    LOG_DEBUG(logger, R"s(Nodes "{}" and "{}" connected ({}-{}).)s", connection_params.from, connection_params.to,
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
