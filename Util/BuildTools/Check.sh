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

You can also set the command-line arguments passed to GTest on a ".gtest"
config file in the Carla project main folder. E.g.

    # Contents of ${CARLA_ROOT_FOLDER}/.gtest
    gtest_shuffle
    gtest_filter=misc*
END
)

GDB=
XML_OUTPUT=false
GTEST_ARGS=`sed -e 's/#.*$//g' ${CARLA_ROOT_FOLDER}/.gtest | sed -e '/^[[:space:]]*$/!s/^/--/g' | sed -e ':a;N;$!ba;s/\n/ /g'`
LIBCARLA_RELEASE=false
LIBCARLA_DEBUG=false
PYTHON_API_2=false
PYTHON_API_3=false
SMOKE_TESTS_2=false
SMOKE_TESTS_3=false
RUN_BENCHMARK=false

OPTS=`getopt -o h --long help,gdb,xml,gtest_args:,all,libcarla-release,libcarla-debug,python-api-2,python-api-3,smoke-2,smoke-3,benchmark -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2 ; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --gdb )
      GDB="gdb --args";
      shift ;;
    --xml )
      XML_OUTPUT=true;
      # Create the folder for the test-results
      mkdir -p "${CARLA_TEST_RESULTS_FOLDER}"
      shift ;;
    --gtest_args )
      GTEST_ARGS="$2";
      shift 2 ;;
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
    --smoke-2 )
      SMOKE_TESTS_2=true;
      shift ;;
    --smoke-3 )
      SMOKE_TESTS_3=true;
      shift ;;
    --benchmark )
      LIBCARLA_RELEASE=true;
      RUN_BENCHMARK=true;
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

if ! { ${LIBCARLA_RELEASE} || ${LIBCARLA_DEBUG} || ${PYTHON_API_2} || ${PYTHON_API_3} || ${SMOKE_TESTS_2} || ${SMOKE_TESTS_3}; }; then
  fatal_error "Nothing selected to be done."
fi

# ==============================================================================
# -- Download Content need it by the tests -------------------------------------
# ==============================================================================

if { ${LIBCARLA_RELEASE} || ${LIBCARLA_DEBUG}; }; then

  CONTENT_TAG=0.1.4

  mkdir -p ${LIBCARLA_TEST_CONTENT_FOLDER}
  pushd "${LIBCARLA_TEST_CONTENT_FOLDER}" >/dev/null

  if [ "$(get_git_repository_version)" != "${CONTENT_TAG}" ]; then
    pushd .. >/dev/null
    rm -Rf ${LIBCARLA_TEST_CONTENT_FOLDER}
    git clone -b ${CONTENT_TAG} https://github.com/carla-simulator/opendrive-test-files.git ${LIBCARLA_TEST_CONTENT_FOLDER}
    popd >/dev/null
  fi

  popd >/dev/null

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

  log "Running LibCarla.server unit tests (debug)."
  echo "Running: ${GDB} libcarla_test_server_debug ${GTEST_ARGS} ${EXTRA_ARGS}"
  LD_LIBRARY_PATH=${LIBCARLA_INSTALL_SERVER_FOLDER}/lib ${GDB} ${LIBCARLA_INSTALL_SERVER_FOLDER}/test/libcarla_test_server_debug ${GTEST_ARGS} ${EXTRA_ARGS}

  log "Running LibCarla.client unit tests (debug)."
  echo "Running: ${GDB} libcarla_test_client_debug ${GTEST_ARGS} ${EXTRA_ARGS}"
  ${GDB} ${LIBCARLA_INSTALL_CLIENT_FOLDER}/test/libcarla_test_client_debug ${GTEST_ARGS} ${EXTRA_ARGS}

fi

if ${LIBCARLA_RELEASE} ; then

  if ${XML_OUTPUT} ; then
    EXTRA_ARGS="--gtest_output=xml:${CARLA_TEST_RESULTS_FOLDER}/libcarla-release.xml"
  else
    EXTRA_ARGS=
  fi

  log "Running LibCarla.server unit tests (release)."
  echo "Running: ${GDB} libcarla_test_server_release ${GTEST_ARGS} ${EXTRA_ARGS}"
  LD_LIBRARY_PATH=${LIBCARLA_INSTALL_SERVER_FOLDER}/lib ${GDB} ${LIBCARLA_INSTALL_SERVER_FOLDER}/test/libcarla_test_server_release ${GTEST_ARGS} ${EXTRA_ARGS}

  if ! { ${RUN_BENCHMARK} ; }; then

    log "Running LibCarla.client unit tests (release)."
    echo "Running: ${GDB} libcarla_test_client_debug ${GTEST_ARGS} ${EXTRA_ARGS}"
    ${GDB} ${LIBCARLA_INSTALL_CLIENT_FOLDER}/test/libcarla_test_client_release ${GTEST_ARGS} ${EXTRA_ARGS}

  fi

fi

# ==============================================================================
# -- Run Python API unit tests -------------------------------------------------
# ==============================================================================

pushd "${CARLA_PYTHONAPI_ROOT_FOLDER}/test/unit" >/dev/null

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
# -- Run smoke tests -----------------------------------------------------------
# ==============================================================================

if ${SMOKE_TESTS_2} || ${SMOKE_TESTS_3} ; then
  pushd "${CARLA_PYTHONAPI_ROOT_FOLDER}/util" >/dev/null
    log "Checking connection with the simulator."
    ./test_connection.py -p 3654 --timeout=60.0
  popd >/dev/null
fi

pushd "${CARLA_PYTHONAPI_ROOT_FOLDER}/test/smoke" >/dev/null

if ${XML_OUTPUT} ; then
  EXTRA_ARGS="-X"
else
  EXTRA_ARGS=
fi

if ${SMOKE_TESTS_2} ; then

  log "Running smoke tests for Python 2."

  /usr/bin/env python2 -m nose2 ${EXTRA_ARGS}

  if ${XML_OUTPUT} ; then
    mv test-results.xml ${CARLA_TEST_RESULTS_FOLDER}/smoke-tests-2.xml
  fi

fi

if ${SMOKE_TESTS_3} ; then

  log "Running smoke tests for Python 3."

  /usr/bin/env python3 -m nose2 ${EXTRA_ARGS}

  if ${XML_OUTPUT} ; then
    mv test-results.xml ${CARLA_TEST_RESULTS_FOLDER}/smoke-tests-3.xml
  fi

fi

popd >/dev/null

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
