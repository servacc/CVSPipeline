cmake_minimum_required(VERSION 3.17)

project(dummymodules)

find_package(PkgConfig REQUIRED)
pkg_search_module(gmock REQUIRED IMPORTED_TARGET gmock)

add_library(dummyA SHARED dummyA.cpp)
add_library(dummyB SHARED dummyB.cpp)
add_library(dummyC SHARED dummyC.cpp)

target_link_libraries(dummyA cvs::Pipeline PkgConfig::gmock)
target_link_libraries(dummyB cvs::Pipeline PkgConfig::gmock)
target_link_libraries(dummyC cvs::Pipeline PkgConfig::gmock)

add_library(dummymodule SHARED module.cpp)

target_link_libraries(dummymodule cvs::Pipeline)
