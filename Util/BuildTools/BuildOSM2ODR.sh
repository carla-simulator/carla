#! /bin/bash

source $(dirname "$0")/Environment.sh

function get_source_code_checksum {
  local EXCLUDE='*__pycache__*'
  find "${OSM2ODR_ROOT_FOLDER}"/* \! -path "${EXCLUDE}" -print0 | sha1sum | awk '{print $1}'
}

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

OPTS=`getopt -o h --long help,rebuild,build,clean -n 'parse-options' -- "$@"`

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
    --clean )
      REMOVE_INTERMEDIATE=true;
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      log "Here22. $1"
      shift ;;
  esac
done

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

  [ ! -d ${OSM2ODR_BUILD_FOLDER} ] && mkdir ${OSM2ODR_BUILD_FOLDER}
  cd ${OSM2ODR_BUILD_FOLDER}
  # define clang compiler
  export CC=/usr/bin/clang-8
  export CXX=/usr/bin/clang++-8

  cmake ${OSM2ODR_ROOT_FOLDER} \
      -G "Eclipse CDT4 - Ninja" \
      -DCMAKE_INSTALL_PREFIX=${LIBCARLA_INSTALL_CLIENT_FOLDER}

  ninja osm2odr
  ninja install

fi

log "Success!"
