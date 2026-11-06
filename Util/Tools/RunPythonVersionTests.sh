#! /bin/bash

set -o pipefail

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
CARLA_ROOT=$(cd "${SCRIPT_DIR}/../.." && pwd)
PYTHON_API_DIR=${CARLA_ROOT}/PythonAPI

WHEEL_DIR=${CARLA_ROOT}/Build/Release/PythonAPI/dist
RESULTS_DIR=${CARLA_ROOT}/Build/test-results/python-versions
STAGES=offline
PORT=2000
TM_PORT=8000
EXAMPLE_TIMEOUT=45

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Test the Python API once per Python interpreter given. Each version gets a
fresh virtual environment with the wheel built for it and the carla, test and
examples requirements, then runs the selected stages:

  offline   import carla and the agents with warnings as errors, compile every
            example, agent and util script with warnings as errors, unit tests.
  smoke     Util/Tools/RunSmokeTests.sh, against a server on port 3654.
  examples  a set of PythonAPI/examples run headless for a few seconds each,
            against a server on --port. Fails on a traceback or an early exit.

Logs go to RESULTS_DIR/cpXY/. Fails if any stage fails for any version."

USAGE_STRING="Usage: $0 [-h|--help] [--stages offline,smoke,examples] [--wheel-dir DIR] [--results-dir DIR] [--port PORT] [--tm-port PORT] [--example-timeout SECONDS] PYTHON [PYTHON...]"

OPTS=`getopt -o h --long help,stages:,wheel-dir:,results-dir:,port:,tm-port:,example-timeout: -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --stages )
      STAGES="$2";
      shift 2 ;;
    --wheel-dir )
      WHEEL_DIR="$2";
      shift 2 ;;
    --results-dir )
      RESULTS_DIR="$2";
      shift 2 ;;
    --port )
      PORT="$2";
      shift 2 ;;
    --tm-port )
      TM_PORT="$2";
      shift 2 ;;
    --example-timeout )
      EXAMPLE_TIMEOUT="$2";
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

for STAGE in ${STAGES//,/ } ; do
  case "${STAGE}" in
    offline | smoke | examples ) ;;
    * )
      echo "Unknown stage \"${STAGE}\", expected offline, smoke or examples."
      exit 1 ;;
  esac
done

WHEEL_DIR=$(realpath "${WHEEL_DIR}")
mkdir -p "${RESULTS_DIR}"
RESULTS_DIR=$(realpath "${RESULTS_DIR}")

has_stage () {
  [[ ",${STAGES}," == *",$1,"* ]]
}

# ==============================================================================
# -- Stages --------------------------------------------------------------------
# ==============================================================================

# The agents are not part of the wheel; the examples put PythonAPI/carla on
# sys.path to find them, the import check does the same.
IMPORT_CHECK="
import carla
import agents.navigation.basic_agent
import agents.navigation.behavior_agent
import agents.navigation.constant_velocity_agent
import agents.navigation.global_route_planner
print('carla', carla.__file__)
"

run_offline () {
  local VENV_PYTHON=$1
  local OUT_DIR=$2

  PYTHONPATH="${PYTHON_API_DIR}/carla" "${VENV_PYTHON}" -W error -c "${IMPORT_CHECK}" || return 1

  "${VENV_PYTHON}" -W error -m compileall -q -f \
    -x '/nvidia/cosmos/server/' \
    "${PYTHON_API_DIR}/examples" "${PYTHON_API_DIR}/carla/agents" "${PYTHON_API_DIR}/util" || return 1

  # Own config: the directory's unittest.cfg writes a JUnit XML into the source tree.
  printf '[unittest]\n' > "${OUT_DIR}/unittest.cfg"
  (cd "${PYTHON_API_DIR}/test/unit" && "${VENV_PYTHON}" -m nose2 -v -c "${OUT_DIR}/unittest.cfg")
}

run_smoke () {
  local VENV_PYTHON=$1
  local OUT_DIR=$2

  "${SCRIPT_DIR}/RunSmokeTests.sh" --wheel-dir "${WHEEL_DIR}" --results-dir "${OUT_DIR}" "${VENV_PYTHON}"
}

# Each entry: script|arguments. Every one loops until interrupted, so being
# stopped by SIGINT at EXAMPLE_TIMEOUT is the expected way out.
EXAMPLES=(
  "generate_traffic.py|--port ${PORT} -n 30 -w 10 --safe --seed 1"
  "automatic_control.py|--port ${PORT} --sync --res 640x360 -l"
  "manual_control.py|--port ${PORT} --autopilot --res 640x360"
  "no_rendering_mode.py|--port ${PORT} --res 800x600"
  "sensor_synchronization.py|"
  "visualize_multiple_sensors.py|--port ${PORT} --res 1280x720"
  "dynamic_day_night_weather_cycle.py|--port ${PORT} --speed 50"
  "lane_explorer.py|--port ${PORT} -t 0.05"
)

run_examples () {
  local VENV_PYTHON=$1
  local OUT_DIR=$2
  local FAILED_EXAMPLES=()

  for ENTRY in "${EXAMPLES[@]}" ; do
    local SCRIPT=${ENTRY%%|*}
    local ARGS=(${ENTRY#*|})
    local LOG=${OUT_DIR}/${SCRIPT%.py}.log
    if grep -q "'--tm-port'" "${PYTHON_API_DIR}/examples/${SCRIPT}" ; then
      ARGS+=(--tm-port "${TM_PORT}")
    fi

    echo "Running ${SCRIPT} ${ARGS[*]}"
    (cd "${PYTHON_API_DIR}/examples" &&
      SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy timeout -s INT -k 30 "${EXAMPLE_TIMEOUT}" \
        "${VENV_PYTHON}" "${SCRIPT}" "${ARGS[@]}") > "${LOG}" 2>&1
    local RESULT=$?

    # 124: stopped by the time limit, as expected. 139: the client can segfault
    # at interpreter exit after the work is done, which is not counted here.
    if grep -q '^Traceback' "${LOG}" && ! grep -q '^KeyboardInterrupt' "${LOG}" ; then
      echo "  ${SCRIPT} raised:"
      tail -n 20 "${LOG}" | sed 's/^/    /'
      FAILED_EXAMPLES+=("${SCRIPT}")
    elif [[ ${RESULT} -ne 0 && ${RESULT} -ne 124 && ${RESULT} -ne 139 ]] ; then
      echo "  ${SCRIPT} exited with ${RESULT} before the time limit:"
      tail -n 20 "${LOG}" | sed 's/^/    /'
      FAILED_EXAMPLES+=("${SCRIPT}")
    fi
  done

  if [[ ${#FAILED_EXAMPLES[@]} -ne 0 ]] ; then
    echo "Examples failed: ${FAILED_EXAMPLES[*]}"
    return 1
  fi
}

# ==============================================================================
# -- Run the tests -------------------------------------------------------------
# ==============================================================================

FAILED=()
SUMMARY=()

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

  OUT_DIR=${RESULTS_DIR}/${TAG}
  VENV=${RESULTS_DIR}/venv-${TAG}
  rm -rf "${OUT_DIR}" "${VENV}"
  mkdir -p "${OUT_DIR}"
  # Keep the bytecode of every stage out of the source tree.
  export PYTHONPYCACHEPREFIX=${OUT_DIR}/pycache

  echo "Setting up ${TAG} (${PYTHON}) with $(basename "${WHEELS[0]}")."
  if ! { "${PYTHON}" -m venv "${VENV}" &&
         "${VENV}/bin/python" -m pip install --quiet --upgrade pip &&
         "${VENV}/bin/python" -m pip install --quiet "${WHEELS[0]}" \
           -r "${PYTHON_API_DIR}/carla/requirements.txt" \
           -r "${PYTHON_API_DIR}/test/requirements.txt" \
           -r "${PYTHON_API_DIR}/examples/requirements.txt" ; } > "${OUT_DIR}/setup.log" 2>&1 ; then
    tail -n 20 "${OUT_DIR}/setup.log"
    FAILED+=("${TAG}")
    SUMMARY+=("${TAG} setup FAIL")
    continue
  fi

  for STAGE in offline smoke examples ; do
    has_stage ${STAGE} || continue
    echo "Running the ${STAGE} stage for ${TAG}."
    run_${STAGE} "${VENV}/bin/python" "${OUT_DIR}" 2>&1 | tee "${OUT_DIR}/${STAGE}.log"
    if [[ ${PIPESTATUS[0]} -eq 0 ]] ; then
      SUMMARY+=("${TAG} ${STAGE} PASS")
    else
      SUMMARY+=("${TAG} ${STAGE} FAIL")
      [[ " ${FAILED[*]} " == *" ${TAG} "* ]] || FAILED+=("${TAG}")
    fi
  done
done

printf '%s\n' "${SUMMARY[@]}" | column -t | tee "${RESULTS_DIR}/summary.txt"

if [[ ${#FAILED[@]} -ne 0 ]] ; then
  echo "Python version tests failed for: ${FAILED[*]}"
  exit 1
fi

echo "Python version tests passed for every Python version."
