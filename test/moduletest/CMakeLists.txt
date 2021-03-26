cmake_minimum_required(VERSION 3.16)

project(moduletest)

add_subdirectory(dummymodules)

find_package(Boost REQUIRED COMPONENTS filesystem)

add_executable(${PROJECT_NAME} main.cpp)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    TEST_MODULES_PATH="${CMAKE_CURRENT_BINARY_DIR}/dummymodules")
target_link_libraries(${PROJECT_NAME} cvs::CVSPipeline Boost::boost Boost::filesystem dl)

add_test(ModuleTest ${PROJECT_NAME})
