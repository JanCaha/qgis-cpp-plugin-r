# #######################################################
# Files
set(r_plugin_SRCS
  rplugin.cpp

  qgsrstatsapplicationwrapper.cpp
  qgsrstatsfunctions.cpp
  qgsrstatsmaplayerwrapper.cpp
  qgsrstatsrunner.cpp
  qgsrstatssession.cpp
  qgsrstatsutils.cpp

  qgsrstatsconsole.cpp
  qgsrstatssettings.cpp
)

set(r_plugin_RCCS
  pluginres.qrc
)

SET(PUBLIC_HEADERS_FILES
  rplugin.h

  qgsrstatsapplicationwrapper.h
  qgsrstatsfunctions.h
  qgsrstatsmaplayerwrapper.h
  qgsrstatsrunner.h
  qgsrstatssession.h
  qgsrstatsutils.h

  qgsrstatsconsole.h
  qgsrstatssettings.h
)

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
  ${INCLUDES}
)

ADD_CUSTOM_COMMAND(
  TARGET r_plugin POST_BUILD
  COMMAND cp ${CMAKE_BINARY_DIR}/src/libr_plugin.so ${CMAKE_SOURCE_DIR}/plugin/libr_plugin.so
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Copy final library to folder"
)

if(DOCKER_RELEASE)
  ADD_CUSTOM_COMMAND(
    TARGET r_plugin POST_BUILD
    COMMAND cp ${CMAKE_BINARY_DIR}/src/libr_plugin.so ${QGIS_PLUGIN_DIR}/libr_plugin.so
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Copy final library to QGIS plugins folder."
  )
endif()