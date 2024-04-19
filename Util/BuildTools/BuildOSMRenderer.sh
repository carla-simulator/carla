#! /bin/bash

source $(dirname "$0")/Environment.sh

LIBOSMSCOUT_REPO=https://github.com/Framstag/libosmscout
LUNASVG_REPO=https://github.com/sammycage/lunasvg

LIBOSMSCOUT_SOURCE_FOLDER=${CARLA_BUILD_FOLDER}/libosmscout-source
LIBOSMSCOUT_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/libosmscout-build
LIBOSMSCOUT_COMMIT=e83e4881a4adc69c5a4bcc05de5e1f23ebf06238

LUNASVG_SOURCE_FOLDER=${CARLA_BUILD_FOLDER}/lunasvg-source
LUNASVG_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/lunasvg-build

OSM_RENDERER_SOURCE=${CARLA_ROOT_FOLDER}/osm-world-renderer
OSM_RENDERER_BUILD=${CARLA_BUILD_FOLDER}/osm-world-renderer-build

INSTALLATION_PATH=${OSM_RENDERER_SOURCE}/ThirdParties



# ==============================================================================
# -- Download and build libosmscout --------------------------------------------
# ==============================================================================
echo "Cloning libosmscout."
echo ${CARLA_BUILD_FOLDER}
if [ ! -d ${LIBOSMSCOUT_SOURCE_FOLDER} ] ; then
  git clone ${LIBOSMSCOUT_REPO} ${LIBOSMSCOUT_SOURCE_FOLDER}
fi

cd ${LIBOSMSCOUT_SOURCE_FOLDER}
git fetch
git checkout ${LIBOSMSCOUT_COMMIT}
cd ..

mkdir -p ${LIBOSMSCOUT_BUILD_FOLDER}
cd ${LIBOSMSCOUT_BUILD_FOLDER}

cmake ${LIBOSMSCOUT_SOURCE_FOLDER} \
    -DCMAKE_INSTALL_PREFIX=${INSTALLATION_PATH}

make
make install

# ==============================================================================
# -- Download and build lunasvg ------------------------------------------------
# ==============================================================================
echo "Cloning luna-svg"
if [ ! -d ${LUNASVG_SOURCE_FOLDER} ] ; then
  git clone ${LUNASVG_REPO} ${LUNASVG_SOURCE_FOLDER}
fi

mkdir -p ${LUNASVG_BUILD_FOLDER}
cd ${LUNASVG_BUILD_FOLDER}

cmake ${LUNASVG_SOURCE_FOLDER} \
    -DCMAKE_INSTALL_PREFIX=${INSTALLATION_PATH}

make
make install


# ==============================================================================
# -- Build osm-map-renderer tool -----------------------------------------------
# ==============================================================================
echo "Building osm-map-renderer"

mkdir -p ${OSM_RENDERER_BUILD}
cd ${OSM_RENDERER_BUILD}

cmake -DCMAKE_CXX_FLAGS="-std=c++17 -g -pthread -I${CARLA_BUILD_FOLDER}/boost-1.80.0-c10-install/include" \
    ${OSM_RENDERER_SOURCE}
make

echo "SUCCESS! Finishing setting up renderer."