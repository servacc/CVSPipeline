cmake_minimum_required(VERSION 3.16)

project(CVSPipelineMain)

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(CVSLogger REQUIRED)
find_package(CVSLoggerOpenCV REQUIRED)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp
    )
target_link_libraries(${PROJECT_NAME} CVSPipeline Boost::program_options cvs::LoggerOpenCV)
