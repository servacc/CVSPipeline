#include <cvs/common/config.hpp>
#include <cvs/common/configbase.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/pipeline/imodulemanager.hpp>
#include <cvs/pipeline/ipipeline.hpp>
#include <cvs/pipeline/registrationhelper.hpp>
#include <cvs/pipeline/tbb/tbbhelpers.hpp>
#include <gtest/gtest.h>

TEST(Pipeline, createGraph) {
  std::string config_json = R"json(
{
  "ModuleManager" : {
    "module_path": [
        ")json" TEST_MODULES_PATH R"json("
      ]
  },
  "loggers" : [
    { "name": "", "level": "0", "sink": "1" },
    { "name": "cvs.pipeline.Pipeline", "level": "1", "sink": "1" },
    { "name": "cvs.pipeline.ModuleManager", "level": "1", "sink": "1" },
    { "name": "cvs.pipeline.NodeFactory", "level": "0", "sink": "1" },
    { "name": "cvs.pipeline.tbb.helper", "level": "1", "sink": "1" }
  ],
  "Pipeline" : {
    "graph" : {
      "type": "TbbGraph"
    },
    "nodes" : [
      { "name": "a",  "element" : "A", "node" : "TbbSourceDefault"    },
      { "name": "b",  "element" : "B", "node" : "TbbFunctionDefault"  },
      { "name": "c",  "element" : "C", "node" : "TbbFunctionDefault"  },
      { "name": "d",  "element" : "D", "node" : "TbbFunctionDefault"  },
      { "name": "e",  "element" : "E", "node" : "TbbFunctionDefault"  },
      { "name": "j",  "element" : "D", "node" : "TbbJoinDefault"      },
      { "name": "bc", "element" : "A", "node" : "TbbBroadcastDefault" }
    ],
    "connections" : [
      { "from" : "a" , "output" : "0", "to" : "bc", "input" : "0" },
      { "from" : "bc", "output" : "0", "to" : "b" , "input" : "0" },
      { "from" : "bc", "output" : "0", "to" : "c" , "input" : "0" },
      { "from" : "b" , "output" : "0", "to" : "j" , "input" : "0" },
      { "from" : "c" , "output" : "0", "to" : "j" , "input" : "1" },
      { "from" : "j" , "output" : "0", "to" : "d" , "input" : "0" },
      { "from" : "d" , "output" : "0", "to" : "e" , "input" : "0" }
    ]
  }
}
)json";

  auto cfg_root = cvs::common::Config::make(std::move(config_json)).value();
  cvs::logger::initLoggers(cfg_root);

  auto factory = cvs::common::Factory<std::string>::defaultInstance();
  cvs::pipeline::registerDefault(factory);
  cvs::pipeline::tbb::registerBase(factory);

  auto module_manager = factory->create<cvs::pipeline::IModuleManagerUPtr>("Default", cfg_root).value();
  ASSERT_NE(nullptr, module_manager);

  module_manager->loadModules();
  module_manager->registerTypes(factory);

  auto pipeline =
      factory
          ->create<cvs::pipeline::IPipelineUPtr, cvs::common::Config&, const cvs::common::FactoryPtr<std::string>&>(
              "Default", cfg_root, factory)
          .value();
  ASSERT_NE(nullptr, pipeline);

  auto a_node = pipeline->getNode("a");
  auto a_src_node =
      std::dynamic_pointer_cast<cvs::pipeline::ISourceExecutionNode<cvs::pipeline::NodeType::Functional>>(a_node);
  ASSERT_NE(nullptr, a_src_node);

  a_src_node->activate();

  pipeline->waitForAll();
}
