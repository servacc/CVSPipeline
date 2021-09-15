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

class AElement : public IElement<int()>, cvs::logger::Loggable<AElement> {
 public:
  static auto make(const common::Properties &) { return std::make_unique<AElement>(); }

  int process() override {
    LOG_INFO(logger(), "{}", cnt);
    ++cnt;
    return 10;
  }

  bool isStopped() const override { return cnt != 0; }

  int cnt = 0;
};

class BElement : public IElement<int(int)>, cvs::logger::Loggable<BElement> {
 public:
  static auto make(const common::Properties &) { return std::make_unique<BElement>(); }

  int process(int a) override {
    LOG_INFO(logger(), "{}", a);
    return a;
  }
};

class DummyA : public cvs::pipeline::IModule {
 public:
  std::string name() const override { return "DummyA"; }
  int         version() const override { return 0; }
  void        registerTypes(const cvs::common::FactoryPtr<std::string> &factory) const override {
    registerBase(factory);

    registerElemetAndTbbHelper<IElementUPtr<int()>(const common::Properties &), AElement>("A"s, factory);
    registerElemetAndTbbHelper<IElementUPtr<int(int)>(const common::Properties &), BElement>("B"s, factory);
  }
};

REGISTER_MODULE(DummyA)
