#include <cvs/common/config.hpp>
#include <cvs/logger/loggable.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/imodule.hpp>
#include <cvs/pipeline/registrationhelper.hpp>
#include <cvs/pipeline/tbb/tbbhelpers.hpp>

using namespace std::string_literals;
using namespace cvs::pipeline::tbb;
using namespace cvs;
using namespace cvs::pipeline;
using namespace cvs::common;

class CElement : public IElement<float(int)>, public cvs::logger::Loggable<CElement> {
 public:
  static auto make(common::Config&) { return std::make_unique<CElement>(); }

  float process(int a) override {
    LOG_INFO(logger(), "{}", a);
    return a / 100.f;
  }
};

class DElement : public IElement<float(int, float)>, public cvs::logger::Loggable<DElement> {
 public:
  static auto make(common::Config&) { return std::make_unique<DElement>(); }

  float process(int a, float b) override {
    LOG_INFO(logger(), "{} {}", a, b);
    return a + b;
  }
};

class DummyB : public cvs::pipeline::IModule {
 public:
  std::string name() const override { return "DummyB"; }
  int         version() const override { return 0; }
  void        registerTypes(cvs::common::FactoryPtr<std::string> factory) const override {
    registerBase(factory);

    registerElemetAndTbbHelper<IElementUPtr<float(int)>(common::Config&), CElement>("C"s, factory);
    registerElemetAndTbbHelper<IElementUPtr<float(int, float)>(common::Config&), DElement>("D"s, factory);
  }
};

REGISTER_MODULE(DummyB)
