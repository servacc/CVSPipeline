project(tbbtest)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
    PRIVATE
        src/graphtest.cpp
    )

target_link_libraries(${PROJECT_NAME} gtest_main cvspipeline)
