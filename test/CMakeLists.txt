include(CTest)

if(NOT BUILD_TESTING)
    return()
endif()

enable_testing()

add_subdirectory(tbbtest)
add_subdirectory(moduletest)
