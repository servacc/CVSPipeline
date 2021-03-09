cmake_minimum_required(VERSION 3.16)

project(cvspipeline_main)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp
    )
target_link_libraries(${PROJECT_NAME} cvspipeline)
