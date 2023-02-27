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

# #######################################################
# Files
set(r_plugin_SRCS
  rplugin.cpp
  maplayerwrapper.cpp
  rstatsession.cpp
  rstatsrunner.cpp
  scopedprogresstask.cpp
  qgsrstatsconsole.cpp
)

set(r_plugin_RCCS
  pluginres.qrc
)

SET(PUBLIC_HEADERS_FILES
  rplugin.h
  maplayerwrapper.h
  qgisapplicationrwrapper.h
  rstatsrunner.h
  rstatsession.h
  scopedprogresstask.h
  qgsrstatsconsole.h
)

# QT5_WRAP_CPP(PUBLIC_MOC_SRCS ${PUBLIC_MOC_HEADERS_FILES})
# QT5_ADD_RESOURCES(r_plugin_RCCS_SRCS ${r_plugin_RCCS})

# #######################################################
# Build
add_library(r_plugin MODULE
  ${r_plugin_SRCS}
  ${r_plugin_RCCS}
  ${PUBLIC_HEADERS_FILES}
  ${R_INSIDE}
)

# require c++17
target_compile_features(r_plugin PRIVATE cxx_std_17)

target_link_libraries(r_plugin
  ${LIBS}
)

target_include_directories(r_plugin PUBLIC
  ${QGIS_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${R_INCLUDE_DIR}
  ${RCpp_INCLUDE_DIR}
  ${R_INSIDE_INCLUDE}
  ${CMAKE_SOURCE_DIR}/inlude/
)

# target_link_libraries(r_plugin_tests ${LIBS})
include_directories(
  ${QGIS_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${R_INCLUDE_DIR}
  ${RCpp_INCLUDE_DIR}
  ${R_INSIDE_INCLUDE}
  ${CMAKE_SOURCE_DIR}/inlude/
)

ADD_CUSTOM_COMMAND(
  TARGET r_plugin POST_BUILD
  COMMAND cp ${CMAKE_BINARY_DIR}/src/libr_plugin.so ${CMAKE_SOURCE_DIR}/plugin/libr_plugin.so
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Copy final library to folder"
)