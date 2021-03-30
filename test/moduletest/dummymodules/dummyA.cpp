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

class AElement : public IElement<int(bool*)> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<AElement>();
    return e;
  }

  int process(bool* stop) override {
    static int cnt = 0;
    *stop          = cnt++ != 0;
    return 10;
  }
};

class BElement : public IElement<int(int)> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<BElement>();
    return e;
  }

  int process(int a) override { return a; }
};

class DummyA : public cvs::pipeline::IModule {
 public:
  std::string name() const override { return "DummyA"; }
  int         version() const override { return 0; }
  void        registerTypes(cvs::common::FactoryPtr<std::string> factory) const override {
    registrateBase(factory);

    registrateElemetAndTbbHelper<IElementUPtr<int(bool*)>(common::Config&), AElement>("A"s, factory);
    registrateElemetAndTbbHelper<IElementUPtr<int(int)>(common::Config&), BElement>("B"s, factory);
  }
};

REGISTER_MODULE(DummyA)
