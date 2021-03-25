project(tbbtest)

find_package(PkgConfig REQUIRED)
pkg_search_module(gmock_main REQUIRED IMPORTED_TARGET gmock_main)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/graphtest.cpp
    )

target_link_libraries(${PROJECT_NAME} PkgConfig::gmock_main cvspipeline)

add_test(TbbPipelineTest ${PROJECT_NAME})
