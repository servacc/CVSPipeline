#include "../../pipeline/include/cvs/pipeline/impl/moduleManager.hpp"

#include <cvs/common/config.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/iexecutionGraph.hpp>
#include <cvs/pipeline/iexecutionNode.hpp>
#include <cvs/pipeline/tbb/tbbDefinitions.hpp>
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
    { "name": "", "level": "trace", "sink": "std" },
    { "name": "cvs.pipeline.ModuleManager", "level": "debug", "sink": "std" },
    { "name": "cvs.pipeline.NodeFactory", "level": "trace", "sink": "std" },
    { "name": "cvs.pipeline.tbb.helper", "level": "debug", "sink": "std" },
    { "name": "cvs.pipeline.helper", "level": "trace", "sink": "std" }
  ]
}
)json";

  auto factory = std::make_shared<cvs::common::Factory<std::string>>();

  auto cfg_root_opt = cvs::common::CVSConfigBase::load(config_str);

  std::string node_cfg_str = R"({
  "name" : "TestName",
  "element" : "TestElement"
})";

  const auto node_cfg = common::CVSConfigBase::load(node_cfg_str);

  cvs::logger::initLoggers(cfg_root_opt);

  auto manager = cvs::pipeline::impl::ModuleManager::make(cfg_root_opt);
  manager->loadModules();
  manager->registerTypes(factory);

  IExecutionGraphPtr graph = factory->create<IExecutionGraphUPtr>(TbbDefaultName::graph).value();

  auto a_node = factory->create<IExecutionNodeUPtr>("A"s, TbbDefaultName::source, node_cfg, graph).value();
  auto b_node = factory->create<IExecutionNodeUPtr>("B"s, TbbDefaultName::function, node_cfg, graph).value();
  auto c_node = factory->create<IExecutionNodeUPtr>("C"s, TbbDefaultName::function, node_cfg, graph).value();
  auto d_node = factory->create<IExecutionNodeUPtr>("D"s, TbbDefaultName::function, node_cfg, graph).value();
  auto e_node = factory->create<IExecutionNodeUPtr>("E"s, TbbDefaultName::function, node_cfg, graph).value();

  auto bc_node = factory->create<IExecutionNodeUPtr>("A"s, TbbDefaultName::broadcast_out, node_cfg, graph).value();
  auto j_node  = factory->create<IExecutionNodeUPtr>("D"s, TbbDefaultName::join, node_cfg, graph).value();

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
