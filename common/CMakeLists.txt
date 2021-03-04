cmake_minimum_required(VERSION 3.19)

project(cvscommon)

add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME}
    PRIVATE
        include/cvs/common/factory.hpp
        include/cvs/common/configuration.hpp

        src/configuration.cpp
        src/factory.cpp
    )
target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
    )
