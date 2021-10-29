cmake_minimum_required(VERSION 3.16)

find_package(CVSCommonCMakeHelpers REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${CVSCOMMON_MODULE_PATH}")
include(CVSCommon)

project(CVSPipelineLib CVS_PROJECT
                       VERSION 0.1.0
                       DESCRIPTION "CVS Pipeline.")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
set(CVS_LIBS_DIR    "${CMAKE_INSTALL_LIBDIR}/cvs")
set(CVS_MODULE_DIR  "${CVS_LIBS_DIR}/cvsmodules")
set(CVS_RUNTIME_DIR "${CMAKE_INSTALL_BINDIR}")
set(CVS_CONFIG_DIR  "${CMAKE_INSTALL_SYSCONFDIR}")
set(CVS_DATA_DIR    "${CMAKE_INSTALL_DATADIR}")

add_compile_definitions(
    PROJECT_VERSION="${PROJECT_VERSION}"
    PROJECT_DESCRIPTION="${PROJECT_DESCRIPTION}"
    DEFAULT_CONFIG="${CMAKE_INSTALL_PREFIX}/${CVS_CONFIG_DIR}/cvspipeline.json"
    CVSPipeline_MODULE_DIR="${CMAKE_INSTALL_PREFIX}/${CVS_MODULE_DIR}"
    CVSPipeline_VER_MAJOR=${PROJECT_VERSION_MAJOR}
    CVSPipeline_VER_MINOR=${PROJECT_VERSION_MINOR}
    CVSPipeline_VER_PATCH=${PROJECT_VERSION_PATCH})


set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_INSTALL_PREFIX}/${CVS_LIBS_DIR};${CMAKE_INSTALL_PREFIX}/${CVS_MODULE_DIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


add_subdirectory(pipeline)
add_subdirectory(libmain)
add_subdirectory(tool)

include(CTest)
add_subdirectory(test)

include(CVSInstall)

cvs_install(NAME CVSPipeline
  TARGETS_BIN CVSPipelineTool Pipeline
  TARGETS_DEV PipelineMain
  VERSION ${CVSPipelineLib_VERSION_MAJOR}.${CVSPipelineLib_VERSION_MINOR}.${CVSPipelineLib_VERSION_PATCH}
  FILES_DEV
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/CVSPipeline/cvs/pipeline
    FILES pipeline/include/cvs/pipeline/ipipeline.hpp
          pipeline/include/cvs/pipeline/imoduleManager.hpp
          pipeline/include/cvs/pipeline/iexecutionNode.hpp
          pipeline/include/cvs/pipeline/iexecutionGraph.hpp
          pipeline/include/cvs/pipeline/ielement.hpp
          pipeline/include/cvs/pipeline/iview.hpp
          pipeline/include/cvs/pipeline/imodule.hpp
          pipeline/include/cvs/pipeline/registrationHelper.hpp
          pipeline/include/cvs/pipeline/dataCounter.hpp
          ${CMAKE_CURRENT_BINARY_DIR}/pipeline/generate/cvs/pipeline/version.hpp

    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/CVSPipeline/cvs/pipeline/impl
    FILES pipeline/include/cvs/pipeline/impl/pipeline.hpp
          pipeline/include/cvs/pipeline/impl/moduleManager.hpp

    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/CVSPipeline/cvs/pipeline/tbb
    FILES pipeline/include/cvs/pipeline/tbb/tbbFunctionNode.hpp
          pipeline/include/cvs/pipeline/tbb/tbbFlowGraph.hpp
          pipeline/include/cvs/pipeline/tbb/tbbJoinNode.hpp
          pipeline/include/cvs/pipeline/tbb/tbbContinueNode.hpp
          pipeline/include/cvs/pipeline/tbb/tbbSplitNode.hpp
          pipeline/include/cvs/pipeline/tbb/tbbSourceNode.hpp
          pipeline/include/cvs/pipeline/tbb/tbbHelpers.hpp
          pipeline/include/cvs/pipeline/tbb/tbbDefinitions.hpp
          pipeline/include/cvs/pipeline/tbb/tbbView.hpp
          pipeline/include/cvs/pipeline/tbb/tbbBroadcastNode.hpp
          pipeline/include/cvs/pipeline/tbb/tbbOverwriteNode.hpp
          pipeline/include/cvs/pipeline/tbb/tbbBufferNode.hpp
          pipeline/include/cvs/pipeline/tbb/tbbBufferTemplateNode.hpp
          pipeline/include/cvs/pipeline/tbb/tbbMultifunctionNode.hpp
          pipeline/include/cvs/pipeline/tbb/tbbQueueNode.hpp)
