#include <cvs/common/config.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/imodule.hpp>
#include <cvs/pipeline/registrationhelper.hpp>
#include <cvs/pipeline/tbb/tbbhelpers.hpp>
#include <gmock/gmock.h>

using namespace std::string_literals;
using namespace cvs::pipeline::tbb;
using namespace cvs;
using namespace cvs::pipeline;
using namespace cvs::common;
using testing::_;

class EElement : public IElement<void(float)> {
 public:
  static auto make(common::Config) {
    auto e = std::make_unique<EElement>();
    EXPECT_CALL(*e, process(10.1f));
    return e;
  }

  MOCK_METHOD(void, process, (float), (override));
};

class DummyC : public cvs::pipeline::IModule {
 public:
  std::string name() const override { return "DummyC"; }
  int         version() const override { return 0; }
  void        registerTypes() const override {
    registrateBase();

    registrateElemetAndTbbHelper<IElementUPtr<void(float)>(common::Config), EElement>("E"s);
  }
};

REGISTER_MODULE(DummyC)
