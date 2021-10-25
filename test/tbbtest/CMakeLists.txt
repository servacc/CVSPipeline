project(tbbtest)

find_package(PkgConfig REQUIRED)
pkg_search_module(gmock_main REQUIRED IMPORTED_TARGET gmock_main)

add_executable(${PROJECT_NAME} src/graphtest.cpp)
target_link_libraries(${PROJECT_NAME} PkgConfig::gmock_main cvs::Pipeline)
add_test(TbbPipelineTest ${PROJECT_NAME})

add_executable(guiTest src/guiTest.cpp)
target_link_libraries(guiTest PkgConfig::gmock_main cvs::Pipeline)
add_test(TbbGuiTest guiTest)
