#include <cvs/common/factory.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/tbb/tbbhelpers.hpp>
#include <cvs/pipeline/tbb/tbbview.hpp>
#include <gmock/gmock.h>

using namespace std::literals::string_literals;

using namespace cvs;
using namespace cvs::pipeline;
using namespace cvs::pipeline::tbb;

using ::testing::_;

namespace {

class InOut : public IElement<int(int)> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<InOut>();
    EXPECT_CALL(*e, process(_)).WillOnce([](int a) -> int { return a * 2; });
    return e;
  }

  MOCK_METHOD(int, process, (int), (override));
};

class Process : public IElement<int(int, int)> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<Process>();
    EXPECT_CALL(*e, process(_, _)).WillOnce([](int a, int b) -> int { return a + b; });
    return e;
  }

  MOCK_METHOD(int, process, (int, int), (override));
};

class TestView : public TbbView<std::tuple<int, int>, std::tuple<int, int>> {
 public:
  int exec() override {
    tryPut<0>(5);
    tryPut<1>(10);

    return 0;
  }

  bool check() {
    tryPut<0>(5);
    tryPut<1>(10);
  }
};

}  // namespace

namespace {

class GuiTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {
    cvs::logger::initLoggers();

    registerBase(factory);

    registerElemetAndTbbHelper<IElementUPtr<int(int)>(common::Config&), InOut>("InOut"s, factory);
    registerElemetAndTbbHelper<IElementUPtr<int(int, int)>(common::Config&), Process>("Process"s, factory);
  }

  static cvs::common::FactoryPtr<std::string> factory;
};

cvs::common::FactoryPtr<std::string> GuiTest::factory = std::make_shared<cvs::common::Factory<std::string>>();

TEST_F(GuiTest, in_out) {
  // Graph:
  //     A   B
  //      \ /
  //     join
  //       |
  //       C
  //       |
  //   Broadcast
  //      / \
  //     D   E

  common::Config cfg;

  IExecutionGraphPtr graph = factory->create<IExecutionGraphUPtr>(TbbDefaultName::graph).value_or(nullptr);
  ASSERT_NE(nullptr, graph);

  auto a_node = factory->create<IExecutionNodeUPtr>("InOut"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);
  auto b_node = factory->create<IExecutionNodeUPtr>("InOut"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);
  auto c_node = factory->create<IExecutionNodeUPtr>("Process"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);
  auto d_node = factory->create<IExecutionNodeUPtr>("InOut"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);
  auto e_node = factory->create<IExecutionNodeUPtr>("InOut"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);

  auto bc_node =
      factory->create<IExecutionNodeUPtr>("Process"s, TbbDefaultName::broadcast, cfg, graph).value_or(nullptr);
  auto j_node = factory->create<IExecutionNodeUPtr>("Process"s, TbbDefaultName::join, cfg, graph).value_or(nullptr);

  //  TbbView<std::tuple<int, int>, std::tuple<int, int>> view;

  ASSERT_NE(nullptr, a_node);
  ASSERT_NE(nullptr, b_node);
  ASSERT_NE(nullptr, c_node);
  ASSERT_NE(nullptr, d_node);
  ASSERT_NE(nullptr, e_node);

  ASSERT_NE(nullptr, bc_node);
  ASSERT_NE(nullptr, j_node);

  TestView view;

  view.addReceiver(0, a_node->receiver(0));
  view.addReceiver(1, b_node->receiver(0));

  view.addSender(0, d_node->sender(0));
  view.addSender(1, e_node->sender(0));

  ASSERT_TRUE(j_node->connect(a_node->sender(0), 0));
  ASSERT_TRUE(j_node->connect(b_node->sender(0), 1));
  ASSERT_TRUE(c_node->connect(j_node->sender(0), 0));
  ASSERT_TRUE(bc_node->connect(c_node->sender(0), 0));
  ASSERT_TRUE(d_node->connect(bc_node->sender(0), 0));
  ASSERT_TRUE(e_node->connect(bc_node->sender(0), 0));

  view.exec();
  graph->waitForAll();

  //  ASSERT_TRUE(bc_node->connect(a_node->sender(0), 0));
}

}  // namespace
