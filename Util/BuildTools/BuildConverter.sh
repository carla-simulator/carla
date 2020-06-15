#! /bin/bash

source $(dirname "$0")/Environment.sh

# define clang compiler
export CC=/usr/bin/clang-8
export CXX=/usr/bin/clang++-8

CONVERTER_BASE_DIR=${CARLA_ROOT_FOLDER}/Util/Converter
CONVERTER_BUILD_DIR=${CARLA_ROOT_FOLDER}/Build/converter-build
CONVERTER_BIN_DIR=${CONVERTER_BASE_DIR}/bin
[ ! -d ${CONVERTER_BUILD_DIR} ] && mkdir ${CONVERTER_BUILD_DIR}

cd ${CONVERTER_BUILD_DIR}

case $1 in
  --rebuild)
    rm -r * || true
    ;;
esac

cmake ${CONVERTER_BASE_DIR} \
    -G "Eclipse CDT4 - Ninja"

ninja netconvert
