if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  cmake_minimum_required (VERSION 3.16)
  project (carla_studio_app LANGUAGES CXX)
  get_filename_component (CARLA_WORKSPACE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
endif ()

# -----------------------------------------------------------------------------
# CARLA Studio versioning (semantic — bump per the rules in CHANGELOG.md):
#   MAJOR (X.0.0) for incompatible API / breaking overhauls
#   MINOR (0.Y.0) for backward-compatible new features
#   PATCH (0.0.Z) for backward-compatible bug fixes
# Version is exported as a compile-time #define so the GUI can show it in the
# top-right of the menu bar without needing a separate header. The value here
# is the source of truth — bump in lockstep with the matching CHANGELOG.md
# entry sitting next to this file.
# -----------------------------------------------------------------------------
set (CARLA_STUDIO_VERSION "1.1.0")

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

set (CARLA_STUDIO_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set (CARLA_STUDIO_RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set (CARLA_STUDIO_APP_SOURCE "${CARLA_STUDIO_SOURCE_DIR}/app/carla_studio.cpp")
set (CARLA_STUDIO_APP_TEST_SOURCE "${CARLA_STUDIO_SOURCE_DIR}/tests/CarlaStudioAppTest.cpp")
set (CARLA_STUDIO_QRC "${CARLA_STUDIO_RESOURCE_DIR}/carla-studio.qrc")

find_package (Qt6 COMPONENTS Widgets Core QUIET)
if (Qt6_FOUND)
  set (CARLA_STUDIO_QT_VERSION 6)
  set (CARLA_STUDIO_QT_WIDGETS_TARGET Qt6::Widgets)
  set (CARLA_STUDIO_QT_CORE_TARGET Qt6::Core)
  find_package (Qt6 COMPONENTS Gamepad Test QUIET)
else ()
  find_package (Qt5 COMPONENTS Widgets Core REQUIRED)
  set (CARLA_STUDIO_QT_VERSION 5)
  set (CARLA_STUDIO_QT_WIDGETS_TARGET Qt5::Widgets)
  set (CARLA_STUDIO_QT_CORE_TARGET Qt5::Core)
  find_package (Qt5 COMPONENTS Gamepad Test QUIET)
endif ()

carla_message ("CARLA Building CarlaStudio application (top-layer app module)")

add_library (
  carla-studio-core STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/core/StudioAppContext.cpp
)

target_include_directories (
  carla-studio-core PUBLIC
  ${CARLA_STUDIO_SOURCE_DIR}
)

target_link_libraries (
  carla-studio-core PUBLIC
  ${CARLA_STUDIO_QT_CORE_TARGET}
)

# Cross-cutting helpers: hardware probes (ResourceFit), storage volume
# enumeration, license/resource extraction. Pure-Qt, no LibCarla coupling,
# no external-system glue — that lives under integrations/ instead.
add_library (
  carla-studio-utils STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/ResourceFit.cpp
)

target_include_directories (
  carla-studio-utils PUBLIC
  ${CARLA_STUDIO_SOURCE_DIR}
)

target_link_libraries (
  carla-studio-utils PUBLIC
  ${CARLA_STUDIO_QT_CORE_TARGET}
)

# External-system glue: Hugging Face auth/API, Brev CLI wrapper, uv env
# manager, third-party tool registry, JSON-driven launch dispatcher. Each
# module here knows about ONE external system and stays narrowly scoped.
# Generic helpers belong in carla-studio-utils.
add_library (
  carla-studio-integ STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/integrations/HuggingFace.cpp
)

target_include_directories (
  carla-studio-integ PUBLIC
  ${CARLA_STUDIO_SOURCE_DIR}
)

target_link_libraries (
  carla-studio-integ PUBLIC
  ${CARLA_STUDIO_QT_CORE_TARGET}
  carla-studio-utils
)

add_library (
  carla-studio-setup-wizard STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/setup_wizard/PrereqProbe.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/setup_wizard/SetupWizardDialog.cpp
)

set_target_properties (
  carla-studio-setup-wizard PROPERTIES
  AUTOMOC ON
)

target_include_directories (
  carla-studio-setup-wizard PUBLIC
  ${CARLA_STUDIO_SOURCE_DIR}
)

target_link_libraries (
  carla-studio-setup-wizard PUBLIC
  ${CARLA_STUDIO_QT_WIDGETS_TARGET}
)

add_library (
  carla-studio-vehicle-import STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vehicle_import/MeshAABB.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vehicle_import/ObjSanitizer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vehicle_import/ImporterClient.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vehicle_import/BPAutopicker.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vehicle_import/PrebuiltPackagePage.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vehicle_import/VehicleImportPage.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vehicle_import/VehicleImportContainer.cpp
)

set_target_properties (
  carla-studio-vehicle-import PROPERTIES
  AUTOMOC ON
)

target_include_directories (
  carla-studio-vehicle-import PUBLIC
  ${CARLA_STUDIO_SOURCE_DIR}
)

target_link_libraries (
  carla-studio-vehicle-import PUBLIC
  ${CARLA_STUDIO_QT_WIDGETS_TARGET}
  ${CARLA_STUDIO_QT_CONCURRENT_TARGET}
)

# Platform-specific icon resources that live alongside the executable sources.
set (CARLA_STUDIO_PLATFORM_SOURCES "")

if (WIN32)
  # Windows: compile the .rc referencing carla-studio.ico into the executable.
  set (CARLA_STUDIO_WIN32_RC "${CARLA_STUDIO_RESOURCE_DIR}/windows/carla-studio.rc")
  list (APPEND CARLA_STUDIO_PLATFORM_SOURCES ${CARLA_STUDIO_WIN32_RC})
endif ()

if (APPLE)
  # macOS: bundle the .icns inside the .app; set_source_files_properties
  # routes the file to Contents/Resources and MACOSX_BUNDLE_ICON_FILE gives it
  # to the bundle's Info.plist.
  set (CARLA_STUDIO_MACOS_ICNS "${CARLA_STUDIO_RESOURCE_DIR}/macos/carla-studio.icns")
  set_source_files_properties (
    ${CARLA_STUDIO_MACOS_ICNS} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
  )
  list (APPEND CARLA_STUDIO_PLATFORM_SOURCES ${CARLA_STUDIO_MACOS_ICNS})
endif ()

carla_add_executable (
  carla-studio
  "Build the CARLA Studio main application."
  WIN32            # MACOSX_BUNDLE is applied via property below (keeps CMake < 3.16 compat path consistent)
  ${CARLA_STUDIO_APP_SOURCE}
  ${CARLA_STUDIO_QRC}
  ${CARLA_STUDIO_PLATFORM_SOURCES}
)

set_target_properties (
  carla-studio PROPERTIES
  AUTOMOC ON
  AUTORCC ON
)

if (APPLE)
  set_target_properties (
    carla-studio PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "CARLA Studio"
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.carla-simulator.CarlaStudio"
    MACOSX_BUNDLE_ICON_FILE "carla-studio.icns"
  )
endif ()

target_link_libraries (
  carla-studio PUBLIC
  ${CARLA_STUDIO_QT_WIDGETS_TARGET}
  carla-studio-core
  carla-studio-utils
  carla-studio-integ
  carla-studio-vehicle-import
  carla-studio-setup-wizard
)

# Export version as a compile-time string. Quoted-stringify so the source can
# use it directly as a string literal (`CARLA_STUDIO_VERSION_STRING`).
target_compile_definitions (
  carla-studio PRIVATE
  "CARLA_STUDIO_VERSION_STRING=\"${CARLA_STUDIO_VERSION}\""
)

if (CARLA_STUDIO_QT_VERSION EQUAL 6)
  if (TARGET Qt6::Gamepad)
    target_link_libraries (carla-studio PUBLIC Qt6::Gamepad)
    target_compile_definitions (carla-studio PRIVATE CARLA_QT_GAMEPAD_ENABLED)
  endif ()
else ()
  if (TARGET Qt5::Gamepad)
    target_link_libraries (carla-studio PUBLIC Qt5::Gamepad)
    target_compile_definitions (carla-studio PRIVATE CARLA_QT_GAMEPAD_ENABLED)
  endif ()
endif ()

if (TARGET carla-client)
  target_link_libraries (carla-studio PUBLIC carla-client)
  target_compile_definitions (carla-studio PRIVATE CARLA_STUDIO_WITH_LIBCARLA)
else ()
  carla_warning ("carla-client target is unavailable; configure from the CARLA workspace root with BUILD_CARLA_CLIENT=ON and BUILD_CARLA_STUDIO=ON for full LibCarla integration.")
endif ()

# Linux install rules: hicolor icons + .desktop entry so the app appears in menus.
if (UNIX AND NOT APPLE)
  include (GNUInstallDirs)
  set (CARLA_STUDIO_LINUX_ICON_DIR "${CARLA_STUDIO_RESOURCE_DIR}/linux/icons/hicolor")
  foreach (ICON_SIZE 16x16 22x22 24x24 32x32 48x48 64x64 96x96 128x128 192x192 256x256 512x512 scalable)
    install (
      FILES "${CARLA_STUDIO_LINUX_ICON_DIR}/${ICON_SIZE}/apps/carla-studio.png"
      DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/${ICON_SIZE}/apps"
      OPTIONAL
    )
  endforeach ()
  install (
    FILES "${CARLA_STUDIO_RESOURCE_DIR}/linux/carla-studio.desktop"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
    OPTIONAL
  )
  install (TARGETS carla-studio RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif ()

if (APPLE)
  include (GNUInstallDirs)
  install (TARGETS carla-studio BUNDLE DESTINATION ".")
endif ()

if (WIN32)
  include (GNUInstallDirs)
  install (TARGETS carla-studio RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif ()

# License + third-party manifest. Both are bundled as Qt resources for the
# in-app "Help → License" dialog AND installed to the standard share/doc
# location so distro packagers can collect them with `dh_installdocs` or
# equivalent. The Qt resource path is the source of truth at runtime.
include (GNUInstallDirs)
install (
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSES/THIRD_PARTY.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/CITATION.cff"
  DESTINATION "${CMAKE_INSTALL_DOCDIR}"
  OPTIONAL
)

set (CARLA_STUDIO_QT_TEST_TARGET "")
if (CARLA_STUDIO_QT_VERSION EQUAL 6)
  find_package (Qt6 COMPONENTS Test QUIET)
  if (TARGET Qt6::Test)
    set (CARLA_STUDIO_QT_TEST_TARGET Qt6::Test)
  endif ()
else ()
  find_package (Qt5 COMPONENTS Test QUIET)
  if (TARGET Qt5::Test)
    set (CARLA_STUDIO_QT_TEST_TARGET Qt5::Test)
  endif ()
endif ()
carla_message ("CARLA Studio Qt Test target: ${CARLA_STUDIO_QT_TEST_TARGET}")

include (CTest)

if (BUILD_TESTING AND UNIX)
  add_test (
    NAME carla-studio-ue4-quick-launch-kill
    COMMAND /usr/bin/env bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/quick_runtime_checks.sh ue4
  )
  add_test (
    NAME carla-studio-ue5-quick-launch-kill
    COMMAND /usr/bin/env bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/quick_runtime_checks.sh ue5
  )
  add_test (
    NAME carla-studio-thirdparty-checks
    COMMAND /usr/bin/env bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/quick_runtime_checks.sh thirdparty
  )
endif ()

if (CARLA_STUDIO_QT_TEST_TARGET)
  add_executable (
    carla-studio-tests
    ${CARLA_STUDIO_APP_TEST_SOURCE}
  )

  set_target_properties (
    carla-studio-tests PROPERTIES
    AUTOMOC ON
  )

  target_link_libraries (
    carla-studio-tests PRIVATE
    ${CARLA_STUDIO_QT_TEST_TARGET}
    ${CARLA_STUDIO_QT_WIDGETS_TARGET}
    carla-studio-core
    carla-studio-utils
    carla-studio-vehicle-import
  )

  target_compile_definitions (
    carla-studio-tests PRIVATE
    "CARLA_STUDIO_SOURCE_PATH=\"${CARLA_STUDIO_APP_SOURCE}\""
  )

  if (BUILD_TESTING)
    add_test (NAME carla-studio-tests COMMAND carla-studio-tests)
  endif ()
endif ()
