#[[

  Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
  de Barcelona (UAB).
  
  This work is licensed under the terms of the MIT license.
  For a copy, see <https://opensource.org/licenses/MIT>.

]]

option (
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
  ${CARLA_UNREAL_ENGINE_PATH_INFERRED}
)

carla_message ("Using ${CARLA_UNREAL_ENGINE_PATH} as Unreal Engine root path.")

if (EXISTS ${CARLA_UNREAL_ENGINE_PATH})
  set (CARLA_HAS_UNREAL_ENGINE_PATH ON)
else ()
  set (CARLA_HAS_UNREAL_ENGINE_PATH OFF)
endif ()

option (
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
  ${CARLA_UNREAL_RHI_DEFAULT}
)

if (${BUILD_CARLA_UNREAL} AND ${CARLA_HAS_UNREAL_ENGINE_PATH})
  carla_message (
    "Carla UE project successfully added to build. (UE path: ${CARLA_UNREAL_ENGINE_PATH})"
  )
else ()
  carla_error (
    "Could not add UE project to build since the option CARLA_UNREAL_ENGINE_PATH"
    "is not set to a valid path (\"${CARLA_UNREAL_ENGINE_PATH}\")."
    "Please set it to point to the root path of your CARLA Unreal Engine installation."
  )
endif ()

carla_string_option (
  CARLA_LAUNCH_ARGS
  "CMake-style semicolon-separated list of arguments to pass when launching the Unreal Editor with CARLA."
  ""
)

carla_string_option (
  CARLA_UNREAL_BUILD_TYPE
  "Carla Unreal-style build type (Debug/Development/Shipping)."
  Development
)
