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

class CElement : public IElement<float(int)> {
 public:
  static auto make(common::Config) {
    auto e = std::make_unique<CElement>();
    EXPECT_CALL(*e, process(10)).WillOnce([](int a) -> float { return a / 100.f; });
    return e;
  }

  MOCK_METHOD(float, process, (int), (override));
};

class DElement : public IElement<float(int, float)> {
 public:
  static auto make(common::Config) {
    auto e = std::make_unique<DElement>();
    EXPECT_CALL(*e, process(10, 0.1f)).WillOnce([](int a, float b) -> float { return a + b; });
    return e;
  }

  MOCK_METHOD(float, process, (int, float), (override));
};

class DummyB : public cvs::pipeline::IModule {
 public:
  std::string name() const override { return "DummyB"; }
  int         version() const override { return 0; }
  void        registerTypes() const override {
    registrateBase();

    registrateElemetAndTbbHelper<IElementUPtr<float(int)>(common::Config), CElement>("C"s);
    registrateElemetAndTbbHelper<IElementUPtr<float(int, float)>(common::Config), DElement>("D"s);
  }
};

REGISTER_MODULE(DummyB)
