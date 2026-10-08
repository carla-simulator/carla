#!/usr/bin/env bash
# Run the CARLA Leaderboard evaluator with a preflight and a guaranteed async reset.
#
#   TEAM_AGENT=~/team_code/my_agent.py bash run_leaderboard.sh --routes-subset 0
#   bash run_leaderboard.sh --routes .../routes_validation.xml --repetitions 3
#   bash run_leaderboard.sh --resume 1
#
# Everything after the script name is passed through to leaderboard_evaluator.py,
# and overrides the env defaults below.
#
# Env: TEAM_AGENT (required), TEAM_CONFIG, ROUTES, ROUTES_SUBSET, REPETITIONS,
#      CHALLENGE_TRACK_CODENAME, CHECKPOINT_ENDPOINT, DEBUG_CHECKPOINT_ENDPOINT,
#      DEBUG_CHALLENGE, RECORD_PATH, RESUME, TIMEOUT, SKIP_PREFLIGHT=1
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"
# env.sh runs `set -euo pipefail`, and sourcing it applies -e to THIS shell. This
# wrapper must survive a non-zero exit from the tool it runs, or its verification
# and cleanup are skipped precisely when a run failed.
set +e


[ -n "${LEADERBOARD_ROOT}" ] || { echo "LEADERBOARD_ROOT is not set — run check_env.sh" >&2; exit 2; }
: "${TEAM_AGENT:=${LEADERBOARD_ROOT}/leaderboard/autoagents/npc_agent.py}"
[ -f "${TEAM_AGENT}" ] || { echo "ERROR: TEAM_AGENT=${TEAM_AGENT} does not exist" >&2; exit 3; }

ROUTES="${ROUTES:-${LEADERBOARD_ROOT}/data/routes_devtest.xml}"
ROUTES_SUBSET="${ROUTES_SUBSET:-}"
REPETITIONS="${REPETITIONS:-1}"
CHECKPOINT_ENDPOINT="${CHECKPOINT_ENDPOINT:-${PWD}/results.json}"
# The repo's own run_leaderboard.sh passes --debug-checkpoint=$DEBUG_CHECKPOINT_ENDPOINT
# without ever exporting it, so live results land nowhere. Default it beside the
# checkpoint instead.
DEBUG_CHECKPOINT_ENDPOINT="${DEBUG_CHECKPOINT_ENDPOINT:-${CHECKPOINT_ENDPOINT%.json}_live.txt}"
DEBUG_CHALLENGE="${DEBUG_CHALLENGE:-0}"
TIMEOUT="${TIMEOUT:-300}"

if [ "${SKIP_PREFLIGHT:-0}" != "1" ]; then
  echo "=== preflight ==="
  if ! bash "${HERE}/check_env.sh"; then
    echo "ERROR: preflight found hard blockers. Fix them, or set SKIP_PREFLIGHT=1 to run anyway." >&2
    exit 4
  fi
fi

# The evaluator derives the agent class from the file name and fails with a bare
# AttributeError otherwise — after it has already loaded a world.
EXPECT="$("${PYTHON}" -c "import os,sys;print(os.path.basename(sys.argv[1]).split('.')[0].title().replace('_',''))" "${TEAM_AGENT}")"
if ! grep -qE "^class[[:space:]]+${EXPECT}\b" "${TEAM_AGENT}"; then
  echo "ERROR: ${TEAM_AGENT} must define 'class ${EXPECT}' (derived from the file name)" >&2
  grep -nE '^class [A-Za-z_]+' "${TEAM_AGENT}" | sed 's/^/       found: /' >&2
  exit 5
fi

mkdir -p "$(dirname "${CHECKPOINT_ENDPOINT}")"

ARGS=(--routes "${ROUTES}"
      --repetitions "${REPETITIONS}"
      --track "${CHALLENGE_TRACK_CODENAME}"
      --agent "${TEAM_AGENT}"
      --checkpoint "${CHECKPOINT_ENDPOINT}"
      --debug-checkpoint "${DEBUG_CHECKPOINT_ENDPOINT}"
      --debug "${DEBUG_CHALLENGE}"
      --timeout "${TIMEOUT}"
      --host "${CARLA_HOST}" --port "${CARLA_PORT}"
      --traffic-manager-port "${CARLA_TM_PORT}")
[ -n "${ROUTES_SUBSET}" ]     && ARGS+=(--routes-subset "${ROUTES_SUBSET}")
[ -n "${TEAM_CONFIG:-}" ]     && ARGS+=(--agent-config "${TEAM_CONFIG}")
[ -n "${RECORD_PATH:-}" ]     && { mkdir -p "${RECORD_PATH}"; ARGS+=(--record "${RECORD_PATH}"); }
# --resume is argparse type=bool: ANY non-empty string is True, so only add it when
# the caller actually wants to resume.
case "${RESUME:-}" in 1|true|True|yes) ARGS+=(--resume 1) ;; esac

# LB 1.0 needs the separate scenario annotations file.
LBV="$(carla_lb_version)"
if [ "${LBV}" = "1.0" ]; then
  SCENARIOS="${SCENARIOS:-${LEADERBOARD_ROOT}/data/all_towns_traffic_scenarios_public.json}"
  [ -f "${SCENARIOS}" ] || { echo "ERROR: LB 1.0 needs --scenarios; ${SCENARIOS} not found" >&2; exit 6; }
  ARGS+=(--scenarios "${SCENARIOS}")
fi

# The evaluator resets the world to async itself, but only when the run did not
# time out (`not self._client_timed_out`). A killed or timed-out run leaves
# synchronous mode on and the world reads as hung to everything else.
reset_async() {
  "${PYTHON}" - "${CARLA_HOST}" "${CARLA_PORT}" "${CARLA_TM_PORT}" <<'PY' || true
import sys
try:
    import carla
    c = carla.Client(sys.argv[1], int(sys.argv[2])); c.set_timeout(5.0)
    w = c.get_world(); s = w.get_settings()
    if s.synchronous_mode:
        s.synchronous_mode = False; s.fixed_delta_seconds = None
        s.deterministic_ragdolls = False
        w.apply_settings(s)
        c.get_trafficmanager(int(sys.argv[3])).set_synchronous_mode(False)
        print("[lb] world reset to asynchronous mode")
except Exception as e:
    print(f"[lb] WARNING could not reset async mode: {e}")
PY
}
trap reset_async EXIT INT TERM

echo "=== running leaderboard ${LBV} ==="
echo "[lb] agent      ${TEAM_AGENT}  (class ${EXPECT})"
echo "[lb] routes     ${ROUTES}${ROUTES_SUBSET:+  subset ${ROUTES_SUBSET}}  x${REPETITIONS}"
echo "[lb] track      ${CHALLENGE_TRACK_CODENAME}"
echo "[lb] checkpoint ${CHECKPOINT_ENDPOINT}"
echo "[lb] live       ${DEBUG_CHECKPOINT_ENDPOINT} (written when --debug >= 2)"
NROUTES="$("${PYTHON}" - "${ROUTES}" <<'PY'
import sys, xml.etree.ElementTree as ET
try:
    print(len(ET.parse(sys.argv[1]).getroot().findall("route")))
except Exception:
    print("?")
PY
)"
echo "[lb] ${NROUTES} route(s) in the file — a full Town12 route is 10-25 min of wall clock"
echo "[lb] progress:  python3 -c \"import json;print(json.load(open('${CHECKPOINT_ENDPOINT}'))['_checkpoint']['progress'])\""
echo

cd "${LEADERBOARD_ROOT}"
# -u matters: a route is 10-25 minutes and Python block-buffers stdout whenever
# it is not a tty, so `bash run_leaderboard.sh > log` otherwise shows nothing at
# all until the run ends — indistinguishable from a hang.
"${PYTHON}" -u "${LEADERBOARD_ROOT}/leaderboard/leaderboard_evaluator.py" "${ARGS[@]}" "$@"
RC=$?
echo "[lb] evaluator exited ${RC}"
echo "[lb] read the results:  python3 ../read-leaderboard-results/scripts/read_results.py ${CHECKPOINT_ENDPOINT}"
exit ${RC}
