cmake_minimum_required(VERSION 3.18)

project(cvspipeline)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
        PRIVATE
        include/cvs/pipeline/ipipeline.hpp
        include/cvs/pipeline/imodulemanager.hpp
        include/cvs/config/config.h
        include/cvs/config/config_base.hpp
        include/cvs/config/config_utils.hpp

        src/pipeline/pipeline.hpp
        src/pipeline/modulemanager.hpp

        src/pipeline/modulemanager.cpp
        src/pipeline/pipeline.cpp
        src/config/config.cpp
        )
target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
    )
target_link_libraries(${PROJECT_NAME} cvscommon)
