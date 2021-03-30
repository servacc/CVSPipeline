#include <cvs/common/config.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/imodule.hpp>
#include <cvs/pipeline/registrationhelper.hpp>
#include <cvs/pipeline/tbb/tbbhelpers.hpp>

using namespace std::string_literals;
using namespace cvs::pipeline::tbb;
using namespace cvs;
using namespace cvs::pipeline;
using namespace cvs::common;

class EElement : public IElement<void(float)> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<EElement>();
    return e;
  }

  void process(float) override {}
};

class DummyC : public cvs::pipeline::IModule {
 public:
  std::string name() const override { return "DummyC"; }
  int         version() const override { return 0; }
  void        registerTypes(cvs::common::FactoryPtr<std::string> factory) const override {
    registrateBase(factory);

    registrateElemetAndTbbHelper<IElementUPtr<void(float)>(common::Config&), EElement>("E"s, factory);
  }
};

REGISTER_MODULE(DummyC)
