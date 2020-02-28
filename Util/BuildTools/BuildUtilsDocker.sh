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

log "Downloading and preparing FBX SDK..."

LIB_NAME=fbx202001_fbxsdk_linux
FBXSDK_URL=https://www.autodesk.com/content/dam/autodesk/www/adn/fbx/2020-0-1/${LIB_NAME}.tar.gz

echo "Downloading FBX SDK 2020"

wget -c "${FBXSDK_URL}" -P "${CARLA_DOCKER_UTILS_FOLDER}"

echo "Unpacking..."
mkdir -p "${FBX2OBJ_DEP_FOLDER}"
tar -xvzf "${CARLA_DOCKER_UTILS_FOLDER}/${LIB_NAME}.tar.gz" -C "${CARLA_DOCKER_UTILS_FOLDER}" "${LIB_NAME}"
rm "${CARLA_DOCKER_UTILS_FOLDER}/${LIB_NAME}.tar.gz"

echo "Installing..."
echo -e "y\nyes\nn\n" | "${CARLA_DOCKER_UTILS_FOLDER}/${LIB_NAME}" "${FBX2OBJ_DEP_FOLDER}"
echo
rm "${CARLA_DOCKER_UTILS_FOLDER}/${LIB_NAME}"

echo "Compiling FBX2OBJ..."
mkdir -p "${FBX2OBJ_DIST}"

cmake -S "${FBX2OBJ_FOLDER}" -B "${FBX2OBJ_BUILD_FOLDER}"

make -C "${FBX2OBJ_BUILD_FOLDER}" install

log "Success!"
