#!/usr/bin/env bash
# Run a Scenic scenario against the running CARLA server.
#
#   bash scripts/run_scenic.sh srunner/scenic/carlaChallenge1.scenic
#   bash scripts/run_scenic.sh carlaChallenge1          # resolved by name
#
# Knobs (env vars):
#   COUNT=1        simulations to run                    (default 1; empty = unbounded)
#   TIME=300       step bound per simulation             (default 300; empty = unbounded)
#   SEED=          random seed, for a reproducible scene
#   MODE2D=1       pass --2d                             (default 1)
#   VERBOSITY=2    scenic -v level                       (default 2)
#   PARAMS="k v"   extra --param pairs, space separated, repeatable via PARAMS2..
#   TIMEOUT=180    client timeout in seconds              (default 180)
#   LOG_DIR=dir    where the run log lands               (default ./scenic-runs)
#   EXTRA="..."    raw extra scenic arguments
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
set +e

TARGET="${1:-}"
[ -n "${TARGET}" ] || { echo "usage: run_scenic.sh <scenario.scenic|name>"; exit 2; }
[ -n "${SCENIC_BIN}" ] || { echo "FAIL no scenic CLI — run check_env.sh"; exit 2; }

# Resolve a bare name against both scenario sources so callers do not have to
# know which tree a scenario lives in.
resolve() {
  local t="$1"
  [ -f "${t}" ] && { echo "${t}"; return; }
  local base="${t%.scenic}.scenic"
  for d in "${SCENARIO_RUNNER_ROOT}/srunner/scenic" "${SCENIC_EXAMPLES}"; do
    [ -n "${d}" ] || continue
    local hit
    hit="$(find "${d}" -name "${base}" -print -quit 2>/dev/null)"
    [ -n "${hit}" ] && { echo "${hit}"; return; }
  done
}
FILE="$(resolve "${TARGET}")"
if [ -z "${FILE}" ]; then
  echo "FAIL no scenario '${TARGET}' under \$SCENARIO_RUNNER_ROOT/srunner/scenic or \$SCENIC_EXAMPLES"
  echo "     list what exists:  python3 ${HERE}/list_scenic.py"
  exit 2
fi

COUNT="${COUNT-1}"; TIME="${TIME-300}"; MODE2D="${MODE2D:-1}"; VERBOSITY="${VERBOSITY:-2}"
# Scenic's own default is `param timeout = 10`, and loading a large map in the
# editor takes minutes. The load still completes after the client gives up, so the
# symptom is a confusing "CARLA could not load world" on a map that then appears.
TIMEOUT="${TIMEOUT:-180}"
LOG_DIR="${LOG_DIR:-./scenic-runs}"; mkdir -p "${LOG_DIR}"
LOG="${LOG_DIR}/$(basename "${FILE}" .scenic)-$(date +%Y%m%d-%H%M%S).log"

args=("${FILE}" --simulate)
[ "${MODE2D}" = "1" ] && args+=(--2d)
[ -n "${COUNT}" ] && args+=(--count "${COUNT}")
[ -n "${TIME}" ]  && args+=(--time "${TIME}")
[ -n "${SEED:-}" ] && args+=(--seed "${SEED}")
args+=(-v "${VERBOSITY}")
args+=(--param timeout "${TIMEOUT}")
for v in "${PARAMS:-}" "${PARAMS2:-}" "${PARAMS3:-}"; do
  # shellcheck disable=SC2086
  [ -n "${v}" ] && args+=(--param ${v})
done
# shellcheck disable=SC2206,SC2086
[ -n "${EXTRA:-}" ] && args+=(${EXTRA})

# An interrupted run leaves the world synchronous with nobody ticking it, which
# makes every other client look frozen. Scenic restores async on a clean exit;
# this covers Ctrl-C and crashes.
restore_async() {
  "${PYTHON}" - "$CARLA_HOST" "$CARLA_PORT" <<'PY' 2>/dev/null
import sys
try:
    import carla
    c = carla.Client(sys.argv[1], int(sys.argv[2])); c.set_timeout(10.0)
    w = c.get_world(); s = w.get_settings()
    if s.synchronous_mode:
        s.synchronous_mode = False; s.fixed_delta_seconds = None
        w.apply_settings(s); print("[run] restored asynchronous mode")
except Exception:
    pass
PY
}
trap restore_async EXIT INT TERM

echo "[run] ${SCENIC_BIN} ${args[*]}"
echo "[run] log -> ${LOG}"
"${SCENIC_BIN}" "${args[@]}" 2>&1 | tee "${LOG}"
rc=${PIPESTATUS[0]}

# A scenic run can exit 0 having simulated nothing (all scenes rejected), so the
# artifact to confirm is a termination line, not the exit code.
echo "== verify =="
if grep -qE "ended successfully at time step" "${LOG}"; then
  grep -oE "Simulation [0-9]+ ended successfully at time step [0-9]+ because: .*" "${LOG}" | sed 's/^/  /'
  echo "  PASS simulation(s) ran"
elif grep -q "tried to make discrete distribution over empty domain" "${LOG}"; then
  echo "  FAIL scene not constructible on this map — a filter matched nothing."
  echo "       python3 ${HERE}/list_scenic.py --check-maps  shows each map's features"
  rc=1
elif grep -q "could not load world" "${LOG}"; then
  echo "  FAIL the map did not load within TIMEOUT=${TIMEOUT}s."
  echo "       An editor loading a large map often exceeds it and finishes anyway —"
  echo "       raise TIMEOUT, or re-run now that the map is loaded."
  rc=1
elif grep -qE "blueprints recorded for CARLA" "${LOG}"; then
  echo "  FAIL Scenic has no blueprints of that type for this client version."
  echo "       check_env.sh lists the empty categories"
  rc=1
elif grep -q "createObjectInSimulator" "${LOG}"; then
  echo "  FAIL a blueprint id does not exist on this server. LibCarla's find()"
  echo "       raises a bare std::exception without naming it, so check every id:"
  echo "       python3 ../create-scenic-scenario/scripts/blueprint_table.py --check <ids>"
  rc=1
else
  echo "  FAIL no simulation completed; last lines:"
  tail -5 "${LOG}" | sed 's/^/    /'
  [ "$rc" -eq 0 ] && rc=1
fi
exit $rc
