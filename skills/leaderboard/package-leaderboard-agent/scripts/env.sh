#!/usr/bin/env bash
# Self-contained environment for the package-leaderboard-agent skill (CARLA Leaderboard).
# Source before the skill's other scripts:  source scripts/env.sh
#
# The Leaderboard is a thin evaluation harness on top of ScenarioRunner: it
# imports `srunner` for the scenario library and criteria, and `carla` +
# `agents` for the simulator. All four roots must agree on a version — that
# pairing is the single most common cause of a broken setup, so env.sh derives
# and prints it instead of trusting the user.
#
#   LEADERBOARD_ROOT      the leaderboard checkout (holds leaderboard/leaderboard_evaluator.py)
#   SCENARIO_RUNNER_ROOT  the *matching* scenario_runner checkout
#   CARLA_ROOT            CARLA release root — source of `agents` and the egg
#   TEAM_AGENT            path to the agent .py to evaluate
#   TEAM_CONFIG           optional path passed to the agent's setup()
#   CHALLENGE_TRACK_CODENAME  SENSORS | MAP | SENSORS_QUALIFIER | MAP_QUALIFIER
#   CARLA_HOST/CARLA_PORT/CARLA_TM_PORT  where the simulator listens
#   PYTHON                interpreter that imports carla        (default python3)

set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

carla_lb_is_root() { [ -f "${1:-}/leaderboard/leaderboard_evaluator.py" ]; }
carla_sr_is_root() { [ -f "${1:-}/scenario_runner.py" ] && [ -d "${1:-}/srunner" ]; }

if [ -z "${LEADERBOARD_ROOT:-}" ]; then
  for _c in "${PWD}" "${HOME}/leaderboard" "/workspace/leaderboard"; do
    if carla_lb_is_root "${_c}"; then LEADERBOARD_ROOT="${_c}"; break; fi
  done
fi
export LEADERBOARD_ROOT="${LEADERBOARD_ROOT:-}"

if [ -z "${SCENARIO_RUNNER_ROOT:-}" ]; then
  for _c in "${HOME}/scenario_runner" "/workspace/scenario_runner" "$(dirname "${LEADERBOARD_ROOT:-/nonexistent}")/scenario_runner"; do
    if carla_sr_is_root "${_c}"; then SCENARIO_RUNNER_ROOT="${_c}"; break; fi
  done
fi
export SCENARIO_RUNNER_ROOT="${SCENARIO_RUNNER_ROOT:-}"

if [ -z "${CARLA_ROOT:-}" ]; then
  for _c in "${CARLA_TARGET:-}" "${CARLA_PACKAGE_ROOT:-}" "${CARLA_UE4_ROOT:-}"; do
    if [ -n "${_c}" ] && [ -d "${_c}/PythonAPI/carla" ]; then CARLA_ROOT="${_c}"; break; fi
  done
fi
export CARLA_ROOT="${CARLA_ROOT:-}"

export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"
export CARLA_TM_PORT="${CARLA_TM_PORT:-8000}"
export PYTHON="${PYTHON:-python3}"
export CHALLENGE_TRACK_CODENAME="${CHALLENGE_TRACK_CODENAME:-SENSORS}"

# Order matters and is the same as the official Dockerfile: leaderboard and
# scenario_runner before site-packages, `agents` from the CARLA tree.
_pp=""
[ -n "${LEADERBOARD_ROOT}" ]     && _pp="${LEADERBOARD_ROOT}"
[ -n "${SCENARIO_RUNNER_ROOT}" ] && _pp="${_pp:+${_pp}:}${SCENARIO_RUNNER_ROOT}"
if [ -n "${CARLA_ROOT}" ] && [ -d "${CARLA_ROOT}/PythonAPI/carla" ]; then
  _pp="${_pp:+${_pp}:}${CARLA_ROOT}/PythonAPI/carla"
  for _egg in "${CARLA_ROOT}"/PythonAPI/carla/dist/carla-*.egg; do
    [ -e "${_egg}" ] && _pp="${_pp}:${_egg}"
  done
fi
export PYTHONPATH="${_pp}${PYTHONPATH:+:${PYTHONPATH}}"
export PYTHONIOENCODING="${PYTHONIOENCODING:-utf-8}"

carla_lb_branch() {
  [ -n "${LEADERBOARD_ROOT}" ] || { echo "unknown"; return; }
  git -C "${LEADERBOARD_ROOT}" rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown"
}

# The leaderboard "version" is a property of the checkout, not of a version
# string in a file — there is none. Two facts identify it unambiguously:
#   * route format: <waypoints>/<scenarios>/<weathers> (2.x) vs flat <waypoint> (1.0)
#   * infraction maths: PENALTY_PERC_DICT present (2.0) vs absent (2.1, additive)
# so detection reads the code rather than the branch name, which lets it work in
# a tarball, a docker image or a detached HEAD.
carla_lb_version() {
  local sm="${LEADERBOARD_ROOT}/leaderboard/utils/statistics_manager.py"
  local aa="${LEADERBOARD_ROOT}/leaderboard/autoagents/autonomous_agent.py"
  [ -f "${sm}" ] && [ -f "${aa}" ] || { echo "unknown"; return; }
  # The qualifier tracks arrived with 2.0; 1.0 has only SENSORS and MAP.
  grep -q "SENSORS_QUALIFIER" "${aa}" || { echo "1.0"; return; }
  # 2.0 multiplies penalties (needs the percentage table); 2.1 sums them.
  if grep -q "PENALTY_PERC_DICT" "${sm}"; then echo "2.0"; else echo "2.1"; fi
}

# Which scenario_runner branch this leaderboard version requires.
carla_lb_required_sr_branch() {
  case "$(carla_lb_version)" in
    1.0) echo "leaderboard-1.0" ;;
    2.0) echo "leaderboard-2.0" ;;
    2.1) echo "leaderboard-2.1" ;;
    *)   echo "unknown" ;;
  esac
}

echo "[env] LEADERBOARD_ROOT     = ${LEADERBOARD_ROOT:-<unset>}  (branch $(carla_lb_branch), version $(carla_lb_version))"
echo "[env] SCENARIO_RUNNER_ROOT = ${SCENARIO_RUNNER_ROOT:-<unset>}"
echo "[env] CARLA_ROOT           = ${CARLA_ROOT:-<unset>}"
echo "[env] CARLA_HOST:PORT      = ${CARLA_HOST}:${CARLA_PORT}  (TM ${CARLA_TM_PORT})"
echo "[env] TRACK                = ${CHALLENGE_TRACK_CODENAME}"
echo "[env] TEAM_AGENT           = ${TEAM_AGENT:-<unset>}"
