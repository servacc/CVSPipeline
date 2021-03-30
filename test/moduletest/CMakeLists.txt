cmake_minimum_required(VERSION 3.16)

project(moduletest)

add_subdirectory(dummymodules)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(PkgConfig REQUIRED)
pkg_search_module(gmock_main REQUIRED IMPORTED_TARGET gmock_main)

add_executable(${PROJECT_NAME} main.cpp)
add_executable(moduleManagerTest modulemanagertest.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE TEST_MODULES_PATH="${CMAKE_CURRENT_BINARY_DIR}/dummymodules")
target_compile_definitions(moduleManagerTest PRIVATE TEST_MODULES_PATH="${CMAKE_CURRENT_BINARY_DIR}/dummymodules")

target_link_libraries(${PROJECT_NAME} cvs::CVSPipeline Boost::boost Boost::filesystem dl)
target_link_libraries(moduleManagerTest cvs::CVSPipeline Boost::boost Boost::filesystem dl PkgConfig::gmock_main)

add_test(ModuleTest ${PROJECT_NAME})
add_test(ModuleManagerTest moduleManagerTest)
