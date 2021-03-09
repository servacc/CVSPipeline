cmake_minimum_required(VERSION 3.18)

project(config_test)

enable_testing()
find_package(GTest REQUIRED)

add_executable(config_test config_test.cpp)
target_link_libraries(config_test cvspipeline GTest::gtest GTest::gtest_main)

add_test(AllTestsInFoo foo)