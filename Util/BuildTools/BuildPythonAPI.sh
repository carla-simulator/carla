#! /bin/bash

source $(dirname "$0")/Environment.sh

export CC=clang-6.0
export CXX=clang++-6.0

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Build and package CARLA Python API."

USAGE_STRING="Usage: $0 [-h|--help] [--rebuild] [--py2] [--py3] [--clean]"

REMOVE_INTERMEDIATE=false
BUILD_FOR_PYTHON2=false
BUILD_FOR_PYTHON3=false

OPTS=`getopt -o h --long help,rebuild,py2,py3,clean -n 'parse-options' -- "$@"`

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

PYTHONS=("cp27-cp27mu"
		 "cp34-cp34m"
		 "cp35-cp35m"
		 "cp36-cp36m"
		 "cp37-cp37m")

# Create .whl if not existing
mkdir -p .whl
# Clear previously built wheels. Alternatively, the git clean ... command inside the for loop below can be enabled to take care of this
if [ "$(ls -A .whl)" ]; then
	rm .whl/*
fi

# Build wheels for each PYTHONS
for ((i=0; i<${#PYTHONS[@]}; ++i)); do
	PYTHON=${PYTHONS[i]}
	# -f : clean untracked files
	# -d : rm dirs
	# -x : ignore .gitignore file
	# -e : except .whl dir
	# git clean -f -x -d -e .whl
	log "Building Python API for: "${PYTHON}
	PATH=/opt/python/${PYTHON}/bin:$PATH /opt/python/${PYTHON}/bin/python setup.py bdist_wheel --universal

done

cp dist/*.whl .whl/

# Rename wheels (replace platform tag wit manylinux) to satisfy PyPI. TODO: Use auditwheel
pushd .whl
	find *.whl -exec bash -c 'mv $1 ${1//linux/manylinux1}' bash {} \;
popd

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null

log "Success!"
