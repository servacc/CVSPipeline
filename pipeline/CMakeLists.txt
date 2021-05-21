cmake_minimum_required(VERSION 3.19)

project(CVSPipeline)

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
        include/cvs/pipeline/imodulemanager.hpp
        include/cvs/pipeline/iexecutionnode.hpp
        include/cvs/pipeline/iexecutiongraph.hpp
        include/cvs/pipeline/ielement.hpp
        include/cvs/pipeline/imodule.hpp
        include/cvs/pipeline/iview.hpp
        include/cvs/pipeline/registrationhelper.hpp

        include/cvs/pipeline/impl/pipeline.hpp
        include/cvs/pipeline/impl/guiPipeline.hpp
        include/cvs/pipeline/impl/modulemanager.hpp

        include/cvs/pipeline/tbb/tbbfunctionnode.hpp
        include/cvs/pipeline/tbb/tbbflowgraph.hpp
        include/cvs/pipeline/tbb/tbbbroadcastnode.hpp
        include/cvs/pipeline/tbb/tbbjoinnode.hpp
        include/cvs/pipeline/tbb/tbbcontinuenode.hpp
        include/cvs/pipeline/tbb/tbbsplitnode.hpp
        include/cvs/pipeline/tbb/tbbsourcenode.hpp
        include/cvs/pipeline/tbb/tbbhelpers.hpp
        include/cvs/pipeline/tbb/tbbdefinitions.hpp
        include/cvs/pipeline/tbb/tbbview.hpp

        src/pipeline/tbb/tbbdefinitions.cpp
        src/pipeline/tbb/tbbhelpers.cpp
        src/pipeline/tbb/tbbflowgraph.cpp

        src/pipeline/registrationhelper.cpp
        src/pipeline/modulemanager.cpp
        src/pipeline/pipeline.cpp
        src/pipeline/guiPipeline.cpp
        src/pipeline/imodule.cpp
    )
target_include_directories(${PROJECT_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generate>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>)

target_link_libraries(${PROJECT_NAME}
  Boost::boost Boost::filesystem
  dl
  TBB::tbb
  cvs::CVSLogger)
