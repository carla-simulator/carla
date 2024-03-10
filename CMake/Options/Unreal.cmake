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

carla_string_option (
  CARLA_UNREAL_ENGINE_PATH
  "Path to the CARLA fork of Unreal Engine."
  ""
)

if (WIN32)
  set (CARLA_UNREAL_RHI_DEFAULT d3d12)
elseif (LINUX)
  set (CARLA_UNREAL_RHI_DEFAULT vulkan)
endif ()

carla_string_option (
  CARLA_UNREAL_RHI
  "Target CARLA Unreal Engine RHI."
  "vulkan"
  "${CARLA_UNREAL_RHI_DEFAULT}"
)

if (EXISTS ${CARLA_UNREAL_ENGINE_PATH})
  set (CARLA_HAS_UNREAL_ENGINE_PATH ON)
else ()
  set (CARLA_HAS_UNREAL_ENGINE_PATH OFF)
endif ()

if (${BUILD_CARLA_UNREAL})
  if (${CARLA_HAS_UNREAL_ENGINE_PATH})
    message ("Carla UE project successfully added to build. (UE path: ${CARLA_UNREAL_ENGINE_PATH})")
  else ()
    message (
      FATAL_ERROR
      "Could not add UE project to build since the option CARLA_UNREAL_ENGINE_PATH is not set to a valid path (\"${CARLA_UNREAL_ENGINE_PATH}\"). Please set it to point to the root path of your CARLA Unreal Engine installation.")
  endif ()
endif ()

carla_string_option (
  CARLA_LAUNCH_ARGS
  "CMake-style semicolon-separated list of arguments to pass when launching the Unreal Editor with CARLA."
  ""
)