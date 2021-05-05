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

class AElement : public IElement<int(bool*)>, cvs::logger::Loggable<AElement> {
 public:
  static auto make(common::Config&) { return std::make_unique<AElement>(); }

  int process(bool* stop) override {
    static int cnt = 0;
    LOG_INFO(logger(), "{}", cnt);
    *stop = cnt++ != 0;
    return 10;
  }
};

class BElement : public IElement<int(int)>, cvs::logger::Loggable<BElement> {
 public:
  static auto make(common::Config&) { return std::make_unique<BElement>(); }

  int process(int a) override {
    LOG_INFO(logger(), "{}", a);
    return a;
  }
};

class DummyA : public cvs::pipeline::IModule {
 public:
  std::string name() const override { return "DummyA"; }
  int         version() const override { return 0; }
  void        registerTypes(cvs::common::FactoryPtr<std::string> factory) const override {
    registerBase(factory);

    registerElemetAndTbbHelper<IElementUPtr<int(bool*)>(common::Config&), AElement>("A"s, factory);
    registerElemetAndTbbHelper<IElementUPtr<int(int)>(common::Config&), BElement>("B"s, factory);
  }
};

REGISTER_MODULE(DummyA)
