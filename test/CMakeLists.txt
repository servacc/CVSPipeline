cmake_minimum_required(VERSION 3.19)

project(cvspipeline_test)

find_package(Boost REQUIRED COMPONENTS filesystem)

add_library(dummymodule SHARED)
target_sources(dummymodule
    PRIVATE
        module.cpp
    )
target_link_libraries(dummymodule cvspipeline)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} cvspipeline Boost::boost Boost::filesystem dl)
