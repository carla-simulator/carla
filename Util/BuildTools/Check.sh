#! /bin/bash

source $(dirname "$0")/Environment.sh

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Run unit tests."

USAGE_STRING=$(cat <<- END
Usage: $0 [-h|--help] [--gdb] [--xml] [--gtest_args=ARGS]

Then either ran all the tests

    [--all]

Or choose one or more of the following

    [--libcarla-release] [--libcarla-debug]
    [--python-api-2] [--python-api-3]
    [--benchmark]
END
)

GDB=
XML_OUTPUT=false
GTEST_ARGS=
LIBCARLA_RELEASE=false
LIBCARLA_DEBUG=false
PYTHON_API_2=false
PYTHON_API_3=false

OPTS=`getopt -o h --long help,gdb,xml,gtest_args:,all,libcarla-release,libcarla-debug,python-api-2,python-api-3,benchmark -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2 ; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --gdb )
      GDB="gdb --args";
      shift ;;
    --xml )
      XML_OUTPUT=true;
      shift ;;
    --gtest_args )
      GTEST_ARGS="$2";
      shift ;;
    --all )
      LIBCARLA_RELEASE=true;
      LIBCARLA_DEBUG=true;
      PYTHON_API_2=true;
      PYTHON_API_3=true;
      shift ;;
    --libcarla-release )
      LIBCARLA_RELEASE=true;
      shift ;;
    --libcarla-debug )
      LIBCARLA_DEBUG=true;
      shift ;;
    --python-api-2 )
      PYTHON_API_2=true;
      shift ;;
    --python-api-3 )
      PYTHON_API_3=true;
      shift ;;
    --benchmark )
      LIBCARLA_RELEASE=true;
      GTEST_ARGS="--gtest_filter=benchmark*";
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo -e "$USAGE_STRING"
      exit 1
      ;;
    * )
      break ;;
  esac
done

if ! { ${LIBCARLA_RELEASE} || ${LIBCARLA_DEBUG} || ${PYTHON_API_2} || ${PYTHON_API_3}; }; then
  fatal_error "Nothing selected to be done."
fi

# ==============================================================================
# -- Run LibCarla tests --------------------------------------------------------
# ==============================================================================

if ${LIBCARLA_DEBUG} ; then

  if ${XML_OUTPUT} ; then
    EXTRA_ARGS="--gtest_output=xml:${CARLA_TEST_RESULTS_FOLDER}/libcarla-debug.xml"
  else
    EXTRA_ARGS=
  fi

  log "Running LibCarla unit tests debug."

  LD_LIBRARY_PATH=${LIBCARLA_INSTALL_SERVER_FOLDER}/lib ${GDB} ${LIBCARLA_INSTALL_SERVER_FOLDER}/test/libcarla_test_debug ${GTEST_ARGS} ${EXTRA_ARGS}

fi

if ${LIBCARLA_RELEASE} ; then

  if ${XML_OUTPUT} ; then
    EXTRA_ARGS="--gtest_output=xml:${CARLA_TEST_RESULTS_FOLDER}/libcarla-release.xml"
  else
    EXTRA_ARGS=
  fi

  log "Running LibCarla unit tests release."

  LD_LIBRARY_PATH=${LIBCARLA_INSTALL_SERVER_FOLDER}/lib ${GDB} ${LIBCARLA_INSTALL_SERVER_FOLDER}/test/libcarla_test_release ${GTEST_ARGS} ${EXTRA_ARGS}

fi

# ==============================================================================
# -- Run Python API tests ------------------------------------------------------
# ==============================================================================

pushd "${CARLA_PYTHONAPI_ROOT_FOLDER}/test" >/dev/null

if ${XML_OUTPUT} ; then
  EXTRA_ARGS="-X"
else
  EXTRA_ARGS=
fi

if ${PYTHON_API_2} ; then

  log "Running Python API for Python 2 unit tests."

  /usr/bin/env python2 -m nose2 ${EXTRA_ARGS}

  if ${XML_OUTPUT} ; then
    mv test-results.xml ${CARLA_TEST_RESULTS_FOLDER}/python-api-2.xml
  fi

fi

if ${PYTHON_API_3} ; then

  log "Running Python API for Python 3 unit tests."

  /usr/bin/env python3 -m nose2 ${EXTRA_ARGS}

  if ${XML_OUTPUT} ; then
    mv test-results.xml ${CARLA_TEST_RESULTS_FOLDER}/python-api-3.xml
  fi

fi

popd >/dev/null

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
