# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Optional JPEG support uses a static codec, like the vendored PNG/zlib
# dependencies. libjpeg-turbo explicitly forbids add_subdirectory(), so build
# and install it privately with ExternalProject instead of FetchContent.
set (CARLA_JPEG_ENABLED OFF)
set (CARLA_JPEG_LIBRARIES)
foreach (FORMAT ${LIBCARLA_IMAGE_SUPPORTED_FORMATS})
  string (TOLOWER "${FORMAT}" FORMAT_LOWERCASE)
  if (FORMAT_LOWERCASE STREQUAL "jpeg")
    set (CARLA_JPEG_ENABLED ON)
  endif ()
endforeach ()
if (NOT CARLA_JPEG_ENABLED)
  return ()
endif ()

include (ExternalProject)

set (CARLA_JPEG_INSTALL_ROOT ${CMAKE_BINARY_DIR}/_deps/libjpeg-turbo-install)
get_property (CARLA_JPEG_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
set (CARLA_JPEG_EXTERNAL_CONFIG_ARGS)
if (CARLA_JPEG_MULTI_CONFIG)
  # Both the child cache and its install must be separate per configuration.
  # Otherwise installing Release can replace the archive/header used by Debug,
  # or a later install can reuse the other configuration's CMake cache.
  set (CARLA_JPEG_PREFIX ${CARLA_JPEG_INSTALL_ROOT}/$<CONFIG>)
  set (CARLA_JPEG_BINARY_DIR ${CMAKE_BINARY_DIR}/_deps/libjpeg-turbo-build/$<CONFIG>)
  foreach (CONFIGURATION ${CMAKE_CONFIGURATION_TYPES})
    file (MAKE_DIRECTORY ${CARLA_JPEG_INSTALL_ROOT}/${CONFIGURATION}/include)
  endforeach ()
else ()
  set (CARLA_JPEG_PREFIX ${CARLA_JPEG_INSTALL_ROOT})
  file (MAKE_DIRECTORY ${CARLA_JPEG_PREFIX}/include)
endif ()
if (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
  set (CARLA_JPEG_ARCHIVE_NAME jpeg-static.lib)
else ()
  set (CARLA_JPEG_ARCHIVE_NAME libjpeg.a)
endif ()
set (CARLA_JPEG_ARCHIVE ${CARLA_JPEG_PREFIX}/lib/${CARLA_JPEG_ARCHIVE_NAME})

set (CARLA_JPEG_CMAKE_ARGS
  -DCMAKE_INSTALL_PREFIX:PATH=${CARLA_JPEG_PREFIX}
  -DCMAKE_INSTALL_LIBDIR:STRING=lib
  -DCMAKE_INSTALL_INCLUDEDIR:STRING=include
  -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
  -DENABLE_SHARED:BOOL=OFF
  -DENABLE_STATIC:BOOL=ON
  -DWITH_CRT_DLL:BOOL=ON
  -DWITH_TURBOJPEG:BOOL=OFF
  -DWITH_TOOLS:BOOL=OFF
  -DWITH_TESTS:BOOL=OFF
)
foreach (VARIABLE CMAKE_TOOLCHAIN_FILE CMAKE_C_COMPILER_TARGET CMAKE_SYSROOT
    CMAKE_MSVC_RUNTIME_LIBRARY CMAKE_MAKE_PROGRAM CMAKE_GENERATOR_INSTANCE
    CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_OSX_ARCHITECTURES CMAKE_OSX_DEPLOYMENT_TARGET CMAKE_OSX_SYSROOT)
  if (DEFINED ${VARIABLE} AND NOT "${${VARIABLE}}" STREQUAL "")
    list (APPEND CARLA_JPEG_CMAKE_ARGS "-D${VARIABLE}:STRING=${${VARIABLE}}")
  endif ()
endforeach ()

# The engine root may exist only in the parent's CMake cache. Environment
# changes made by its toolchain do not persist into a later `cmake --build`.
set (CARLA_JPEG_UNREAL_ENGINE_PATH "${CARLA_UNREAL_ENGINE_PATH}")
if ("${CARLA_JPEG_UNREAL_ENGINE_PATH}" STREQUAL "" AND NOT "${UE_ROOT}" STREQUAL "")
  set (CARLA_JPEG_UNREAL_ENGINE_PATH "${UE_ROOT}")
endif ()
if (NOT "${CARLA_JPEG_UNREAL_ENGINE_PATH}" STREQUAL "")
  list (APPEND CARLA_JPEG_CMAKE_ARGS
    "-DCARLA_UNREAL_ENGINE_PATH:PATH=${CARLA_JPEG_UNREAL_ENGINE_PATH}")
endif ()
if (NOT "${UE_ROOT}" STREQUAL "")
  list (APPEND CARLA_JPEG_CMAKE_ARGS "-DUE_ROOT:PATH=${UE_ROOT}")
endif ()

# Immutable libjpeg-turbo 3.1.4.1 release; no codec is downloaded for PNG-only
# builds. ExternalProject's URL override also permits an offline local archive.
set (CARLA_LIBJPEG_TURBO_URL
  "https://github.com/libjpeg-turbo/libjpeg-turbo/archive/9217719d3a58633923b096af4c1d50d304768a64.zip"
  CACHE STRING "libjpeg-turbo 3.1.4.1 source archive (or an identical local archive).")
set (CARLA_JPEG_SOURCE_URL "${CARLA_LIBJPEG_TURBO_URL}")
if (EXISTS "${CARLA_JPEG_SOURCE_URL}")
  file (TO_CMAKE_PATH "${CARLA_JPEG_SOURCE_URL}" CARLA_JPEG_SOURCE_URL)
endif ()
if (CARLA_JPEG_MULTI_CONFIG)
  set (CARLA_JPEG_GENERATOR_ARGS -G "${CMAKE_GENERATOR}")
  if (CMAKE_GENERATOR_PLATFORM)
    list (APPEND CARLA_JPEG_GENERATOR_ARGS -A "${CMAKE_GENERATOR_PLATFORM}")
  endif ()
  if (CMAKE_GENERATOR_TOOLSET)
    list (APPEND CARLA_JPEG_GENERATOR_ARGS -T "${CMAKE_GENERATOR_TOOLSET}")
  endif ()
  # ExternalProject creates BINARY_DIR at configure time, before generator
  # expressions can select a configuration. Use explicit commands so each
  # configuration gets its own actual child cache without a literal $<CONFIG>
  # directory or a shared, last-configured CMake cache.
  list (APPEND CARLA_JPEG_EXTERNAL_CONFIG_ARGS
    CONFIGURE_COMMAND ${CMAKE_COMMAND}
      -S <SOURCE_DIR> -B ${CARLA_JPEG_BINARY_DIR}
      ${CARLA_JPEG_GENERATOR_ARGS} ${CARLA_JPEG_CMAKE_ARGS}
    BUILD_COMMAND ${CMAKE_COMMAND} --build ${CARLA_JPEG_BINARY_DIR} --config $<CONFIG>
    INSTALL_COMMAND ${CMAKE_COMMAND} --install ${CARLA_JPEG_BINARY_DIR} --config $<CONFIG>)
endif ()
ExternalProject_Add (
  carla-jpeg-build
  PREFIX ${CMAKE_BINARY_DIR}/_deps/libjpeg-turbo
  URL ${CARLA_JPEG_SOURCE_URL}
  URL_HASH SHA256=65c86c4ef12019f0a399acd3e72236fd87c3c0e16b5632a7ce82eb0aa0a98676
  DOWNLOAD_EXTRACT_TIMESTAMP FALSE
  CMAKE_ARGS ${CARLA_JPEG_CMAKE_ARGS}
  INSTALL_BYPRODUCTS ${CARLA_JPEG_ARCHIVE}
  ${CARLA_JPEG_EXTERNAL_CONFIG_ARGS}
)
add_library (carla-jpeg STATIC IMPORTED GLOBAL)
set_target_properties (carla-jpeg PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CARLA_JPEG_PREFIX}/include
)
if (CARLA_JPEG_MULTI_CONFIG)
  foreach (CONFIGURATION ${CMAKE_CONFIGURATION_TYPES})
    string (TOUPPER "${CONFIGURATION}" CONFIGURATION_UPPERCASE)
    set_property (TARGET carla-jpeg APPEND PROPERTY IMPORTED_CONFIGURATIONS ${CONFIGURATION})
    set_target_properties (carla-jpeg PROPERTIES
      IMPORTED_LOCATION_${CONFIGURATION_UPPERCASE}
        ${CARLA_JPEG_INSTALL_ROOT}/${CONFIGURATION}/lib/${CARLA_JPEG_ARCHIVE_NAME})
  endforeach ()
else ()
  set_target_properties (carla-jpeg PROPERTIES IMPORTED_LOCATION ${CARLA_JPEG_ARCHIVE})
endif ()
add_dependencies (carla-jpeg carla-jpeg-build)
set (CARLA_JPEG_LIBRARIES carla-jpeg)
