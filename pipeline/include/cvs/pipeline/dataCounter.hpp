#pragma once

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/tools/fpsCounter.hpp>

namespace cvs::pipeline {

class DataCounter {
 public:
  CVS_CONFIG(Settings, "") {
    CVS_CONFIG(Counter, "Processed data counter settings.") {
      CVS_FIELD(name, std::string,
                "Counter name. Used as a key linking the start and end of processing. Creates a logger with the same "
                "name by default. ");
      CVS_FIELD(type, bool, "Start or end of counter.");
    };
    CVS_FIELD_OPT(before, Counter, "Before of data processing.");
    CVS_FIELD_OPT(after, Counter, "After of data processing.");
  };

  static void init(DataCounter &, const common::Properties &, const common::FactoryPtr<std::string> &);

  virtual ~DataCounter() = default;

 protected:
  void beforeProcessing() const;
  void afterProcessing() const;

 protected:
  Settings settings;

  logger::tools::IFPSCounterPtr before_counter;
  logger::tools::IFPSCounterPtr after_counter;
};

}  // namespace cvs::pipeline
