cmake_minimum_required(VERSION 3.16)

project(elementtest)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(PkgConfig REQUIRED)
pkg_search_module(gmock_main REQUIRED IMPORTED_TARGET gmock_main)

add_executable(${PROJECT_NAME} cvselement.cpp)


target_link_libraries(${PROJECT_NAME} cvs::Pipeline Boost::boost Boost::filesystem dl PkgConfig::gmock_main)

add_test(ElementTest ${PROJECT_NAME})
