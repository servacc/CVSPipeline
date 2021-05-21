#include "../../include/cvs/pipeline/impl/guiPipeline.hpp"

#include <cvs/common/configbase.hpp>
#include <cvs/logger/logging.hpp>

#include <exception>

namespace {

CVSCFG_DECLARE_CONFIG(ViewConfig, CVSCFG_VALUE_DEFAULT(autostart, bool, false))

}  // namespace

namespace cvs::pipeline::impl {

IPipelineUPtr GuiPipeline::make(common::Config &cfg, const cvs::common::FactoryPtr<std::string> &factory) {
  std::unique_ptr<GuiPipeline> pipeline{new GuiPipeline};

  initPipeline(*pipeline, cfg, factory);

  auto view = cfg.getFirstChild("view").value();

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
