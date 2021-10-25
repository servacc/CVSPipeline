#include "../../include/cvs/pipeline/impl/guiPipeline.hpp"

#include <cvs/common/config.hpp>
#include <cvs/logger/logging.hpp>

#include <exception>

namespace {

CVS_CONFIG(ViewConfig, "") { CVS_FIELD(type, std::string, ""); };

}  // namespace

namespace cvs::pipeline::impl {

IPipelineUPtr GuiPipeline::make(const common::Properties &cfg, const cvs::common::FactoryPtr<std::string> &factory) {
  std::unique_ptr<GuiPipeline> pipeline{new GuiPipeline};

  initPipeline(*pipeline, cfg, factory);

  auto view_cfg    = cfg.get_child("view");
  auto view_params = ViewConfig::make(view_cfg).value();

  pipeline->view = factory
                       ->create<IViewUPtr, const common::Properties &, IExecutionGraphPtr &,
                                const std::map<std::string, IExecutionNodePtr> &>(view_params.type, view_cfg,
                                                                                  pipeline->graph, pipeline->nodes)
                       .value();

  return pipeline;
}

int GuiPipeline::exec() {
  LOG_DEBUG(logger(), "Starting...");
  onStarted();

  LOG_INFO(logger(), "Started");

  auto result = view->exec();

  waitForAll();

  LOG_DEBUG(logger(), "Stopping...");
  onStopped();

  LOG_INFO(logger(), "Stopped");

  return result;
}

}  // namespace cvs::pipeline::impl
