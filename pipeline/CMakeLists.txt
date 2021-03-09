cmake_minimum_required(VERSION 3.19)

project(cvspipeline)

find_package(Boost REQUIRED)
find_package(TBB REQUIRED)

add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME}
    PRIVATE
        include/cvs/pipeline/ipipeline.hpp
        include/cvs/pipeline/imodulemanager.hpp
        include/cvs/pipeline/iexecutionnode.hpp
        include/cvs/pipeline/iexecutiongraph.hpp
        include/cvs/pipeline/ielement.hpp
        include/cvs/pipeline/registrationhelper.hpp

        include/cvs/pipeline/tbb/tbbfunctionnode.hpp
        include/cvs/pipeline/tbb/tbbflowgraph.hpp
        include/cvs/pipeline/tbb/tbbbroadcastnode.hpp
        include/cvs/pipeline/tbb/tbbjoinnode.hpp
        include/cvs/pipeline/tbb/tbbcontinuenode.hpp
        include/cvs/pipeline/tbb/tbbsplitnode.hpp
        include/cvs/pipeline/tbb/tbbsourcenode.hpp
        include/cvs/pipeline/tbb/tbbhelpers.hpp
        include/cvs/pipeline/tbb/tbbdefinitions.hpp

        src/pipeline/tbb/tbbdefinitions.cpp
        src/pipeline/tbb/tbbhelpers.cpp
        src/pipeline/tbb/tbbflowgraph.cpp

        src/pipeline/pipeline.hpp
        src/pipeline/modulemanager.hpp

        src/pipeline/modulemanager.cpp
        src/pipeline/pipeline.cpp
        src/pipeline/imodule.cpp
    )
target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
    )
target_link_libraries(${PROJECT_NAME} cvscommon Boost::boost dl TBB::tbb)
