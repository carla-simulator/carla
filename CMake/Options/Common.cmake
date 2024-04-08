#[[

  Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
  de Barcelona (UAB).
  
  This work is licensed under the terms of the MIT license.
  For a copy, see <https://opensource.org/licenses/MIT>.

]]

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
  OFF
)

carla_option (
  ENABLE_EXCEPTIONS
  "Enable C++ Exceptions."
  OFF
)

carla_option (
  PREFER_CLONE
  "Whether to clone dependencies instead of directly downloading a compressed archive."
  OFF
)

carla_option (
  BUILD_PYTHON_API_WHEEL_PACKAGE
  "Whether to build the CARLA python API wheel package."
  ON
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
  png
)

carla_string_option (  
  GCC_COMPILER
  "gcc compiler used by some CARLA extensions."
  /usr/bin/gcc-7
)

carla_string_option (
  GXX_COMPILER
  "g++ compiler used by some CARLA extensions."
  /usr/bin/g++-7
)
