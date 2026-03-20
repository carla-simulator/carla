#[[

  Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
  de Barcelona (UAB).

  This work is licensed under the terms of the MIT license.
  For a copy, see <https://opensource.org/licenses/MIT>.

]]

# ================================
#   CUDA Toolkit Detection
# ================================

find_package (CUDAToolkit QUIET)

if (CUDAToolkit_FOUND)
  carla_message ("CUDA Toolkit ${CUDAToolkit_VERSION} found at ${CUDAToolkit_TARGET_DIR}")

  if (CUDAToolkit_VERSION VERSION_LESS "12.8")
    carla_message (
      "CUDA ${CUDAToolkit_VERSION} detected. "
      "CUDA 12.8+ is recommended for NVIDIA Blackwell (RTX 50 series / sm_100 / sm_120) GPU support."
    )
  endif ()
else ()
  carla_message ("CUDA Toolkit not found. CUDA-dependent features will be unavailable.")
endif ()

# ================================
#   PyTorch + CUDA Validation
# ================================

if (ENABLE_PYTORCH)
  if (NOT CUDAToolkit_FOUND)
    carla_warning (
      "ENABLE_PYTORCH=ON but CUDA Toolkit was not found. "
      "PyTorch GPU acceleration will not be available. "
      "Install CUDA Toolkit ${CARLA_CUDA_MIN_VERSION}+ for GPU support."
    )
  elseif (CUDAToolkit_VERSION VERSION_LESS "${CARLA_CUDA_MIN_VERSION}")
    carla_error (
      "ENABLE_PYTORCH=ON requires CUDA Toolkit >= ${CARLA_CUDA_MIN_VERSION}, "
      "but found ${CUDAToolkit_VERSION}. "
      "Please upgrade CUDA or set CARLA_CUDA_MIN_VERSION to a lower value."
    )
  else ()
    carla_message ("CUDA ${CUDAToolkit_VERSION} meets minimum requirement (${CARLA_CUDA_MIN_VERSION}).")
    add_compile_definitions (CARLA_WITH_CUDA)
    set (CMAKE_CUDA_ARCHITECTURES "${CARLA_CUDA_ARCHITECTURES}")
    carla_message ("CUDA architectures: ${CMAKE_CUDA_ARCHITECTURES}")
  endif ()
endif ()
