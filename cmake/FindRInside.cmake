# Cmake module to find RInisde
# - Try to find Rinside
# Once done, this will define
#
# RINSIDE_FOUND - system has RINSIDE
# RINSIDE_INCLUDE_DIRS - the RINSIDE include directories
# RINSIDE_LIBRARIES - link these to use RINSIDE
# Autor: Omar Andres Zapata Mesa 31/05/2013

message(STATUS "Looking for RInside")
find_program(R_EXECUTABLE
    NAMES R R.exe
)

if(R_EXECUTABLE)
    execute_process(COMMAND Rscript -e "RInside:::CxxFlags()" OUTPUT_VARIABLE RInside_INCLUDE_DIR_TMP)

    string(REGEX REPLACE "^-I" "" RInside_INCLUDE_DIR "${RInside_INCLUDE_DIR_TMP}")

    message(STATUS "***" ${RInside_INCLUDE_DIR})

    find_library(RInside_LIB
        libRInside.so
        PATHS
        "${RInside_INCLUDE_DIR}/../lib"
    )
endif(R_EXECUTABLE)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this  lib depends on.
set(RINSIDE_INCLUDE_DIRS ${RINSIDE_INCLUDE_DIR})
set(RINSIDE_LIBRARIES ${RINSIDE_LIBRARY})

if(("${RInside_INCLUDE_DIR}" STREQUAL "") OR("${RInside_INCLUDE_DIR}" STREQUAL ""))
    set(RINSIDE_FOUND FALSE)
    message(STATUS "Looking for RInside -- not found")
    message(STATUS "Install it running 'R -e \"install.packages(\\\"RInside\\\")\"'")
else()
    set(RINSIDE_FOUND TRUE)
    message(STATUS "Looking for RInside -- found")
    message(STATUS ${RInside_LIB})
    message(STATUS ${RInside_INCLUDE_DIR})

    add_library(RInside UNKNOWN IMPORTED)
    set_property(TARGET RInside PROPERTY IMPORTED_LOCATION "${RInside_LIB}")

    mark_as_advanced(RInside_INCLUDE_DIR)
    mark_as_advanced(RInside_LIB)
endif()