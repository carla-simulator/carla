#! /bin/bash

source $(dirname "$0")/Environment.sh

FBX2OBJ_DIST=${CARLA_DOCKER_UTILS_FOLDER}/dist
FBX2OBJ_FOLDER=${CARLA_DOCKER_UTILS_FOLDER}/fbx
FBX2OBJ_BUILD_FOLDER=${FBX2OBJ_FOLDER}/build
FBX2OBJ_DEP_FOLDER=${FBX2OBJ_FOLDER}/dependencies

if [ -f "${FBX2OBJ_DIST}/FBX2OBJ" ]; then
  log "FBX SDK already installed."
  exit
fi

LIB_NAME=fbx202001_fbxsdk_linux
FBXSDK_URL=https://www.autodesk.com/content/dam/autodesk/www/adn/fbx/2020-0-1/${LIB_NAME}.tar.gz

if [ ! -d "${FBX2OBJ_DEP_FOLDER}" ]; then
  log "Downloading FBX SDK..."
  wget -c "${FBXSDK_URL}" -P "${CARLA_DOCKER_UTILS_FOLDER}"

  echo "Unpacking..."
  mkdir -p "${FBX2OBJ_DEP_FOLDER}"
  tar -xvzf "${CARLA_DOCKER_UTILS_FOLDER}/${LIB_NAME}.tar.gz" -C "${CARLA_DOCKER_UTILS_FOLDER}" "${LIB_NAME}"
  rm "${CARLA_DOCKER_UTILS_FOLDER}/${LIB_NAME}.tar.gz"

  echo "Installing FBX SDK..."
  echo -e "y\nyes\nn\n" | "${CARLA_DOCKER_UTILS_FOLDER}/${LIB_NAME}" "${FBX2OBJ_DEP_FOLDER}"
  echo
  rm "${CARLA_DOCKER_UTILS_FOLDER}/${LIB_NAME}"
fi

log "Compiling FBX2OBJ..."
mkdir -p "${FBX2OBJ_DIST}"
mkdir -p "${FBX2OBJ_BUILD_FOLDER}"

pushd "${FBX2OBJ_BUILD_FOLDER}" >/dev/null

cmake -G "Ninja" \
    -DCMAKE_CXX_FLAGS="-fPIC -std=c++14" \
    ..

set +e

ninja

if [ $? -eq 1 ]; then
  fatal_error "Make sure \"libxml2-dev\" is installed using:\n\n    sudo apt-get install libxml2-dev\n"
  exit 1
fi

ninja install

set -e

popd >/dev/null

log "Success!"
