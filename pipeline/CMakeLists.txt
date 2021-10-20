cmake_minimum_required(VERSION 3.16)

project(Pipeline)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(TBB REQUIRED)
find_package(CVSLogger REQUIRED)

add_library(${PROJECT_NAME} SHARED)

cvs_create_version_file(NAME ${PROJECT_NAME}
                        MAJOR ${CVSPipelineLib_VERSION_MAJOR}
                        MINOR ${CVSPipelineLib_VERSION_MINOR}
                        PATCH ${CVSPipelineLib_VERSION_PATCH}
                        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generate/cvs/pipeline")
cvs_add_to_alias(${PROJECT_NAME})
cvs_library_version(TARGET ${PROJECT_NAME}
                    VERSION_MAJOR ${CVSPipelineLib_VERSION_MAJOR}
                    VERSION_MINOR ${CVSPipelineLib_VERSION_MINOR})

target_sources(${PROJECT_NAME}
    PRIVATE
        include/cvs/pipeline/ipipeline.hpp
        include/cvs/pipeline/imoduleManager.hpp
        include/cvs/pipeline/iexecutionNode.hpp
        include/cvs/pipeline/iexecutionGraph.hpp
        include/cvs/pipeline/ielement.hpp
        include/cvs/pipeline/imodule.hpp
        include/cvs/pipeline/iview.hpp
        include/cvs/pipeline/registrationHelper.hpp

        include/cvs/pipeline/impl/pipeline.hpp
        include/cvs/pipeline/impl/guiPipeline.hpp
        include/cvs/pipeline/impl/moduleManager.hpp

        include/cvs/pipeline/tbb/tbbBufferTemplateNode.hpp
        include/cvs/pipeline/tbb/tbbBroadcastNode.hpp
        include/cvs/pipeline/tbb/tbbOverwriteNode.hpp
        include/cvs/pipeline/tbb/tbbBufferNode.hpp

        include/cvs/pipeline/tbb/tbbMultifunctionNode.hpp
        include/cvs/pipeline/tbb/tbbFunctionNode.hpp
        include/cvs/pipeline/tbb/tbbFlowGraph.hpp
        include/cvs/pipeline/tbb/tbbJoinNode.hpp
        include/cvs/pipeline/tbb/tbbContinueNode.hpp
        include/cvs/pipeline/tbb/tbbSplitNode.hpp
        include/cvs/pipeline/tbb/tbbSourceNode.hpp
        include/cvs/pipeline/tbb/tbbHelpers.hpp
        include/cvs/pipeline/tbb/tbbDefinitions.hpp
        include/cvs/pipeline/tbb/tbbView.hpp

        src/pipeline/tbb/tbbDefinitions.cpp
        src/pipeline/tbb/tbbHelpers.cpp
        src/pipeline/tbb/tbbFlowGraph.cpp

        src/pipeline/registrationHelper.cpp
        src/pipeline/moduleManager.cpp
        src/pipeline/pipeline.cpp
        src/pipeline/guiPipeline.cpp
        src/pipeline/imodule.cpp
    )
target_include_directories(${PROJECT_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generate>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CVS${PROJECT_NAME}>)

target_link_libraries(${PROJECT_NAME}
  Boost::boost Boost::filesystem
  dl
  TBB::tbb
  cvs::Logger)
