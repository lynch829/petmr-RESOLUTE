cmake_minimum_required( VERSION 3.7.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

# Set package information
set(PACKAGE_NAME "RESOLUTE")
set(PACKAGE_VERSION "1.0.0")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME "${PACKAGE_NAME}-${PACKAGE_VERSION}")
set(PACKAGE_LIBNAME "${PACKAGE_NAME}-${PACKAGE_VERSION}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(${PACKAGE_NAME} VERSION ${PACKAGE_VERSION})

option(BUILD_TESTING "" OFF)
include(CTest)

make_directory(${PROJECT_BINARY_DIR}/config)

set(BUILDER_NAME ${CURRENT_USER})

configure_file(
  "${PROJECT_SOURCE_DIR}/EnvironmentInfo.h.in"
  "${PROJECT_BINARY_DIR}/config/EnvironmentInfo.h"
)

include(resolute-deps)

include_directories("${PROJECT_BINARY_DIR}/config")
#include_directories(lib)
include_directories(src)
add_subdirectory(src)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()