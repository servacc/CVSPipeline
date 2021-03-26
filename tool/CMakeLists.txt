cmake_minimum_required(VERSION 3.19)

project(CVSPipelinetool)

add_executable(CVSPipelinetool)
target_sources(${PROJECT_NAME}
    PRIVATE
        src/cvspipelinetool.cpp
    )
target_link_libraries(${PROJECT_NAME} CVSPipeline_main)
