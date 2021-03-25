cmake_minimum_required(VERSION 3.19)

find_package(CVSCommonCMakeHelpers REQUIRED)
list(APPEND CMAKE_MODULE_PATH "${CVSCOMMON_MODULE_PATH}")

include(CVSCommon)

project(CVSPipelineLib CVS_PROJECT
                       VERSION 0.1.0
                       DESCRIPTION "CVS Pipeline.")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
set(CVS_LIBS_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cvs")
set(CVS_MODULE_DIR "${CMAKE_INSTALL_PREFIX}/${HSC_LIBS_DIR}/cvsmodules")
set(CVS_RUNTIME_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")
set(CVS_CONFIG_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}")
set(CVS_DATA_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}")

add_compile_definitions(
    PROJECT_VERSION="${PROJECT_VERSION}"
    PROJECT_DESCRIPTION="${PROJECT_DESCRIPTION}"
    DEFAULT_CONFIG="${CVS_CONFIG_DIR}/cvspipeline.json")

add_subdirectory(pipeline)
add_subdirectory(libmain)
add_subdirectory(tool)

add_subdirectory(test)
