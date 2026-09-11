#[[

  Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
  de Barcelona (UAB).

  This work is licensed under the terms of the MIT license.
  For a copy, see <https://opensource.org/licenses/MIT>.

]]

# Resolves CARLA_DLSS_SDK_PATH into CARLA_DLSS_SDK_RESOLVED, fetching the SDK
# when necessary, and defines CARLA_UE_DLSS_ENV_COMMAND — a `cmake -E env`
# prefix that every Unreal build/package/launch command must go through so
# UnrealBuildTool (build time) and NGX (runtime snippet loading) see DLSS_SDK
# regardless of the caller's shell environment. The engine's DLSSRRDenoiser
# plugin deliberately fails its build when DLSS_SDK is neither a valid SDK nor
# the literal "disabled"; resolving here surfaces a bad setup at configure
# time instead of minutes into an engine build.
#
# The SDK itself is NVIDIA-licensed and may not be redistributed inside the
# CARLA or CarlaUnreal/UnrealEngine repositories, so it is never vendored:
# it is cloned from NVIDIA's public repository under NVIDIA's own license
# terms (same contract as Util/SetupUtils/SetupDLSS.sh, which remains the
# shell-setup path and shares the ~/SDKs/DLSS default location).

if (WIN32)
  set (CARLA_DLSS_SDK_LIB_SUBDIR Windows_x86_64)
  # The /MD dynamic-CRT variant DLSSRRDenoiser.Build.cs links on Win64.
  set (CARLA_DLSS_SDK_STATIC_LIB x64/nvsdk_ngx_d.lib)
  set (CARLA_DLSS_SDK_DEFAULT_PATH "$ENV{USERPROFILE}/SDKs/DLSS")
else ()
  set (CARLA_DLSS_SDK_LIB_SUBDIR Linux_x86_64)
  set (CARLA_DLSS_SDK_STATIC_LIB libnvsdk_ngx.a)
  set (CARLA_DLSS_SDK_DEFAULT_PATH "$ENV{HOME}/SDKs/DLSS")
endif ()

# Same validity contract as SetupDLSS.sh and DLSSRRDenoiser.Build.cs: headers,
# the NGX static library, and the Ray Reconstruction model snippet.
function (carla_dlss_sdk_is_valid SDK_PATH OUT_RESULT)
  set (${OUT_RESULT} FALSE PARENT_SCOPE)
  if (NOT IS_DIRECTORY "${SDK_PATH}/include")
    return ()
  endif ()
  if (NOT EXISTS "${SDK_PATH}/lib/${CARLA_DLSS_SDK_LIB_SUBDIR}/${CARLA_DLSS_SDK_STATIC_LIB}")
    return ()
  endif ()
  file (
    GLOB
    CARLA_DLSS_SDK_RR_SNIPPETS
    "${SDK_PATH}/lib/${CARLA_DLSS_SDK_LIB_SUBDIR}/rel/*ngx*dlssd*"
  )
  if ("${CARLA_DLSS_SDK_RR_SNIPPETS}" STREQUAL "")
    return ()
  endif ()
  set (${OUT_RESULT} TRUE PARENT_SCOPE)
endfunction ()

string (TOLOWER "${CARLA_DLSS_SDK_PATH}" CARLA_DLSS_SDK_PATH_LOWER)

if ("${CARLA_DLSS_SDK_PATH_LOWER}" STREQUAL "disabled")

  # Explicit opt-out: the plugin builds as a no-op (NFOR denoiser and spatial
  # upscale fallbacks remain functional). Still forward the literal value so
  # the build result cannot depend on whatever DLSS_SDK the shell carries.
  set (CARLA_DLSS_SDK_RESOLVED disabled)
  carla_message ("DLSS support disabled (CARLA_DLSS_SDK_PATH=disabled).")

else ()

  set (CARLA_DLSS_SDK_RESOLVED "")

  if (NOT "${CARLA_DLSS_SDK_PATH}" STREQUAL "")
    carla_dlss_sdk_is_valid ("${CARLA_DLSS_SDK_PATH}" CARLA_DLSS_SDK_VALID)
    if (CARLA_DLSS_SDK_VALID)
      set (CARLA_DLSS_SDK_RESOLVED "${CARLA_DLSS_SDK_PATH}")
    else ()
      carla_warning (
        "CARLA_DLSS_SDK_PATH (\"${CARLA_DLSS_SDK_PATH}\") is not a valid DLSS SDK "
        "(missing include/, lib/${CARLA_DLSS_SDK_LIB_SUBDIR}/${CARLA_DLSS_SDK_STATIC_LIB} "
        "or the dlssd model snippet); falling back to ${CARLA_DLSS_SDK_DEFAULT_PATH}."
      )
    endif ()
  endif ()

  if ("${CARLA_DLSS_SDK_RESOLVED}" STREQUAL "")
    carla_dlss_sdk_is_valid ("${CARLA_DLSS_SDK_DEFAULT_PATH}" CARLA_DLSS_SDK_VALID)
    if (CARLA_DLSS_SDK_VALID)
      set (CARLA_DLSS_SDK_RESOLVED "${CARLA_DLSS_SDK_DEFAULT_PATH}")
    endif ()
  endif ()

  if ("${CARLA_DLSS_SDK_RESOLVED}" STREQUAL "")
    carla_message ("DLSS SDK not found; fetching into ${CARLA_DLSS_SDK_DEFAULT_PATH} ...")
    find_package (Git REQUIRED)
    cmake_path (GET CARLA_DLSS_SDK_DEFAULT_PATH PARENT_PATH CARLA_DLSS_SDK_PARENT_PATH)
    file (MAKE_DIRECTORY "${CARLA_DLSS_SDK_PARENT_PATH}")
    execute_process (
      COMMAND
        ${GIT_EXECUTABLE} clone --depth 1
        https://github.com/NVIDIA/DLSS.git
        "${CARLA_DLSS_SDK_DEFAULT_PATH}"
      RESULT_VARIABLE CARLA_DLSS_SDK_CLONE_RESULT
    )
    if (CARLA_DLSS_SDK_CLONE_RESULT EQUAL 0)
      carla_dlss_sdk_is_valid ("${CARLA_DLSS_SDK_DEFAULT_PATH}" CARLA_DLSS_SDK_VALID)
      if (CARLA_DLSS_SDK_VALID)
        set (CARLA_DLSS_SDK_RESOLVED "${CARLA_DLSS_SDK_DEFAULT_PATH}")
      endif ()
    endif ()
  endif ()

  if ("${CARLA_DLSS_SDK_RESOLVED}" STREQUAL "")
    carla_error (
      "Could not resolve the NVIDIA DLSS SDK. The engine's DLSSRRDenoiser "
      "plugin requires it; without it the DLSS-RR denoiser and DLSS upscaling "
      "would silently degrade. Either clone https://github.com/NVIDIA/DLSS and "
      "set CARLA_DLSS_SDK_PATH (or the DLSS_SDK environment variable) to the "
      "checkout, or deliberately build without DLSS support by configuring "
      "with -DCARLA_DLSS_SDK_PATH=disabled."
    )
  endif ()

  carla_message ("Using ${CARLA_DLSS_SDK_RESOLVED} as NVIDIA DLSS SDK path.")

endif ()

set (
  CARLA_UE_DLSS_ENV_COMMAND
  ${CMAKE_COMMAND} -E env "DLSS_SDK=${CARLA_DLSS_SDK_RESOLVED}"
)
