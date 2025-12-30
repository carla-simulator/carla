#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Build and package CARLA Python API."

USAGE_STRING="Usage: $0 [-h|--help] [--rebuild] [--clean] [--python-version=VERSION] [--target-wheel-platform=PLATFORM]"

REMOVE_INTERMEDIATE=false
BUILD_RSS_VARIANT=false
BUILD_PYTHONAPI=true
INSTALL_PYTHONAPI=true

OPTS=`getopt -o h --long help,config:,rebuild,clean,rss,carsim,python-version:,build-wheel,target-wheel-platform:,packages:,clean-intermediate,all,xml,target-archive:, -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

PY_VERSION_LIST=3
TARGET_WHEEL_PLATFORM=

while [[ $# -gt 0 ]]; do
  case "$1" in
    --rebuild )
      REMOVE_INTERMEDIATE=true;
      BUILD_PYTHONAPI=true;
      INSTALL_PYTHONAPI=true;
      shift ;;
    --python-version )
      PY_VERSION_LIST="$2"
      shift 2 ;;
    --build-wheel )
      BUILD_PYTHONAPI=true;
      INSTALL_PYTHONAPI=false;
      shift ;;
    --target-wheel-platform )
      TARGET_WHEEL_PLATFORM="$2"
      shift 2 ;;
    --rss )
      BUILD_RSS_VARIANT=true;
      shift ;;
    --clean )
      REMOVE_INTERMEDIATE=true;
      BUILD_PYTHONAPI=false;
      INSTALL_PYTHONAPI=false;
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      shift ;;
  esac
done


export CC="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang"
export CXX="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang++"
export PATH="$UE4_ROOT/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v17_clang-10.0.1-centos7/x86_64-unknown-linux-gnu/bin:$PATH"

source $(dirname "$0")/Environment.sh

if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_PYTHONAPI} || ${BUILD_PYTHONAPI_WHEEL} ; }; then
  fatal_error "Nothing selected to be done."
fi

# Convert comma-separated string to array of unique elements.
IFS="," read -r -a PY_VERSION_LIST <<< "${PY_VERSION_LIST}"

pushd "${CARLA_PYTHONAPI_SOURCE_FOLDER}" >/dev/null

# ==============================================================================
# -- Clean intermediate files --------------------------------------------------
# ==============================================================================

if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  rm -Rf build dist source/carla.egg-info

  find source -name "*.so" -delete
  find source -name "__pycache__" -type d -exec rm -rf "{}" \;

fi

# ==============================================================================
# -- Build API -----------------------------------------------------------------
# ==============================================================================

if ${BUILD_RSS_VARIANT} ; then
  export BUILD_RSS_VARIANT=${BUILD_RSS_VARIANT}
fi


if ${BUILD_PYTHONAPI} ; then
  # Add patchelf to the path. Auditwheel relies on patchelf to repair ELF files.
  export PATH="${LIBCARLA_INSTALL_CLIENT_FOLDER}/bin:${PATH}"
  
  for PY_VERSION in ${PY_VERSION_LIST[@]} ; do
    log "Building Python API wheel for Python ${PY_VERSION}."
    /usr/bin/env python${PY_VERSION} -m build --wheel --outdir dist/.tmp .

    if ${INSTALL_PYTHONAPI} ; then
      log "Installing Python API for Python ${PY_VERSION}."
      /usr/bin/env python${PY_VERSION} -m pip install --force-reinstall dist/.tmp/$(ls dist/.tmp | grep .whl)
    fi

    if [[ -z ${TARGET_WHEEL_PLATFORM} ]] ; then
      cp dist/.tmp/$(ls dist/.tmp | grep .whl) dist
    else
      log "Tagging Python API wheel to ${TARGET_WHEEL_PLATFORM} for Python ${PY_VERSION}."
      /usr/bin/env python${PY_VERSION} -m wheel tags --platform-tag ${TARGET_WHEEL_PLATFORM} dist/.tmp/$(ls dist/.tmp | grep .whl)
      /usr/bin/env python${PY_VERSION} -m auditwheel repair --plat ${TARGET_WHEEL_PLATFORM} --wheel-dir dist dist/.tmp/$(ls dist/.tmp | grep ${TARGET_WHEEL_PLATFORM}.whl)
    fi
    rm -rf dist/.tmp
  done
fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null

log "Success!"
