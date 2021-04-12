#include "pipeline.hpp"

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

IPipelineUPtr Pipeline::make(common::Config &root, const cvs::common::FactoryPtr<std::string> &factory) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.Pipeline");

  auto pipeline_cfg = root.getFirstChild("Pipeline").value();

  auto params = pipeline_cfg.parse<PipelineConfig>();

  auto               graph_cfg = pipeline_cfg.getFirstChild("graph")->parse<GraphConfig>().value();
  IExecutionGraphPtr graph     = factory->create<IExecutionGraphUPtr>(graph_cfg.type).value();

  auto nodes_list = pipeline_cfg.getFirstChild("nodes").value();

  std::map<std::string, IExecutionNodePtr> nodes;
  for (auto &node_cfg : nodes_list.getChildren()) {
    auto node_params = node_cfg.parse<NodeConfig>().value();

    auto node = factory
                    ->create<IExecutionNodeUPtr, const std::string &, common::Config &, IExecutionGraphPtr &>(
                        node_params.element, node_params.node, node_cfg, graph)
                    .value();
    nodes.emplace(node_params.name, std::move(node));

    LOG_DEBUG(logger, R"s(Node "{}" with element "{}" created.)s", node_params.node, node_params.element);
  }

  auto connection_list = pipeline_cfg.getFirstChild("Connections").value();
  for (auto &connection_cfg : connection_list.getChildren()) {
    auto connection_params = connection_cfg.parse<ConnectionConfig>().value();

    auto from = nodes[connection_params.from].get();
    if (!from)
      throw std::runtime_error("Can't find node " + connection_params.from);

    auto to = nodes[connection_params.to].get();
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

  auto pipeline = std::make_unique<Pipeline>();

  pipeline->autostart = params->autostart;
  pipeline->graph     = std::move(graph);
  pipeline->nodes     = std::move(nodes);

  return pipeline;
}

Pipeline::Pipeline()
    : cvs::logger::Loggable<Pipeline>("cvs.pipeline.Pipeline") {}

void Pipeline::start() {
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

void Pipeline::stop() {}

IExecutionNodePtr Pipeline::getNode(std::string_view name) const {
  auto iter = nodes.find(std::string{name});
  if (iter != nodes.end())
    return iter->second;
  return {};
}

void Pipeline::waitForAll() { graph->waitForAll(); }

}  // namespace cvs::pipeline::impl
