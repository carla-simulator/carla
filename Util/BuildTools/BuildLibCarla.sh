#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Build LibCarla."

USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help]

Choose one or more build configurations

    [--server]   Build server-side configuration.
    [--client]   Build client-side configuration.

and choose one or more build options

    [--debug]    Build debug targets.
    [--release]  Build release targets.

Other commands

    [--clean]    Clean intermediate files.
    [--rebuild]  Clean and rebuild both configurations.
END
)

REMOVE_INTERMEDIATE=false
BUILD_SERVER=false
BUILD_CLIENT=false
BUILD_OPTION_DEBUG=false
BUILD_OPTION_RELEASE=false
BUILD_OPTION_DUMMY=false
BUILD_RSS_VARIANT=false
USE_PYTORCH=false
USE_ROS2=false

OPTS=`getopt -o h --long help,rebuild,server,client,clean,debug,release,rss,pytorch,carsim,ros2 -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --rebuild )
      REMOVE_INTERMEDIATE=true;
      BUILD_SERVER=true;
      BUILD_CLIENT=true;
      BUILD_OPTION_DEBUG=true;
      BUILD_OPTION_RELEASE=true;
      shift ;;
    --server )
      BUILD_SERVER=true;
      shift ;;
    --client )
      BUILD_CLIENT=true;
      shift ;;
    --clean )
      REMOVE_INTERMEDIATE=true;
      BUILD_OPTION_DUMMY=true;
      shift ;;
    --debug )
      BUILD_OPTION_DEBUG=true;
      shift ;;
    --release )
      BUILD_OPTION_RELEASE=true;
      shift ;;
    --pytorch )
      USE_PYTORCH=true;
      shift ;;
    --ros2 )
      USE_ROS2=true;
      shift ;;
    --rss )
      BUILD_RSS_VARIANT=true;
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
  find "${LIBCARLA_ROOT_FOLDER}"/* \! -path "${EXCLUDE}" -print0 | sha1sum | awk '{print $1}'
}

if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_SERVER} || ${BUILD_CLIENT}; }; then
  fatal_error "Nothing selected to be done."
fi

if ! { ${BUILD_OPTION_DUMMY} || ${BUILD_OPTION_DEBUG} || ${BUILD_OPTION_RELEASE} ; }; then
  fatal_error "Choose --debug and/or --release."
fi

# ==============================================================================
# -- Clean intermediate files --------------------------------------------------
# ==============================================================================

if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  rm -Rf ${LIBCARLA_BUILD_SERVER_FOLDER}* ${LIBCARLA_BUILD_CLIENT_FOLDER}*
  rm -Rf ${LIBCARLA_BUILD_PYTORCH_FOLDER}* ${LIBCARLA_BUILD_PYTORCH_FOLDER}*
  rm -Rf ${LIBCARLA_INSTALL_SERVER_FOLDER} ${LIBCARLA_INSTALL_CLIENT_FOLDER}

fi

# ==============================================================================
# -- Define build function -----------------------------------------------------
# ==============================================================================

# Build LibCarla for the given configuration.
#
#     usage: build_libcarla {Server,Client,ClientRSS} {Debug,Release}
#
function build_libcarla {

  CMAKE_EXTRA_OPTIONS=''

  if [ $1 == Server ] ; then
    M_TOOLCHAIN=${LIBCPP_TOOLCHAIN_FILE}
    M_BUILD_FOLDER=${LIBCARLA_BUILD_SERVER_FOLDER}.$(echo "$2" | tr '[:upper:]' '[:lower:]')
    M_INSTALL_FOLDER=${LIBCARLA_INSTALL_SERVER_FOLDER}
  elif [ $1 == Client ] ; then
    M_TOOLCHAIN=${LIBSTDCPP_TOOLCHAIN_FILE}
    M_BUILD_FOLDER=${LIBCARLA_BUILD_CLIENT_FOLDER}.$(echo "$2" | tr '[:upper:]' '[:lower:]')
    M_INSTALL_FOLDER=${LIBCARLA_INSTALL_CLIENT_FOLDER}
  elif [ $1 == Pytorch ] ; then
    M_TOOLCHAIN=${LIBSTDCPP_TOOLCHAIN_FILE}
    M_BUILD_FOLDER=${LIBCARLA_BUILD_PYTORCH_FOLDER}.$(echo "$2" | tr '[:upper:]' '[:lower:]')
    M_INSTALL_FOLDER=${LIBCARLA_INSTALL_SERVER_FOLDER}
  elif [ $1 == ros2 ] ; then
    M_TOOLCHAIN=${LIBSTDCPP_TOOLCHAIN_FILE}
    M_BUILD_FOLDER=${LIBCARLA_FASTDDS_FOLDER}.$(echo "$2" | tr '[:upper:]' '[:lower:]')
    M_INSTALL_FOLDER=${LIBCARLA_INSTALL_SERVER_FOLDER}
  elif [ $1 == ClientRSS ] ; then
    BUILD_TYPE='Client'
    M_TOOLCHAIN=${LIBSTDCPP_TOOLCHAIN_FILE}
    M_BUILD_FOLDER=${LIBCARLA_BUILD_CLIENT_FOLDER}.rss.$(echo "$2" | tr '[:upper:]' '[:lower:]')
    M_INSTALL_FOLDER=${LIBCARLA_INSTALL_CLIENT_FOLDER}
    CMAKE_EXTRA_OPTIONS="${CMAKE_EXTRA_OPTIONS:+${CMAKE_EXTRA_OPTIONS} }-DBUILD_RSS_VARIANT=ON -DADRSS_INSTALL_DIR=${CARLA_BUILD_FOLDER}/ad-rss-4.4.3/install"
  else
    fatal_error "Invalid build configuration \"$1\""
  fi

  if [ $2 == Debug ] ; then
    M_DEBUG=ON
    M_RELEASE=OFF
  elif [ $2 == Release ] ; then
    M_DEBUG=OFF
    M_RELEASE=ON
  else
    fatal_error "Invalid build option \"$2\""
  fi

  log "Building LibCarla \"$1.$2\" configuration."

  mkdir -p ${M_BUILD_FOLDER}
  pushd "${M_BUILD_FOLDER}" >/dev/null

  CHECKSUM_FILE=checksum.txt

  if [ ! -f "${CHECKSUM_FILE}" ] ; then
    NEEDS_CMAKE=true
  elif [ "$(cat ${CHECKSUM_FILE})" != "$(get_source_code_checksum)" ] ; then
    log "Re-running cmake, some files were added or removed."
    NEEDS_CMAKE=true
  else
    NEEDS_CMAKE=false
  fi

  if ${NEEDS_CMAKE} ; then

    cmake \
        -G "Eclipse CDT4 - Ninja" \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE:-$1} \
        -DLIBCARLA_BUILD_DEBUG=${M_DEBUG} \
        -DLIBCARLA_BUILD_RELEASE=${M_RELEASE} \
        -DCMAKE_TOOLCHAIN_FILE=${M_TOOLCHAIN} \
        -DCMAKE_INSTALL_PREFIX=${M_INSTALL_FOLDER} \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        ${CMAKE_EXTRA_OPTIONS} \
        ${CARLA_ROOT_FOLDER}

    get_source_code_checksum > ${CHECKSUM_FILE}

  fi

  ninja

  ninja install | grep -v "Up-to-date:"

  popd >/dev/null
}

# ==============================================================================
# -- Build all possible configurations -----------------------------------------
# ==============================================================================

if { ${BUILD_SERVER} && ${BUILD_OPTION_DEBUG}; }; then

  build_libcarla Server Debug

fi

if { ${BUILD_SERVER} && ${BUILD_OPTION_RELEASE}; }; then

  build_libcarla Server Release
  if ${USE_PYTORCH} ; then
    build_libcarla Pytorch Release
  fi

  if ${USE_ROS2} ; then
    build_libcarla ros2 Release
  fi

fi

CLIENT_VARIANT='Client'
if [ $BUILD_RSS_VARIANT == true ] ; then
  CLIENT_VARIANT='ClientRSS'
fi

if { ${BUILD_CLIENT} && ${BUILD_OPTION_DEBUG}; }; then

  build_libcarla ${CLIENT_VARIANT} Debug

fi

if { ${BUILD_CLIENT} && ${BUILD_OPTION_RELEASE}; }; then

  build_libcarla ${CLIENT_VARIANT} Release

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
