cmake_minimum_required(VERSION 3.20)

project(
  QgisPluginR
  VERSION 0.1
  DESCRIPTION "Cpp plugin that allows running R from within QGIS"
  LANGUAGES CXX)

option(DOCKER_RELEASE "Prepare release in docker." OFF)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # include(CTest)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QGIS REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets Xml Gui REQUIRED)

find_package(R REQUIRED)
find_package(RCpp REQUIRED)

add_compile_definitions(RINSIDE_CALLBACKS=1)
exec_program(Rscript
  ARGS ${CMAKE_SOURCE_DIR}/external/r_inside/src/tools/RInsideAutoloads.r
  OUTPUT_VARIABLE R_INSIDE_AUTOLOADS
)
file(WRITE
  ${CMAKE_SOURCE_DIR}/external/r_inside/inst/include/RInsideAutoloads.h "${R_INSIDE_AUTOLOADS}"
)

exec_program(Rscript
  ARGS ${CMAKE_SOURCE_DIR}/external/r_inside/src/tools/RInsideEnvVars.r
  OUTPUT_VARIABLE R_INSIDE_ENV_VARS)
file(WRITE
  ${CMAKE_SOURCE_DIR}/external/r_inside/inst/include/RInsideEnvVars.h "${R_INSIDE_ENV_VARS}"
)

set(R_INSIDE
  ${CMAKE_SOURCE_DIR}/external/r_inside/src/MemBuf.cpp
  ${CMAKE_SOURCE_DIR}/external/r_inside/src/RInside.cpp
  ${CMAKE_SOURCE_DIR}/external/r_inside/src/RInside_C.cpp
  ${CMAKE_SOURCE_DIR}/external/r_inside/src/RcppExports.cpp
  ${CMAKE_SOURCE_DIR}/external/r_inside/src/compiler.cpp
)

set(R_INSIDE_INCLUDE
  ${CMAKE_SOURCE_DIR}/external/r_inside/inst/include
  ${CMAKE_SOURCE_DIR}/external/cpp11/inst/include
)

set(LIBS
  ${QGIS_CORE_LIBRARY}
  ${QGIS_GUI_LIBRARY}
  ${QGIS_ANALYSIS_LIBRARY}
  ${R_LIB}
  Rcpp
  ${RINSIDE_LIBRARY}
  Qt5::Core
  Qt5::Gui
  Qt5::Xml
  Qt5::Widgets
)

set(INCLUDES
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QGIS_INCLUDE_DIR}
  ${R_INCLUDE_DIR}
  ${RCpp_INCLUDE_DIR}
  ${R_INSIDE_INCLUDE}
)

include_directories(
  ${INCLUDES}
)

# The compiled library code is here
add_subdirectory(src)

# if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND BUILD_TESTING)
# add_subdirectory(tests)
# endif()
