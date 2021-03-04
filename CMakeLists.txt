cmake_minimum_required(VERSION 3.19)

include(cmake/Helpers.cmake)
git_description(GIT_DESCRIPTOR GIT_TAG GIT_COMMIT_COUNT)

if(NOT BUILD_NUMBER)
    set(BUILD_NUMBER "")
else()
    set(BUILD_NUMBER ".${BUILD_NUMBER}")
endif()

project(CVSPipelineLib
    VERSION 0.0.${GIT_COMMIT_COUNT}${BUILD_NUMBER}
    DESCRIPTION "CVS Pipeline. ${GIT_DESCRIPTOR}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
#set(CVS_LIBS_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/cvs")
set(CVS_MODULE_DIR "${HSC_LIBS_DIR}/cvsmodules")
set(CVS_RUNTIME_DIR "${CMAKE_INSTALL_FULL_BINDIR}")
set(CVS_CONFIG_DIR "${CMAKE_INSTALL_FULL_SYSCONFDIR}")
set(CVS_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}")

add_compile_definitions(
    PROJECT_VERSION="${PROJECT_VERSION}"
    PROJECT_DESCRIPTION="${PROJECT_DESCRIPTION}"
)

add_subdirectory(common)
add_subdirectory(pipeline)
add_subdirectory(libmain)
add_subdirectory(tool)
add_subdirectory(test)
