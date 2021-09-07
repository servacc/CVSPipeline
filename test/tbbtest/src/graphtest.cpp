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

    EXPECT_CALL(*e, process()).Times(2).WillRepeatedly([]() {
      //
      return 10;
    });

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

class MultiElementA : public IElement<std::optional<int>(std::size_t, std::size_t)> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<MultiElementA>();
    EXPECT_CALL(*e, process(_, _))
        .WillOnce([](std::size_t, std::size_t) -> std::optional<int> { return std::nullopt; })
        .WillOnce([](std::size_t, std::size_t) -> std::optional<int> { return 0; });
    return e;
  }

  MOCK_METHOD(std::optional<int>, process, (std::size_t, std::size_t), (override));
};

class MultiElementB : public IElement<std::tuple<std::optional<int>, std::optional<int>>(std::size_t)> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<MultiElementB>();
    EXPECT_CALL(*e, process(1)).WillOnce([](std::size_t) -> std::tuple<std::optional<int>, std::optional<int>> {
      return {std::nullopt, 0};
    });
    EXPECT_CALL(*e, process(3)).WillOnce([](std::size_t) -> std::tuple<std::optional<int>, std::optional<int>> {
      return {0, std::nullopt};
    });
    return e;
  }

  MOCK_METHOD((std::tuple<std::optional<int>, std::optional<int>>), process, (std::size_t), (override));
};

class MultiElementResult : public IElement<void(int)> {
 public:
  static auto make(common::Config&) {
    auto e = std::make_unique<MultiElementResult>();
    EXPECT_CALL(*e, process(_));
    return e;
  }

  MOCK_METHOD(void, process, (int), (override));
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

    registerElemetAndTbbHelper<IElementUPtr<std::optional<int>(std::size_t, std::size_t)>(common::Config&),
                               MultiElementA>("MA"s, factory);
    registerElemetAndTbbHelper<
        IElementUPtr<std::tuple<std::optional<int>, std::optional<int>>(std::size_t)>(common::Config&), MultiElementB>(
        "MB"s, factory);
    registerElemetAndTbbHelper<IElementUPtr<void(int)>(common::Config&), MultiElementResult>("MR"s, factory);
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

  std::string cfg_str = R"({
  "name" : "TestName",
  "element" : "TestElement"
})";

  common::Config cfg = common::Config::make(std::move(cfg_str)).value();

  IExecutionGraphPtr graph = factory->create<IExecutionGraphUPtr>(TbbDefaultName::graph).value_or(nullptr);
  ASSERT_NE(nullptr, graph);

  auto a_node = factory->create<IExecutionNodeUPtr>("A"s, TbbDefaultName::source, cfg, graph).value_or(nullptr);
  auto b_node = factory->create<IExecutionNodeUPtr>("B"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);
  auto c_node = factory->create<IExecutionNodeUPtr>("C"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);
  auto d_node = factory->create<IExecutionNodeUPtr>("D"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);
  auto e_node = factory->create<IExecutionNodeUPtr>("E"s, TbbDefaultName::function, cfg, graph).value_or(nullptr);

  auto bc_node = factory->create<IExecutionNodeUPtr>("A"s, TbbDefaultName::broadcast_out, cfg, graph).value_or(nullptr);
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

TEST_F(GraphTest, multifunctional) {
  std::string cfg_str = R"({
  "name" : "TestName",
  "element" : "TestElement"
})";

  common::Config cfg = common::Config::make(std::move(cfg_str)).value();

  IExecutionGraphPtr graph = factory->create<IExecutionGraphUPtr>(TbbDefaultName::graph).value_or(nullptr);
  ASSERT_NE(nullptr, graph);

  auto ma_node = factory->create<IExecutionNodeUPtr>("MA"s, TbbDefaultName::multifunction, cfg, graph).value();
  auto mb_node = factory->create<IExecutionNodeUPtr>("MB"s, TbbDefaultName::multifunction, cfg, graph).value();

  auto mr0_node = factory->create<IExecutionNodeUPtr>("MR"s, TbbDefaultName::function, cfg, graph).value();
  auto mr1_node = factory->create<IExecutionNodeUPtr>("MR"s, TbbDefaultName::function, cfg, graph).value();
  auto mr2_node = factory->create<IExecutionNodeUPtr>("MR"s, TbbDefaultName::function, cfg, graph).value();

  ASSERT_TRUE(mr0_node->connect(ma_node->sender(0), 0));
  ASSERT_TRUE(mr1_node->connect(mb_node->sender(0), 0));
  ASSERT_TRUE(mr2_node->connect(mb_node->sender(1), 0));

  auto ma_in_node = std::dynamic_pointer_cast<IInputExecutionNode<NodeType::Functional, std::size_t, std::size_t>>(
      IExecutionNodePtr(std::move(ma_node)));
  auto mb_in_node = std::dynamic_pointer_cast<IInputExecutionNode<NodeType::Functional, std::size_t>>(
      IExecutionNodePtr(std::move(mb_node)));

  ASSERT_NE(nullptr, ma_in_node);
  ASSERT_NE(nullptr, mb_in_node);

  ASSERT_TRUE(ma_in_node->tryPut(0, 0));
  ASSERT_TRUE(mb_in_node->tryPut(1));
  ASSERT_TRUE(ma_in_node->tryPut(2, 2));
  ASSERT_TRUE(mb_in_node->tryPut(3));

  graph->waitForAll();
}

}  // namespace
