cmake_minimum_required(VERSION 3.17)

project(CVSPipelineInfo)

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(CVSLogger REQUIRED)
find_package(CVSLoggerOpenCV REQUIRED)

add_executable(CVSPipelineInfo)
target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.cpp
    )

target_link_libraries(${PROJECT_NAME} cvs::Pipeline Boost::program_options)