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

  scopedprogresstask.cpp
  qgsdockablewidgethelper.cpp

  qgsrstatsapplicationwrapper.cpp
  qgsrstatsconsole.cpp
  qgsrstatsfunctions.cpp
  qgsrstatsfunctions.cpp
  qgsrstatsmaplayerwrapper.cpp
  qgsrstatsrunner.cpp
  qgsrstatssession.cpp
  qgsrstatssettings.cpp
  qgsrstatsutils.cpp
)

set(r_plugin_RCCS
  pluginres.qrc
)

SET(PUBLIC_HEADERS_FILES
  rplugin.h

  scopedprogresstask.h
  qgsdockablewidgethelper.h

  qgsrstatsapplicationwrapper.h
  qgsrstatsconsole.h
  qgsrstatsfunctions.h
  qgsrstatsmaplayerwrapper.h
  qgsrstatsrunner.h
  qgsrstatssession.h
  qgsrstatsutils.h
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
)

# target_link_libraries(r_plugin_tests ${LIBS})
include_directories(
  ${QGIS_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${R_INCLUDE_DIR}
  ${RCpp_INCLUDE_DIR}
  ${R_INSIDE_INCLUDE}
)

ADD_CUSTOM_COMMAND(
  TARGET r_plugin POST_BUILD
  COMMAND cp ${CMAKE_BINARY_DIR}/src/libr_plugin.so ${CMAKE_SOURCE_DIR}/plugin/libr_plugin.so
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Copy final library to folder"
)