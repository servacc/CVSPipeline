#include "../../pipeline/include/cvs/pipeline/impl/modulemanager.hpp"

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/iexecutiongraph.hpp>
#include <cvs/pipeline/iexecutionnode.hpp>
#include <cvs/pipeline/tbb/tbbdefinitions.hpp>
#include <gtest/gtest.h>

using namespace cvs;
using namespace cvs::common;
using namespace cvs::pipeline;
using namespace cvs::pipeline::tbb;

using namespace std::string_literals;

TEST(ModuleManagerTest, testPipeline) {
  std::string config_str = R"json(
{
  "ModuleManager" : {
    "module_path": [")json" TEST_MODULES_PATH R"json(",
      ")json" TEST_MODULES_PATH R"json("
    ]
  },
  "loggers" : [
    { "name": "", "level": "0", "sink": "1" },
    { "name": "cvs.pipeline.ModuleManager", "level": "1", "sink": "1" },
    { "name": "cvs.pipeline.NodeFactory", "level": "0", "sink": "1" },
    { "name": "cvs.pipeline.tbb.helper", "level": "1", "sink": "1" },
    { "name": "cvs.pipeline.helper", "level": "0", "sink": "1" }
  ]
}
)json";

  auto factory = std::make_shared<cvs::common::Factory<std::string>>();

  auto cfg_root_opt = cvs::common::Config::make(std::move(config_str));
  ASSERT_TRUE(cfg_root_opt.has_value());

  cvs::logger::initLoggers(cfg_root_opt);

  auto manager = cvs::pipeline::impl::ModuleManager::make(*cfg_root_opt);
  manager->loadModules();
  manager->registerTypes(factory);

  IExecutionGraphPtr graph = factory->create<IExecutionGraphUPtr>(TbbDefaultName::graph).value_or(nullptr);
  ASSERT_NE(nullptr, graph);

  auto a_node =
      factory->create<IExecutionNodeUPtr>("A"s, TbbDefaultName::source, *cfg_root_opt, graph).value_or(nullptr);
  auto b_node =
      factory->create<IExecutionNodeUPtr>("B"s, TbbDefaultName::function, *cfg_root_opt, graph).value_or(nullptr);
  auto c_node =
      factory->create<IExecutionNodeUPtr>("C"s, TbbDefaultName::function, *cfg_root_opt, graph).value_or(nullptr);
  auto d_node =
      factory->create<IExecutionNodeUPtr>("D"s, TbbDefaultName::function, *cfg_root_opt, graph).value_or(nullptr);
  auto e_node =
      factory->create<IExecutionNodeUPtr>("E"s, TbbDefaultName::function, *cfg_root_opt, graph).value_or(nullptr);

  auto bc_node =
      factory->create<IExecutionNodeUPtr>("A"s, TbbDefaultName::broadcast_out, *cfg_root_opt, graph).value_or(nullptr);
  auto j_node = factory->create<IExecutionNodeUPtr>("D"s, TbbDefaultName::join, *cfg_root_opt, graph).value_or(nullptr);

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
