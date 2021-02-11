cmake_minimum_required(VERSION 3.19)

project(cvspipeline)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        include/cvs/pipeline/ipipeline.hpp
        include/cvs/pipeline/imodulemanager.hpp

        src/pipeline/pipeline.hpp
        src/pipeline/modulemanager.hpp

        src/pipeline/modulemanager.cpp
        src/pipeline/pipeline.cpp
    )
target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
    )
target_link_libraries(${PROJECT_NAME} cvscommon)
