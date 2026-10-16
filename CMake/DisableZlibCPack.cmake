#[[

  Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
  de Barcelona (UAB).

  This work is licensed under the terms of the MIT license.
  For a copy, see <https://opensource.org/licenses/MIT>.

]]

# zlib 1.3.2 adds a CPack "package" target. CARLA already owns that target
# for Unreal packages, so disable zlib's standalone packaging registration.
set (ZLIB_CMAKE_FILE "${ZLIB_SOURCE_DIR}/CMakeLists.txt")
file (READ "${ZLIB_CMAKE_FILE}" ZLIB_CMAKE_CONTENT)
string (REPLACE "include(CPack)" "# CPack is disabled when zlib is embedded by CARLA."
  PATCHED_ZLIB_CMAKE_CONTENT "${ZLIB_CMAKE_CONTENT}")
string (REPLACE "set(CPACK_INCLUDED TRUE)" "# CPack is disabled when zlib is embedded by CARLA."
  PATCHED_ZLIB_CMAKE_CONTENT "${PATCHED_ZLIB_CMAKE_CONTENT}")

if (ZLIB_CMAKE_CONTENT STREQUAL PATCHED_ZLIB_CMAKE_CONTENT)
  message (FATAL_ERROR "Unable to disable zlib's embedded CPack target")
endif ()

file (WRITE "${ZLIB_CMAKE_FILE}" "${PATCHED_ZLIB_CMAKE_CONTENT}")
