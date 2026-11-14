#!/usr/bin/env bash
# Run one ScenarioRunner Python scenario (or a group) with the environment,
# preflight and cleanup that scenario_runner.py itself does not do.
#
#   bash run_scenario.sh FollowLeadingVehicle_1
#   bash run_scenario.sh group:ControlLoss
#
# Knobs (env):
#   RELOAD=0        do not reload the world (default 1 — required if the map differs)
#   SYNC=1          synchronous mode at FRAME_RATE Hz (default 20)
#   FILE/JSON/JUNIT=1   result to .txt / .json / .xml (stdout is always enabled)
#   OUTPUT_DIR      directory for result files
#   REPETITIONS     repeat count (default 1)
#   RANDOMIZE=1     randomise scenario parameters
#   RECORD=dir      CARLA recorder + criteria json, relative to SCENARIO_RUNNER_ROOT
#   WAIT_FOR_EGO=1  attach to an existing ego instead of spawning one
#   DEBUG=1         print the behaviour tree each tick
#   TIMEOUT         client timeout seconds (default 120)
#   MAX_WALL        wall-clock guard seconds (default 1800, 0 = none)
#   CONFIG_FILE     extra scenario config xml
#   ADDITIONAL      extra scenario implementation .py
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"
# env.sh runs `set -euo pipefail`, and sourcing it applies -e to THIS shell. This
# wrapper must survive a non-zero exit from the tool it runs, or its verification
# and cleanup are skipped precisely when a run failed.
set +e
source "${HERE}/../../_common/runner_result.sh" || exit 1


SCENARIO="${1:-}"
if [ -z "${SCENARIO}" ]; then
  echo "usage: bash run_scenario.sh <CONFIG_NAME|group:TYPE>" >&2
  echo "       python3 ${HERE}/list_scenarios.py    # to see the names" >&2
  exit 2
fi
[ -n "${SCENARIO_RUNNER_ROOT}" ] || { echo "SCENARIO_RUNNER_ROOT is not set — run check_env.sh" >&2; exit 2; }

RELOAD="${RELOAD:-1}"
SYNC="${SYNC:-0}"
FRAME_RATE="${FRAME_RATE:-20}"
REPETITIONS="${REPETITIONS:-1}"
# ScenarioRunner defaults its own client_timeout to 120 s; passing --timeout
# overrides that. 10 s is far too low against an editor, where switching
# between large maps routinely takes minutes and the client gives up while
# the load still completes.
TIMEOUT="${TIMEOUT:-120}"

# --- preflight: does the requested scenario exist, and on which map? ----------
# scenario_runner.py's own failure for a bad name is a bare "not supported ...
# Exiting" after it has already connected and reloaded the world, so check first.
read -r FOUND WANT_TOWN <<<"$("${PYTHON}" - \
  "$(carla_native_path "${SCENARIO_RUNNER_ROOT}")" "${SCENARIO}" <<'PY' | tr -d '\r'
import glob, sys, xml.etree.ElementTree as ET
root, want = sys.argv[1], sys.argv[2]
group = want.split("group:", 1)[1] if want.startswith("group:") else None
towns, found = [], False
for f in glob.glob(f"{root}/srunner/examples/*.xml"):
    try:
        tree = ET.parse(f)
    except ET.ParseError:
        continue
    for s in tree.getroot().iter("scenario"):
        hit = (s.attrib.get("type") == group) if group else (s.attrib.get("name") == want)
        if hit:
            found = True
            towns.append(s.attrib.get("town", "?"))
print(("yes" if found else "no"), (towns[0] if len(set(towns)) == 1 else "multiple") if towns else "-")
PY
)"
if [ "${FOUND}" != "yes" ]; then
  echo "ERROR: '${SCENARIO}' is not a config in ${SCENARIO_RUNNER_ROOT}/srunner/examples/" >&2
  echo "       python3 ${HERE}/list_scenarios.py    # the real list for this branch" >&2
  exit 3
fi
echo "[run] ${SCENARIO} -> town ${WANT_TOWN}"

CUR_TOWN="$("${PYTHON}" - "${CARLA_HOST}" "${CARLA_PORT}" <<'PY' | tr -d '\r'
import sys
try:
    import carla
    c = carla.Client(sys.argv[1], int(sys.argv[2])); c.set_timeout(5.0)
    print(c.get_world().get_map().name.split('/')[-1])
except Exception:
    print("-")
PY
)"
if [ "${CUR_TOWN}" = "-" ]; then
  echo "ERROR: no CARLA server at ${CARLA_HOST}:${CARLA_PORT} — start one (run-carla-server skill)" >&2
  exit 4
fi
echo "[run] server map is ${CUR_TOWN}"
if [ "${RELOAD}" = "0" ] && [ "${WANT_TOWN}" != "multiple" ] && [ "${WANT_TOWN}" != "${CUR_TOWN}" ]; then
  echo "ERROR: RELOAD=0 but the scenario needs ${WANT_TOWN} and the server is on ${CUR_TOWN}." >&2
  echo "       Drop RELOAD=0, or load the map first (load-map skill)." >&2
  exit 5
fi

# --- always hand the world back in async mode --------------------------------
# An interrupted SYNC=1 run leaves synchronous mode on, and a synchronous world
# with no ticking client is indistinguishable from a hung server for every other
# tool. Runs on normal exit, on error and on Ctrl-C.
reset_async() {
  [ -z "${LOG:-}" ] || rm -f "${LOG}"
  [ "${SYNC}" = "1" ] || return 0
  "${PYTHON}" - "${CARLA_HOST}" "${CARLA_PORT}" "${CARLA_TM_PORT}" <<'PY' || true
import sys
try:
    import carla
    c = carla.Client(sys.argv[1], int(sys.argv[2])); c.set_timeout(5.0)
    w = c.get_world(); s = w.get_settings()
    if s.synchronous_mode:
        s.synchronous_mode = False; s.fixed_delta_seconds = None
        w.apply_settings(s)
        c.get_trafficmanager(int(sys.argv[3])).set_synchronous_mode(False)
        print("[run] world reset to asynchronous mode")
except Exception as e:
    print(f"[run] WARNING could not reset async mode: {e}")
PY
}
LOG=""
trap reset_async EXIT INT TERM

ARGS=(--scenario "${SCENARIO}" --output
      --host "${CARLA_HOST}" --port "${CARLA_PORT}"
      --trafficManagerPort "${CARLA_TM_PORT}"
      --timeout "${TIMEOUT}" --repetitions "${REPETITIONS}")
[ "${RELOAD}" = "1" ]            && ARGS+=(--reloadWorld)
[ "${SYNC}" = "1" ]              && ARGS+=(--sync --frameRate "${FRAME_RATE}")
[ "${DEBUG:-0}" = "1" ]          && ARGS+=(--debug)
[ "${RANDOMIZE:-0}" = "1" ]      && ARGS+=(--randomize)
[ "${WAIT_FOR_EGO:-0}" = "1" ]   && ARGS+=(--waitForEgo)
[ "${FILE:-0}" = "1" ]           && ARGS+=(--file)
[ "${JSON:-0}" = "1" ]           && ARGS+=(--json)
[ "${JUNIT:-0}" = "1" ]          && ARGS+=(--junit)
[ -n "${OUTPUT_DIR:-}" ]         && { mkdir -p "${OUTPUT_DIR}"; ARGS+=(--outputDir "${OUTPUT_DIR}"); }
[ -n "${RECORD:-}" ]             && { mkdir -p "${SCENARIO_RUNNER_ROOT}/${RECORD}"; ARGS+=(--record "${RECORD}"); }
[ -n "${CONFIG_FILE:-}" ]        && ARGS+=(--configFile "${CONFIG_FILE}")
[ -n "${ADDITIONAL:-}" ]         && ARGS+=(--additionalScenario "${ADDITIONAL}")

echo "[run] ${PYTHON} scenario_runner.py ${ARGS[*]}"
echo "[run] NOTE most scenarios need something to drive the ego:"
echo "[run]       ${PYTHON} ${SCENARIO_RUNNER_ROOT}/manual_control.py"
cd "${SCENARIO_RUNNER_ROOT}" || exit 1
# -u: Python block-buffers stdout when it is not a tty, so redirecting a long
# run to a log otherwise shows nothing until it finishes.
LOG="$(mktemp)" || exit 1
# scenario_runner can print its verdict and then never exit — the unsupported-name
# path destroys the ego and hangs. Without a wall-clock guard that hangs the caller
# too, and the verification below never gets to run. 30 min is far beyond any
# single scenario; raise MAX_WALL for a long --repetitions sweep, 0 disables it.
MAX_WALL="${MAX_WALL:-1800}"
if [ "${MAX_WALL}" != "0" ]; then
  timeout --foreground -k 10 "${MAX_WALL}" "${PYTHON}" -u scenario_runner.py "${ARGS[@]}" 2>&1 | tee "${LOG}"
  RC=${PIPESTATUS[0]}
  [ "${RC}" -eq 124 ] && echo "[run] wall-clock guard fired after ${MAX_WALL}s"
else
  "${PYTHON}" -u scenario_runner.py "${ARGS[@]}" 2>&1 | tee "${LOG}"
  RC=${PIPESTATUS[0]}
fi
echo "[run] scenario_runner exited ${RC}"

carla_sr_result "${RC}" "${LOG}"
RC=$?
rm -f "${LOG}"
exit ${RC}
