cmake_minimum_required(VERSION 3.17)

project(CVSPipelineTool)

add_executable(CVSPipelineTool)
target_sources(${PROJECT_NAME}
    PRIVATE
        src/cvspipelinetool.cpp
    )
target_link_libraries(${PROJECT_NAME} cvs::PipelineMain)
