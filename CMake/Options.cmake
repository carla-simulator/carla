#[[

  Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
  de Barcelona (UAB).
  
  This work is licensed under the terms of the MIT license.
  For a copy, see <https://opensource.org/licenses/MIT>.

]]



# ================================
#   Common Options
# ================================

carla_option (
  BUILD_CARLA_CLIENT
  "Build the CARLA client."
  ON
)

carla_option (
  BUILD_CARLA_SERVER
  "Build the CARLA server."
  ON
)

carla_option (
  BUILD_EXAMPLES
  "Build CARLA examples."
  ON
)

carla_option (
  BUILD_LIBCARLA_TESTS
  "Build LibCarla tests."
  ON
)

carla_option (
  ENABLE_PYTORCH
  "Enable PyTorch for CARLA."
  OFF
)

carla_option (
  BUILD_PYTHON_API
  "Build the CARLA Python API."
  ON
)

carla_option (
  ENABLE_ROS2
  "Enable ROS2."
  OFF
)

carla_option (
  ENABLE_ROS2_DEMO
  "Enable ROS2 demo."
  OFF
)

carla_option (
  ENABLE_OSM2ODR
  "Enable OSM2ODR."
  OFF
)

carla_option (
  ENABLE_RSS
  "Enable RSS components (ad-rss-lib)"
  OFF
)

carla_option (
  INSTALL_LIBCARLA
  "Install LibCarla and all of its dependencies."
  ON
)

carla_option (
  BUILD_OSM_WORLD_RENDERER
  "Build OSM World Renderer."
  OFF
)

carla_option (
  ENABLE_RTTI
  "Enable C++ RTTI."
  ON
)

carla_option (
  ENABLE_EXCEPTIONS
  "Enable C++ Exceptions."
  ON
)

carla_option (
  PREFER_CLONE
  "Whether to clone dependencies instead of directly downloading a compressed archive."
  OFF
)

carla_option (
  ENABLE_PEP517
  "Whether to use PEP 517."
  ON
)

carla_option (
  ENABLE_ALL_WARNINGS
  "Whether to emit extra build warnings."
  OFF
)

carla_option (
  ENABLE_WARNINGS_TO_ERRORS
  "Whether to convert build warnings to errors."
  OFF
)

carla_string_option (
  LIBCARLA_IMAGE_SUPPORTED_FORMATS
  "Semicolon-separated list of supported image formats by LibCarla. Available formats: png, jpeg, tiff."
  "png"
)

carla_option (
  VERBOSE_CONFIGURE
  "Whether to emit extra messages during CMake configure."
  OFF
)

carla_option (
  ENABLE_STREETMAP
  "Whether to download the Streetmap UE plugin."
  OFF
)



# ================================
#   Unreal Engine-related Options
# ================================

carla_option (
  CARLA_UNREAL_CONTENT_PREFER_CLONE
  "Whether to perform a git clone instead of a direct archive download of the CARLA Unreal assets (EXTREMELY DISCOURAGED)."
  OFF
)

cmake_path (
  CONVERT
  "$ENV{CARLA_UNREAL_ENGINE_PATH}"
  TO_CMAKE_PATH_LIST
  CARLA_UNREAL_ENGINE_PATH_INFERRED
  NORMALIZE
)

carla_string_option (
  CARLA_UNREAL_ENGINE_PATH
  "Path to the CARLA fork of Unreal Engine."
  "${CARLA_UNREAL_ENGINE_PATH_INFERRED}"
)

carla_message ("Using ${CARLA_UNREAL_ENGINE_PATH} as Unreal Engine root path.")

if (EXISTS ${CARLA_UNREAL_ENGINE_PATH})
  set (CARLA_HAS_UNREAL_ENGINE_PATH ON)
else ()
  set (CARLA_HAS_UNREAL_ENGINE_PATH OFF)
endif ()

carla_option (
  BUILD_CARLA_UNREAL
  "Build the Carla Unreal Engine project."
  ${CARLA_HAS_UNREAL_ENGINE_PATH}
)

if (WIN32)
  set (CARLA_UNREAL_RHI_DEFAULT d3d12)
elseif (LINUX)
  set (CARLA_UNREAL_RHI_DEFAULT vulkan)
endif ()

carla_string_option (
  CARLA_UNREAL_RHI
  "Target CARLA Unreal Engine RHI."
  "${CARLA_UNREAL_RHI_DEFAULT}"
)

if (BUILD_CARLA_UNREAL)
  if (NOT ${CARLA_HAS_UNREAL_ENGINE_PATH})
    carla_error (
      "Could not add UE project to build since the carla_option CARLA_UNREAL_ENGINE_PATH "
      "is not set to a valid path (\"${CARLA_UNREAL_ENGINE_PATH}\")."
      "Please set it to point to the root path of your CARLA Unreal Engine installation."
    )
  endif ()
endif ()

carla_option (
  CARLA_UNREAL_LOG_WINDOW
  "Whether to open a terminal window along the Unreal editor."
  ON
)

set (CARLA_LAUNCH_ARGS_DEFAULT)
if (CARLA_UNREAL_LOG_WINDOW)
  list (APPEND CARLA_LAUNCH_ARGS_DEFAULT -log)
endif ()

carla_string_option (
  CARLA_LAUNCH_ARGS
  "CMake-style semicolon-separated list of arguments to pass when launching the Unreal Editor with CARLA."
  "${CARLA_LAUNCH_ARGS_DEFAULT}"
)

carla_string_option (
  CARLA_UNREAL_BUILD_TYPE
  "Set the default CARLA Unreal Editor build configuration."
  "Development"
)

# Docs for UE5 build configurations:
# https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/DevelopmentSetup/BuildConfigurations/

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set (CARLA_UNREAL_PACKAGE_BUILD_TYPE_DEFAULT Debug)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
  set (CARLA_UNREAL_PACKAGE_BUILD_TYPE_DEFAULT Development)
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
  set (CARLA_UNREAL_PACKAGE_BUILD_TYPE_DEFAULT Shipping)
else ()
  carla_warning("Unexpected CMAKE_BUILD_TYPE \"${CMAKE_BUILD_TYPE}\". Unreal packages will default to Development. Manually override DEFAULT_PACKAGE_CONFIGURATION if this behavior is not desired.")
  set (CARLA_UNREAL_PACKAGE_BUILD_TYPE_DEFAULT Development)
endif ()

carla_string_option (
  CARLA_UNREAL_PACKAGE_BUILD_TYPE
  "Set the default CARLA package build configuration."
  "${CARLA_UNREAL_PACKAGE_BUILD_TYPE_DEFAULT}"
)



# ================================
#   Dependency-specific Options
# ================================

# ==== BOOST ====

carla_string_option (
  CARLA_BOOST_VERSION
  "Target boost version."
  1.84.0
)

carla_string_option (
  CARLA_BOOST_TAG
  "Target boost git tag."
  boost-${CARLA_BOOST_VERSION}
)

# ==== EIGEN ====

carla_string_option (
  CARLA_EIGEN_VERSION
  "Target Eigen version."
  3.4.0
)

carla_string_option (
  CARLA_EIGEN_TAG
  "Target Eigen git tag."
  ${CARLA_EIGEN_VERSION}
)

# ==== GOOGLE TEST ====

carla_string_option (
  CARLA_GTEST_VERSION
  "Target googletest version."
  1.14.0
)

carla_string_option (
  CARLA_GTEST_TAG
  "Target googletest git tag."
  v${CARLA_GTEST_VERSION}
)

# ==== LIBPNG ====

carla_string_option (
  CARLA_LIBPNG_VERSION
  "Target libpng version."
  1.6.40
)

carla_string_option (
  CARLA_LIBPNG_TAG
  "Target libpng git tag."
  v${CARLA_LIBPNG_VERSION}
)

# ==== PROJ ====

carla_string_option (
  CARLA_PROJ_VERSION
  "Target PROJ version."
  7.2.1
)

carla_string_option (
  CARLA_PROJ_TAG
  "Target PROJ git tag."
  ${CARLA_PROJ_VERSION}
)

# ==== RECAST ====

carla_string_option (
  CARLA_RECAST_TAG
  "Target recastnavigation git tag."
  carla
)

# ==== RPCLIB ====

carla_string_option (
  CARLA_RPCLIB_TAG
  "Target rpclib git tag."
  carla
)

# ==== SQLITE3 ====

carla_string_option (
  CARLA_SQLITE_VERSION
  "Target SQLite version (major[1].minor[2].patch[2].revision[2])."
  3.45.01.00
)

# ==== XERCESC ====

carla_string_option (
  CARLA_XERCESC_VERSION
  "Target xerces-c version."
  3.2.4
)

carla_string_option (
  CARLA_XERCESC_TAG
  "Target xerces-c git tag."
  v${CARLA_XERCESC_VERSION}
)

# ==== ZLIB ====

carla_string_option (
  CARLA_ZLIB_VERSION
  "Target zlib git tag."
  1.3.1
)

carla_string_option (
  CARLA_ZLIB_TAG
  "Target zlib git tag."
  v${CARLA_ZLIB_VERSION}
)
  
# ==== LUNASVG ====

carla_string_option (
  CARLA_LUNASVG_VERSION
  "Target lunasvg version."
  master
)

carla_string_option (
  CARLA_LUNASVG_TAG
  "Target lunasvg git tag."
  ${CARLA_LUNASVG_VERSION}
)

# ==== LIBOSMSCOUT ====

carla_string_option (
  CARLA_LIBOSMSCOUT_VERSION
  "Target libosmscout version."
  master
)

carla_string_option (
  CARLA_LIBOSMSCOUT_TAG
  "Target libosmscout git tag."
  ${CARLA_LIBOSMSCOUT_VERSION}
)

# ==== STREETMAP ====

carla_string_option (
  CARLA_STREETMAP_VERSION
  "Target StreetMap version."
  ue5.3
)

carla_string_option (
  CARLA_STREETMAP_TAG
  "Target StreetMap git tag."
  ${CARLA_STREETMAP_VERSION}
)

# ==== FASTDDS ====

carla_string_option (
  CARLA_FASTDDS_VERSION
  "Target Fast-DDS version."
  2.11.2
)

carla_string_option (
  CARLA_FASTDDS_TAG
  "Target Fast-DDS git tag."
  ${CARLA_FASTDDS_VERSION}
)

# ==== FASTCDR ====

carla_string_option (
  CARLA_FASTCDR_VERSION
  "Target Fast-CDR version."
  1.1.x
)

carla_string_option (
  CARLA_FASTCDR_TAG
  "Target Fast-CDR git tag."
  ${CARLA_FASTCDR_VERSION}
)

# ==== FOONATHAN MEMORY VENDOR ====

carla_string_option (
  CARLA_FOONATHAN_MEMORY_VENDOR_VERSION
  "Target foonathan_memory_vendor version."
  master
)

carla_string_option (
  CARLA_FOONATHAN_MEMORY_VENDOR_TAG
  "Target foonathan_memory_vendor git tag."
  ${CARLA_FOONATHAN_MEMORY_VENDOR_VERSION}
)
