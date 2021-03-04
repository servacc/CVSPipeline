//#include <boost/core/demangle.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/pipeline/tbb/tbbhelpers.hpp>
#include <gtest/gtest.h>

using namespace std::literals::string_literals;
using namespace cvs;
using namespace cvs::pipeline;
using namespace cvs::pipeline::tbb;

namespace {

class StartElement : public IElement<int(bool*)> {
 public:
  static auto make(common::Configuration) { return std::make_unique<StartElement>(); }

  int process(bool* stop) override {
    if (counter == 10) {
      *stop = true;
      return counter;
    }
    return counter++;
  }

 public:
  static int counter;
};
int StartElement::counter = 0;

class AElement : public IElement<int(int)> {
 public:
  static auto make(common::Configuration cfg) {
    int n = 5;
    return std::make_unique<AElement>(n);
  }

  AElement(int m)
      : mul(m) {}

  int process(int a) override { return a * mul; }

 private:
  int mul;
};

class BElement : public IElement<int(int, int)> {
 public:
  static auto make(common::Configuration cfg) { return std::make_unique<BElement>(); }

  int process(int a, int b) override { return a + b; }
};

class CElement : public IElement<void(int)> {
 public:
  static auto make(common::Configuration) { return std::make_unique<CElement>(); }

  void process(int a) override { result.push_back(a); }

 public:
  static std::vector<int> result;
};
std::vector<int> CElement::result;

}  // namespace

namespace {
class GraphTest : public ::testing::Test {
 public:
  static void SetUpTestSuite() {
    registrateBase();

    registrateElemetHelper<IElementUPtr<int(bool*)>(common::Configuration), StartElement>("E_S"s);
    registrateElemetHelper<IElementUPtr<int(int)>(common::Configuration), AElement>("E_A"s);
    registrateElemetHelper<IElementUPtr<int(int, int)>(common::Configuration), BElement>("E_B"s);
    registrateElemetHelper<IElementUPtr<void(int)>(common::Configuration), CElement>("E_C"s);
  }
};

TEST_F(GraphTest, create) {
  using namespace cvs::pipeline;
  common::Configuration cfg;

  IExecutionGraphPtr graph = common::Factory::create<IExecutionGraphUPtr>("TbbGraph"s);
  ASSERT_NE(nullptr, graph);

  auto start_node = common::Factory::create<IExecutionNodeUPtr>("E_S"s, TbbDefaultName::source_name, cfg, graph);
  ASSERT_NE(nullptr, start_node);
  auto c_node = common::Factory::create<IExecutionNodeUPtr>("E_C"s, TbbDefaultName::function_name, cfg, graph);
  ASSERT_NE(nullptr, c_node);

  auto sender = start_node->sender(0);
  ASSERT_TRUE(sender.has_value());
  ASSERT_TRUE(c_node->connect(sender, 0));

  auto src_node = std::dynamic_pointer_cast<ISourceExecutionNode>(IExecutionNodePtr(std::move(start_node)));
  src_node->activate();

  graph->waitForAll();

  EXPECT_EQ(StartElement::counter, CElement::result.size());
}

}  // namespace
