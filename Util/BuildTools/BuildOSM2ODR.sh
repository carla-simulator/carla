#! /bin/bash

source $(dirname "$0")/Environment.sh

# define clang compiler
export CC=/usr/bin/clang-8
export CXX=/usr/bin/clang++-8

OSM2ODR_BASE_DIR=${CARLA_ROOT_FOLDER}/Util/OSM2ODR
OSM2ODR_BUILD_DIR=${CARLA_ROOT_FOLDER}/Build/osm2odr-build
OSM2ODR_BIN_DIR=${OSM2ODR_BASE_DIR}/bin
[ ! -d ${OSM2ODR_BUILD_DIR} ] && mkdir ${OSM2ODR_BUILD_DIR}

cd ${OSM2ODR_BUILD_DIR}

case $1 in
  --rebuild)
    rm -r * || true
    ;;
esac

cmake ${OSM2ODR_BASE_DIR} \
    -G "Eclipse CDT4 - Ninja" \
    -DCMAKE_INSTALL_PREFIX=${LIBCARLA_INSTALL_CLIENT_FOLDER}

ninja osm2odr
ninja install
