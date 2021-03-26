cmake_minimum_required(VERSION 3.19)

project(CVSPipeline_main)

find_package(Boost COMPONENTS program_options REQUIRED)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp
    )
target_link_libraries(${PROJECT_NAME} CVSPipeline Boost::program_options)
