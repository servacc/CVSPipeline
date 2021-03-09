cmake_minimum_required(VERSION 3.16)

project(cvspipelinetool)

add_executable(cvspipelinetool)
target_sources(${PROJECT_NAME}
    PRIVATE
        src/cvspipelinetool.cpp
    )
target_link_libraries(${PROJECT_NAME} cvspipeline_main)
