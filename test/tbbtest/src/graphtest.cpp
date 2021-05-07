#include <cvs/common/factory.hpp>
#include <cvs/pipeline/tbb/tbbhelpers.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std::literals::string_literals;
using namespace cvs;
using namespace cvs::pipeline;
using namespace cvs::pipeline::tbb;
using namespace cvs::common;

using ::testing::_;

namespace {

class AElement : public IElement<int()> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<AElement>();

    EXPECT_CALL(*e, process()).Times(2).WillRepeatedly([]() { return 10; });

    EXPECT_CALL(*e, isStopped()).WillOnce([]() { return false; }).WillOnce([]() { return true; });

    return e;
  }

  MOCK_METHOD(int, process, (), (override));
  MOCK_METHOD(bool, isStopped, (), (const override));
};

class BElement : public IElement<int(int)> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<BElement>();
    EXPECT_CALL(*e, process(10)).WillOnce([](int a) -> int { return a; });
    return e;
  }

  MOCK_METHOD(int, process, (int), (override));
};

class CElement : public IElement<float(int)> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<CElement>();
    EXPECT_CALL(*e, process(10)).WillOnce([](int a) -> float { return a / 100.f; });
    return e;
  }

  MOCK_METHOD(float, process, (int), (override));
};

class DElement : public IElement<float(int, float)> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<DElement>();
    EXPECT_CALL(*e, process(10, 0.1f)).WillOnce([](int a, float b) -> float { return a + b; });
    return e;
  }

  MOCK_METHOD(float, process, (int, float), (override));
};

class EElement : public IElement<void(float)> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<EElement>();
    EXPECT_CALL(*e, process(10.1f));
    return e;
  }

  MOCK_METHOD(void, process, (float), (override));
};

}  // namespace

namespace {

class GraphTest : public ::testing::Test {
 public:
  static void SetUpTestCase() {
    cvs::logger::initLoggers();

    registerBase(factory);

    registerElemetAndTbbHelper<IElementUPtr<int()>(common::Config&), AElement>("A"s, factory);
    registerElemetAndTbbHelper<IElementUPtr<int(int)>(common::Config&), BElement>("B"s, factory);
    registerElemetAndTbbHelper<IElementUPtr<float(int)>(common::Config&), CElement>("C"s, factory);
    registerElemetAndTbbHelper<IElementUPtr<float(int, float)>(common::Config&), DElement>("D"s, factory);
    registerElemetAndTbbHelper<IElementUPtr<void(float)>(common::Config&), EElement>("E"s, factory);
  }

  static cvs::common::FactoryPtr<std::string> factory;
};

cvs::common::FactoryPtr<std::string> GraphTest::factory = std::make_shared<cvs::common::Factory<std::string>>();

TEST_F(GraphTest, branch_graph) {
  // Graph:
  //       A
  //       |
  //   Broadcast
  //      / \
  //     B   C
  //      \ /
  //     join
  //       |
  //       D
  //       |
  //       E

  common::Config cfg;

  IExecutionGraphPtr graph = factory->create<IExecutionGraphUPtr>(TbbDefaultName::graph).value_or(nullptr);
  ASSERT_NE(nullptr, graph);

  auto a_node = factory->create<IExecutionNodeUPtr>("A"s, TbbDefaultName::source, cfg, graph).value_or(nullptr);
  auto b_node = factory->create<IExecutionNodeUPtr>("B"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);
  auto c_node = factory->create<IExecutionNodeUPtr>("C"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);
  auto d_node = factory->create<IExecutionNodeUPtr>("D"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);
  auto e_node = factory->create<IExecutionNodeUPtr>("E"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);

  auto bc_node = factory->create<IExecutionNodeUPtr>("A"s, TbbDefaultName::broadcast, cfg, graph).value_or(nullptr);
  auto j_node  = factory->create<IExecutionNodeUPtr>("D"s, TbbDefaultName::join, cfg, graph).value_or(nullptr);

  ASSERT_NE(nullptr, a_node);
  ASSERT_NE(nullptr, b_node);
  ASSERT_NE(nullptr, c_node);
  ASSERT_NE(nullptr, d_node);
  ASSERT_NE(nullptr, e_node);

  ASSERT_NE(nullptr, bc_node);
  ASSERT_NE(nullptr, j_node);

  ASSERT_TRUE(bc_node->connect(a_node->sender(0), 0));
  ASSERT_TRUE(b_node->connect(bc_node->sender(0), 0));
  ASSERT_TRUE(c_node->connect(bc_node->sender(0), 0));

  ASSERT_TRUE(j_node->connect(b_node->sender(0), 0));
  ASSERT_TRUE(j_node->connect(c_node->sender(0), 1));

  ASSERT_TRUE(d_node->connect(j_node->sender(0), 0));

  ASSERT_TRUE(e_node->connect(d_node->sender(0), 0));

  auto src_node =
      std::dynamic_pointer_cast<ISourceExecutionNode<NodeType::Functional>>(IExecutionNodePtr(std::move(a_node)));
  src_node->activate();

  graph->waitForAll();
}

}  // namespace
