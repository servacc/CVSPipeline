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

class AElement : public IElement<int(bool*)> {
 public:
  static auto make(common::Config) {
    auto e = std::make_unique<AElement>();

    EXPECT_CALL(*e, process(_))
        .WillOnce([](bool* stop) {
          *stop = false;
          return 10;
        })
        .WillOnce([](bool* stop) {
          *stop = true;
          return 10;
        });

    return e;
  }

  MOCK_METHOD(int, process, (bool*), (override));
};

class BElement : public IElement<int(int)> {
 public:
  static auto make(common::Config) {
    auto e = std::make_unique<BElement>();
    EXPECT_CALL(*e, process(10)).WillOnce([](int a) -> int { return a; });
    return e;
  }

  MOCK_METHOD(int, process, (int), (override));
};

class DummyA : public cvs::pipeline::IModule {
 public:
  std::string name() const override { return "DummyA"; }
  int         version() const override { return 0; }
  void        registerTypes() const override {
    registrateBase();

    registrateElemetAndTbbHelper<IElementUPtr<int(bool*)>(common::Config), AElement>("A"s);
    registrateElemetAndTbbHelper<IElementUPtr<int(int)>(common::Config), BElement>("B"s);
  }
};

REGISTER_MODULE(DummyA)
