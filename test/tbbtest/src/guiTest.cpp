#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/pipeline/ielement.hpp>
#include <cvs/pipeline/tbb/tbbHelpers.hpp>
#include <cvs/pipeline/tbb/tbbView.hpp>
#include <gmock/gmock.h>

using namespace std::literals::string_literals;

using namespace cvs;
using namespace cvs::pipeline;
using namespace cvs::pipeline::tbb;

using ::testing::_;

namespace {

CVS_CONFIG(InOutConfig, "") { CVS_FIELD_DEF(value, int, 2, ""); };

class InOut : public IElement<int(int)> {
 public:
  static auto make(const common::Properties& cfg) {
    auto param = InOutConfig::make(cfg);
    auto e     = std::make_unique<InOut>();
    EXPECT_CALL(*e, process(_)).WillOnce([mul = param->value](int a) -> int { return a * mul; });
    return e;
  }

  MOCK_METHOD(int, process, (int), (override));
};

class Process : public IElement<int(int, int)> {
 public:
  static auto make(const common::Properties&) {
    auto e = std::make_unique<Process>();
    EXPECT_CALL(*e, process(_, _)).WillOnce([](int a, int b) -> int { return a + b; });
    return e;
  }

  MOCK_METHOD(int, process, (int, int), (override));
};

class TestView : public TbbView<std::tuple<int, int>, std::tuple<int, int>> {
 public:
  TestView(const common::Properties& cfg, cvs::pipeline::IExecutionGraphPtr g)
      : TbbView<std::tuple<int, int>, std::tuple<int, int>>(cfg, g) {}

  int exec() override {
    tryPut<0>(a_input);
    tryPut<1>(b_input);

    return 0;
  }

  bool check() {
    auto result_0 = tryGet<0>(d_output);
    auto result_1 = tryGet<1>(e_output);

    return result_0 && result_1;
  }

  int a_input = 5;
  int b_input = 10;

  int d_output = -1;
  int e_output = -1;
};

}  // namespace

namespace {

class GuiTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {
    cvs::logger::initLoggers();

    registerBase(factory);

    registerElemetAndTbbHelper<IElementUPtr<int(int)>(const common::Properties&), InOut>("InOut"s, factory);
    registerElemetAndTbbHelper<IElementUPtr<int(int, int)>(const common::Properties&), Process>("Process"s, factory);
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
  //     |   |
  //    Buf Buf

  std::string config_1_json = R"({ "value": 1, "name" : "TestName", "element" : "TestElement", "node" : "TestNode" })";
  std::string config_3_json = R"({ "value": 3, "name" : "TestName", "element" : "TestElement", "node" : "TestNode" })";
  std::string config_2_json = R"({ "name" : "TestName", "element" : "TestElement", "node" : "TestNode" })";

  const auto cfg_1 = cvs::common::CVSConfigBase::load(config_1_json);
  const auto cfg_3 = cvs::common::CVSConfigBase::load(config_3_json);
  const auto cfg   = cvs::common::CVSConfigBase::load(config_2_json);

  IExecutionGraphPtr graph = factory->create<IExecutionGraphUPtr>(TbbDefaultName::graph).value();
  ASSERT_NE(nullptr, graph);

  try {
    auto a_node = factory->create<IExecutionNodeUPtr>("InOut"s, TbbDefaultName::function, cfg_1, graph).value();
    auto b_node = factory->create<IExecutionNodeUPtr>("InOut"s, TbbDefaultName::function, cfg_1, graph).value();
    auto c_node = factory->create<IExecutionNodeUPtr>("Process"s, TbbDefaultName::function, cfg, graph).value();
    auto d_node = factory->create<IExecutionNodeUPtr>("InOut"s, TbbDefaultName::function, cfg_3, graph).value();
    auto e_node = factory->create<IExecutionNodeUPtr>("InOut"s, TbbDefaultName::function, cfg, graph).value();

    auto bc_node = factory->create<IExecutionNodeUPtr>("Process"s, TbbDefaultName::broadcast_out, cfg, graph).value();
    auto j_node  = factory->create<IExecutionNodeUPtr>("Process"s, TbbDefaultName::join, cfg, graph).value();

    auto d_buf_node = factory->create<IExecutionNodeUPtr>("InOut"s, TbbDefaultName::buffer_out, cfg, graph).value();
    auto e_buf_node = factory->create<IExecutionNodeUPtr>("InOut"s, TbbDefaultName::buffer_out, cfg, graph).value();

    TestView view(cfg, graph);

    view.addReceiver(0, a_node->receiver(0));
    view.addReceiver(1, b_node->receiver(0));

    view.addSender(0, d_buf_node->sender(0));
    view.addSender(1, e_buf_node->sender(0));

    ASSERT_TRUE(j_node->connect(a_node->sender(0), 0));
    ASSERT_TRUE(j_node->connect(b_node->sender(0), 1));
    ASSERT_TRUE(c_node->connect(j_node->sender(0), 0));
    ASSERT_TRUE(bc_node->connect(c_node->sender(0), 0));
    ASSERT_TRUE(d_node->connect(bc_node->sender(0), 0));
    ASSERT_TRUE(e_node->connect(bc_node->sender(0), 0));
    ASSERT_TRUE(d_buf_node->connect(d_node->sender(0), 0));
    ASSERT_TRUE(e_buf_node->connect(e_node->sender(0), 0));

    view.exec();
    graph->waitForAll();

    EXPECT_TRUE(view.check());

    EXPECT_EQ((view.a_input + view.b_input) * 3, view.d_output);
    EXPECT_EQ((view.a_input + view.b_input) * 2, view.e_output);
  }
  catch (std::exception& e) {
    std::cout << cvs::common::exceptionStr(e) << std::endl;
  }
}

}  // namespace
