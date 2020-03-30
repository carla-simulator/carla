#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

#export CC=/usr/bin/clang-8
#export CXX=/usr/bin/clang++-8

source $(dirname "$0")/Environment.sh

# ==============================================================================
# -- Get and compile ad-rss -------------------------------------------
# ==============================================================================

pushd "${CARLA_ROOT_FOLDER}/dependencies/ad-rss" >/dev/null

if [ "${CMAKE_PREFIX_PATH}" == "" ]; then
  export CMAKE_PREFIX_PATH=${CARLA_BUILD_FOLDER}/boost-1.72.0-c8-install
else
  export CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}:${CARLA_BUILD_FOLDER}/boost-1.72.0-c8-install
fi

#after a fixing clang compile warnings and errors in components
# -DCMAKE_TOOLCHAIN_FILE=${CARLA_BUILD_FOLDER}/LibStdCppToolChain.cmake
colcon build --packages-up-to ad_rss_map_integration --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null

log "Success!"
