#include "../../include/cvs/pipeline/dataCounter.hpp"

namespace cvs::pipeline {

void DataCounter::init(DataCounter &                          counter,
                       const common::Properties &             properties,
                       const common::FactoryPtr<std::string> &factory) {
  const std::string fps_counter_key = "NodeCounter";

  auto counters_properties = properties.get_child_optional("node_counters");
  if (!counters_properties)
    return;

  counter.settings = Settings::make(*counters_properties).value();

  if (counter.settings.before) {
    const auto before_properties = counters_properties->get_child("before");
    counter.before_counter = factory->create<logger::tools::IFPSCounterPtr>(fps_counter_key, before_properties).value();
  }

  if (counter.settings.after) {
    const auto after_properties = counters_properties->get_child("after");
    counter.after_counter = factory->create<logger::tools::IFPSCounterPtr>(fps_counter_key, after_properties).value();
  }
}

void DataCounter::beforeProcessing() const {
  if (!before_counter)
    return;

  if (settings.before->type)
    before_counter->newFrame();
  else
    before_counter->frameProcessed();
}

void DataCounter::afterProcessing() const {
  if (!after_counter)
    return;

  if (settings.after->type)
    after_counter->newFrame();
  else
    after_counter->frameProcessed();
}

}  // namespace cvs::pipeline
