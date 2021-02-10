cmake_minimum_required(VERSION 3.19)

include(cmake/Helpers.cmake)
git_description(GIT_DESCRIPTOR GIT_TAG GIT_COMMIT_COUNT)

if(NOT BUILD_NUMBER)
    set(BUILD_NUMBER "")
else()
    set(BUILD_NUMBER ".${BUILD_NUMBER}")
endif()

project(CVSPipeline
    VERSION 0.0.${GIT_COMMIT_COUNT}${BUILD_NUMBER}
    DESCRIPTION "Image stitching. ${GIT_DESCRIPTOR}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
#set(CVS_LIBS_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/cvs")
set(CVS_MODULE_DIR "${HSC_LIBS_DIR}/cvsmodules")
set(CVS_RUNTIME_DIR "${CMAKE_INSTALL_FULL_BINDIR}")
set(CVS_CONFIG_DIR "${CMAKE_INSTALL_FULL_SYSCONFDIR}")
set(CVS_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}")

add_library(${PROJECT_NAME} INTERFACE)

target_sources(${PROJECT_NAME}
    PRIVATE
        include/common/factory.hpp
    )


set(CVSPipeline_MAIN ${PROJECT_NAME}_main)
add_library(${CVSPipeline_MAIN})
target_sources(${CVSPipeline_MAIN}
    PRIVATE
        src/main.cpp
    )
target_link_libraries(${CVSPipeline_MAIN}
        ${PROJECT_NAME}
    )
