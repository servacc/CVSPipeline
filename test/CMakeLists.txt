option(CVSPIPELINE_TESTS "Enable tests" OFF)

if(NOT CVSPIPELINE_TESTS)
    return()
endif()

if(NOT TARGET gtest)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    add_subdirectory(../thirdparty/googletest googletest)
endif()

add_subdirectory(tbbtest)
