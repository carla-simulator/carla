#! /bin/bash

source $(dirname "$0")/Environment.sh

export CC=clang-7
export CXX=clang++-7

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Build and package CARLA Python API."

USAGE_STRING="Usage: $0 [-h|--help] [--rebuild] [--py2] [--py3] [--clean]"

REMOVE_INTERMEDIATE=false
BUILD_FOR_PYTHON2=false
BUILD_FOR_PYTHON3=false
BUILD_RSS_VARIANT=false

OPTS=`getopt -o h --long help,rebuild,py2,py3,clean,rss -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2 ; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --rebuild )
      REMOVE_INTERMEDIATE=true;
      BUILD_FOR_PYTHON2=true;
      BUILD_FOR_PYTHON3=true;
      shift ;;
    --py2 )
      BUILD_FOR_PYTHON2=true;
      shift ;;
    --py3 )
      BUILD_FOR_PYTHON3=true;
      shift ;;
    --rss )
      BUILD_RSS_VARIANT=true;
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

if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_FOR_PYTHON2} || ${BUILD_FOR_PYTHON3}; }; then
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

if ${BUILD_FOR_PYTHON2} ; then

  log "Building Python API for Python 2."

  /usr/bin/env python2 setup.py bdist_egg

fi

if ${BUILD_FOR_PYTHON3} ; then

  log "Building Python API for Python 3."

  /usr/bin/env python3 setup.py bdist_egg

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null

log "Success!"
