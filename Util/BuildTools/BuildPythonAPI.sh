#! /bin/bash

source $(dirname "$0")/Environment.sh

export CC=clang-8
export CXX=clang++-8

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Build and package CARLA Python API."

USAGE_STRING="Usage: $0 [-h|--help] [--rebuild] [--clean] [--python-version=VERSION]"

REMOVE_INTERMEDIATE=false
BUILD_RSS_VARIANT=false
BUILD_PYTHONAPI=true

OPTS=`getopt -o h --long help,rebuild,clean,rss,python-version:,packages:,clean-intermediate,all,xml, -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2 ; fi

eval set -- "$OPTS"

PY_VERSION=3

while [[ $# -gt 0 ]]; do
  case "$1" in
    --rebuild )
      REMOVE_INTERMEDIATE=true;
      BUILD_PYTHONAPI=true;
      shift ;;
    --python-version )
      PY_VERSION="$2"
      shift 2 ;;
    --rss )
      BUILD_RSS_VARIANT=true;
      shift ;;
    --clean )
      REMOVE_INTERMEDIATE=true;
      BUILD_PYTHONAPI=false;
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

if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_PYTHONAPI} ; }; then
  fatal_error "Nothing selected to be done."
fi

pushd "${CARLA_PYTHONAPI_SOURCE_FOLDER}" >/dev/null

# ==============================================================================
# -- Clean intermediate files --------------------------------------------------
# ==============================================================================

if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  rm -Rf build dist carla.egg-info source/carla.egg-info

  find source -name "*.so" -delete
  find source -name "__pycache__" -type d -exec rm -r "{}" \;

fi

# ==============================================================================
# -- Build API -----------------------------------------------------------------
# ==============================================================================

if ${BUILD_RSS_VARIANT} ; then
  export BUILD_RSS_VARIANT=${BUILD_RSS_VARIANT}
fi

if ${BUILD_PYTHONAPI} ; then

  log "Building Python API for Python 3."

  /usr/bin/env python${PY_VERSION} setup.py bdist_egg

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null

log "Success!"
