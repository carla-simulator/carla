#! /bin/bash
DOC_STRING="Build OSM2ODR."

USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help]

commands

    [--clean]    Clean intermediate files.
    [--rebuild]  Clean and rebuild both configurations.
END
)

REMOVE_INTERMEDIATE=false
BUILD_OSM2ODR=false
GIT_PULL=true
CURRENT_OSM2ODR_COMMIT=ee0c2b9241fef5365a6bc044ac82e6580b8ce936
OSM2ODR_BRANCH=carla_osm2odr
OSM2ODR_REPO=https://github.com/carla-simulator/sumo.git

OPTS=`getopt -o h --long help,rebuild,build,clean,carsim,no-pull -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --rebuild )
      REMOVE_INTERMEDIATE=true;
      BUILD_OSM2ODR=true;
      shift ;;
    --build )
      BUILD_OSM2ODR=true;
      shift ;;
    --no-pull )
      GIT_PULL=false
      shift ;;
    --clean )
      REMOVE_INTERMEDIATE=true;
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

source $(dirname "$0")/Environment.sh

function get_source_code_checksum {
  local EXCLUDE='*__pycache__*'
  find "${OSM2ODR_SOURCE_FOLDER}"/* \! -path "${EXCLUDE}" -print0 | sha1sum | awk '{print $1}'
}

if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_OSM2ODR}; }; then
  fatal_error "Nothing selected to be done."
fi

# ==============================================================================
# -- Clean intermediate files --------------------------------------------------
# ==============================================================================

if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  rm -Rf ${OSM2ODR_BUILD_FOLDER}*

fi

# ==============================================================================
# -- Build library -------------------------------------------------------------
# ==============================================================================

if ${BUILD_OSM2ODR} ; then
  log "Building OSM2ODR."
  # [ ! -d ${OSM2ODR_BUILD_FOLDER} ] && mkdir ${OSM2ODR_BUILD_FOLDER}
  if ${GIT_PULL} ; then
    if [ ! -d ${OSM2ODR_SOURCE_FOLDER} ] ; then
      git clone -b ${OSM2ODR_BRANCH} ${OSM2ODR_REPO} ${OSM2ODR_SOURCE_FOLDER}
    fi
    cd ${OSM2ODR_SOURCE_FOLDER}
    git fetch
    git checkout ${CURRENT_OSM2ODR_COMMIT}
  fi

  mkdir -p ${OSM2ODR_BUILD_FOLDER}
  cd ${OSM2ODR_BUILD_FOLDER}
  # define clang compiler
  export CC=/usr/bin/clang-8
  export CXX=/usr/bin/clang++-8

  cmake ${OSM2ODR_SOURCE_FOLDER} \
      -G "Eclipse CDT4 - Ninja" \
      -DCMAKE_INSTALL_PREFIX=${LIBCARLA_INSTALL_CLIENT_FOLDER} \
      -DPROJ_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/proj-install/include \
      -DPROJ_LIBRARY=${CARLA_BUILD_FOLDER}/proj-install/lib/libproj.a \
      -DXercesC_INCLUDE_DIR=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install/include \
      -DXercesC_LIBRARY=${CARLA_BUILD_FOLDER}/xerces-c-3.2.3-install/lib/libxerces-c.a

  ninja osm2odr
  ninja install

fi

log "Success!"
