#! /bin/bash

source $(dirname "$0")/Environment.sh

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Build LibCarla."

USAGE_STRING="Usage: $0 [-h|--help] [--rebuild] [--server] [--client] [--clean]"

REMOVE_INTERMEDIATE=false
BUILD_SERVER=false
BUILD_CLIENT=false

OPTS=`getopt -o h --long help,rebuild,server,client,clean -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2 ; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --rebuild )
      REMOVE_INTERMEDIATE=true;
      BUILD_SERVER=true;
      BUILD_CLIENT=true;
      shift ;;
    --server )
      BUILD_SERVER=true;
      shift ;;
    --client )
      BUILD_CLIENT=true;
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
      break ;;
  esac
done

if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_SERVER} || ${BUILD_CLIENT}; }; then
  fatal_error "Nothing selected to be done."
fi

# ==============================================================================
# -- Clean intermediate files --------------------------------------------------
# ==============================================================================

if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  rm -Rf ${LIBCARLA_BUILD_SERVER_FOLDER} ${LIBCARLA_BUILD_CLIENT_FOLDER}
  rm -Rf ${LIBCARLA_INSTALL_SERVER_FOLDER} ${LIBCARLA_INSTALL_CLIENT_FOLDER}
  rm -f ${LIBCARLA_ROOT_FOLDER}/source/carla/Version.h

fi

# ==============================================================================
# -- Build Server configuration ------------------------------------------------
# ==============================================================================

if ${BUILD_SERVER} ; then

  log "Building LibCarla \"Server\" configuration."

  mkdir -p ${LIBCARLA_BUILD_SERVER_FOLDER}
  pushd "${LIBCARLA_BUILD_SERVER_FOLDER}" >/dev/null

  if [ ! -f "build.ninja" ]; then

    cmake \
        -G "Ninja" \
        -DCMAKE_BUILD_TYPE=Server \
        -DCMAKE_TOOLCHAIN_FILE=${LIBCPP_TOOLCHAIN_FILE} \
        -DCMAKE_INSTALL_PREFIX=${LIBCARLA_INSTALL_SERVER_FOLDER} \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        ${CARLA_ROOT_FOLDER}

  fi

  ninja

  ninja install | grep -v "Up-to-date:"

  popd >/dev/null

fi

# ==============================================================================
# -- Build Client configuration ------------------------------------------------
# ==============================================================================

if ${BUILD_CLIENT} ; then

  log "Building LibCarla \"Client\" configuration."

  mkdir -p ${LIBCARLA_BUILD_CLIENT_FOLDER}
  pushd "${LIBCARLA_BUILD_CLIENT_FOLDER}" >/dev/null

  if [ ! -f "build.ninja" ]; then

    cmake \
        -G "Ninja" \
        -DCMAKE_BUILD_TYPE=Client \
        -DCMAKE_TOOLCHAIN_FILE=${LIBSTDCPP_TOOLCHAIN_FILE} \
        -DCMAKE_INSTALL_PREFIX=${LIBCARLA_INSTALL_CLIENT_FOLDER} \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        ${CARLA_ROOT_FOLDER}

  fi

  ninja

  ninja install | grep -v "Up-to-date:"

  popd >/dev/null

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
