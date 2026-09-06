if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required (VERSION 3.16)
  project (carla_qt_example LANGUAGES CXX)
endif ()

if (NOT COMMAND carla_message)
  function (carla_message msg)
    message (STATUS "CARLA: ${msg}")
  endfunction ()
endif ()

if (NOT COMMAND carla_warning)
  function (carla_warning msg)
    message (WARNING "${msg}")
  endfunction ()
endif ()

if (NOT COMMAND carla_add_executable)
  function (carla_add_executable target _description)
    add_executable (${target} ${ARGN})
  endfunction ()
endif ()

set (CARLA_EXAMPLE_QT_CLIENT_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

carla_message ("CARLA Building Qt C++ example: dockable community tool + plugin")

find_package (Qt6 COMPONENTS Widgets QUIET)

if (NOT Qt6_FOUND)
  find_package (Qt5 COMPONENTS Widgets QUIET)
  if (NOT Qt5_FOUND)
    carla_warning ("Qt6 or Qt5 not found. Skipping Qt example target.")
    return ()
  endif ()
  set (Qt_VERSION 5)
  set (CARLA_QT_WIDGETS Qt5::Widgets)
else ()
  set (Qt_VERSION 6)
  set (CARLA_QT_WIDGETS Qt6::Widgets)
endif ()

set (CARLA_QT_EXAMPLE_SOURCE ${CARLA_EXAMPLE_QT_CLIENT_SOURCE_PATH}/main.cpp)
set (CARLA_QT_EXAMPLE_INCLUDE ${CARLA_EXAMPLE_QT_CLIENT_SOURCE_PATH}/include)

# -----------------------------------------------------------------------------
# Standalone executable: a self-contained companion window the community can
# launch without CARLA Studio.
# -----------------------------------------------------------------------------
carla_add_executable (
  carla_studio_example_tool
  "Build the CARLA Studio community example tool (standalone)."
  WIN32
  ${CARLA_QT_EXAMPLE_SOURCE}
)

set_target_properties (
  carla_studio_example_tool PROPERTIES
  AUTOMOC ON
)

target_include_directories (
  carla_studio_example_tool PRIVATE
  ${CARLA_QT_EXAMPLE_INCLUDE}
)

target_link_libraries (
  carla_studio_example_tool PRIVATE
  ${CARLA_QT_WIDGETS}
)

# -----------------------------------------------------------------------------
# Plugin shared library: same source compiled without main() and dlopen-loaded
# by CARLA Studio at runtime.
# -----------------------------------------------------------------------------
add_library (
  carla_studio_example_plugin MODULE
  ${CARLA_QT_EXAMPLE_SOURCE}
)

# OUTPUT_NAME forces the produced shared library to follow the
# `qt-*.{so,dylib,dll}` naming convention CARLA Studio uses to
# discover Qt add-on plugins. The internal CMake target name stays
# `carla_studio_example_plugin` so existing references keep working;
# only the on-disk filename changes to `qt-carla-studio-example.so`.
set_target_properties (
  carla_studio_example_plugin PROPERTIES
  AUTOMOC ON
  PREFIX ""
  OUTPUT_NAME "qt-carla-studio-example"
  CXX_VISIBILITY_PRESET hidden
)

target_compile_definitions (
  carla_studio_example_plugin PRIVATE
  CARLA_STUDIO_BUILD_AS_PLUGIN
)

target_include_directories (
  carla_studio_example_plugin PRIVATE
  ${CARLA_QT_EXAMPLE_INCLUDE}
)

target_link_libraries (
  carla_studio_example_plugin PRIVATE
  ${CARLA_QT_WIDGETS}
)

# Install the plugin next to the CARLA Studio executable so it is picked up by
# the default plugin scan path (<bindir>/../plugins or ./plugins).
include (GNUInstallDirs)
install (
  TARGETS carla_studio_example_plugin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/carla-studio/plugins"
  RUNTIME DESTINATION "${CMAKE_INSTALL_LIBDIR}/carla-studio/plugins"
  OPTIONAL
)

install (
  TARGETS carla_studio_example_tool
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  OPTIONAL
)
