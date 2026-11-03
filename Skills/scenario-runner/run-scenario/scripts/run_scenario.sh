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
#   OUTPUT/FILE/JSON/JUNIT=1   result to stdout / .txt / .json / .xml
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
read -r FOUND WANT_TOWN <<<"$("${PYTHON}" - "${SCENARIO_RUNNER_ROOT}" "${SCENARIO}" <<'PY'
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

CUR_TOWN="$("${PYTHON}" - "${CARLA_HOST}" "${CARLA_PORT}" <<'PY'
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
trap reset_async EXIT INT TERM

ARGS=(--scenario "${SCENARIO}"
      --host "${CARLA_HOST}" --port "${CARLA_PORT}"
      --trafficManagerPort "${CARLA_TM_PORT}"
      --timeout "${TIMEOUT}" --repetitions "${REPETITIONS}")
[ "${RELOAD}" = "1" ]            && ARGS+=(--reloadWorld)
[ "${SYNC}" = "1" ]              && ARGS+=(--sync --frameRate "${FRAME_RATE}")
[ "${DEBUG:-0}" = "1" ]          && ARGS+=(--debug)
[ "${RANDOMIZE:-0}" = "1" ]      && ARGS+=(--randomize)
[ "${WAIT_FOR_EGO:-0}" = "1" ]   && ARGS+=(--waitForEgo)
[ "${OUTPUT:-0}" = "1" ]         && ARGS+=(--output)
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
cd "${SCENARIO_RUNNER_ROOT}"
# -u: Python block-buffers stdout when it is not a tty, so redirecting a long
# run to a log otherwise shows nothing until it finishes.
LOG="$(mktemp)"
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

# scenario_runner.py catches its own exceptions and still returns 0, so the exit
# code alone reports success for a run that never started. Confirm the artifact:
# a scenario that actually ran prints a criteria table.
if grep -q "Criteria Information" "${LOG}"; then
  echo "[run] PASS a criteria report was produced"
  grep -qE "GLOBAL RESULT.*(FAILURE|TIMEOUT)" "${LOG}" \
    && echo "[run] NOTE the scenario ran but did not pass — most need a driver for the ego"
elif grep -qE "not supported .*Exiting|no scenario with name" "${LOG}"; then
  # SR prints the scenario TYPE here, not the config name. The config can be
  # present in the XML while the class that implements it was deleted.
  echo "[run] FAIL ScenarioRunner has no implementation for that scenario type on"
  echo "[run]      this branch, or the config name is wrong. A config listed in the"
  echo "[run]      XML with no class behind it cannot run."
  echo "[run]      python3 ${HERE}/list_scenarios.py --types   shows what resolves"
  RC=1
elif grep -q "load_world" "${LOG}" && grep -q "std::exception" "${LOG}"; then
  echo "[run] FAIL the map did not load in time. Loading one large map from another"
  echo "[run]      in the editor can exceed the client timeout; raise TIMEOUT, or"
  echo "[run]      load the town first and re-run with RELOAD=0."
  RC=1
else
  echo "[run] FAIL no criteria report — the scenario did not complete. Last lines:"
  tail -5 "${LOG}" | sed 's/^/[run]        /'
  [ "${RC}" -eq 0 ] && RC=1
fi
rm -f "${LOG}"
exit ${RC}
