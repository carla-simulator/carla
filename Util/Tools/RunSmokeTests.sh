#! /bin/bash

set -o pipefail

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
CARLA_ROOT=$(cd "${SCRIPT_DIR}/../.." && pwd)

WHEEL_DIR=${CARLA_ROOT}/Build/Release/PythonAPI/dist
RESULTS_DIR=${CARLA_ROOT}/Build/test-results
PORT=3654
CONNECT_TIMEOUT=60

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Run the smoke tests (PythonAPI/test/smoke_test_list.txt) once per Python
interpreter given, each with the wheel built for its version, against a CARLA
server already listening on port ${PORT}. Writes the JUnit results to
RESULTS_DIR/smoke-cpXY.xml and fails if any version fails."

USAGE_STRING="Usage: $0 [-h|--help] [--wheel-dir DIR] [--results-dir DIR] [--connect-timeout SECONDS] PYTHON [PYTHON...]"

OPTS=`getopt -o h --long help,wheel-dir:,results-dir:,connect-timeout: -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --wheel-dir )
      WHEEL_DIR="$2";
      shift 2 ;;
    --results-dir )
      RESULTS_DIR="$2";
      shift 2 ;;
    --connect-timeout )
      CONNECT_TIMEOUT="$2";
      shift 2 ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    -- )
      shift
      break ;;
    * )
      shift ;;
  esac
done

if [[ $# -eq 0 ]]; then
  echo "$USAGE_STRING"
  exit 1
fi

WHEEL_DIR=$(realpath "${WHEEL_DIR}")
mkdir -p "${RESULTS_DIR}"
RESULTS_DIR=$(realpath "${RESULTS_DIR}")

# ==============================================================================
# -- Run the smoke tests -------------------------------------------------------
# ==============================================================================

cd "${CARLA_ROOT}/PythonAPI/test"

SMOKE_TESTS=$(cat smoke_test_list.txt)
FAILED=()

for PYTHON in "$@" ; do
  if ! TAG=$("${PYTHON}" -c 'import sys; print("cp%d%d" % sys.version_info[:2])') ; then
    echo "Cannot run ${PYTHON}."
    FAILED+=("${PYTHON}")
    continue
  fi

  WHEELS=(${WHEEL_DIR}/carla-*-${TAG}-${TAG}-*.whl)
  if [[ ${#WHEELS[@]} -ne 1 || ! -f "${WHEELS[0]}" ]] ; then
    echo "Expected exactly one ${TAG} wheel in ${WHEEL_DIR}, found: ${WHEELS[*]}"
    FAILED+=("${TAG}")
    continue
  fi

  echo "Running smoke tests for ${TAG} (${PYTHON}) with $(basename "${WHEELS[0]}")."

  rm -f test-results.xml
  "${PYTHON}" -m pip install --quiet --force-reinstall --no-deps "${WHEELS[0]}" &&
    "${PYTHON}" -m pip install --quiet -r ../carla/requirements.txt -r requirements.txt &&
    "${PYTHON}" test_connection.py -p ${PORT} --timeout=${CONNECT_TIMEOUT} &&
    "${PYTHON}" -m nose2 -v -c smoke/unittest.cfg -X ${SMOKE_TESTS}
  RESULT=$?

  if [[ -f test-results.xml ]] ; then
    mv test-results.xml "${RESULTS_DIR}/smoke-${TAG}.xml"
  fi
  if [[ ${RESULT} -ne 0 ]] ; then
    FAILED+=("${TAG}")
  fi
done

if [[ ${#FAILED[@]} -ne 0 ]] ; then
  echo "Smoke tests failed for: ${FAILED[*]}"
  exit 1
fi

echo "Smoke tests passed for every Python version."
