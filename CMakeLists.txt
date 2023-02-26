cmake_minimum_required(VERSION 3.20)

project(
  QgisPluginR
  VERSION 0.1
  DESCRIPTION "Cpp plugin that allows running R from within QGIS"
  LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # include(CTest)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

find_package(QGIS REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets Xml Gui REQUIRED)

message(STATUS ${QGIS_INCLUDE_DIR})

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
)

# The compiled library code is here
add_subdirectory(src)

# The executable code is here
# add_subdirectory(apps)
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND BUILD_TESTING)
  add_subdirectory(tests)
endif()
