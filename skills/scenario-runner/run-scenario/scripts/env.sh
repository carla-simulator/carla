#!/usr/bin/env bash
# Self-contained environment for the run-scenario skill (ScenarioRunner client).
# Source before the skill's other scripts:  source scripts/env.sh
#
# ScenarioRunner is pure Python: nothing is built. What it needs is an
# importable `carla`, the `agents` package that ships *only* inside a CARLA
# tree (the pip/wheel `carla` does NOT contain it), the checkout itself on
# PYTHONPATH, and a running simulator.
#
#   SCENARIO_RUNNER_ROOT  the scenario_runner checkout (holds scenario_runner.py)
#   CARLA_ROOT            CARLA release or checkout root — source of `agents`
#   CARLA_HOST            simulator address                     (default 127.0.0.1)
#   CARLA_PORT            simulator RPC port                    (default 2000)
#   CARLA_TM_PORT         Traffic Manager port                  (default 8000)
#   PYTHON                interpreter that imports carla        (default python3)

set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

carla_sr_is_root() { [ -f "${1:-}/scenario_runner.py" ] && [ -d "${1:-}/srunner" ]; }

# Resolution order: an explicit export always wins, then the working directory
# (so `cd ~/scenario_runner && bash .../run_scenario.sh` just works), then the
# two places people actually clone it. Never guessed silently — env.sh echoes
# what it picked and check_env.sh FAILs when nothing resolves.
if [ -z "${SCENARIO_RUNNER_ROOT:-}" ]; then
  for _c in "${PWD}" "${HOME}/scenario_runner" "/workspace/scenario_runner" "${HOME}/carla/scenario_runner"; do
    if carla_sr_is_root "${_c}"; then SCENARIO_RUNNER_ROOT="${_c}"; break; fi
  done
fi
export SCENARIO_RUNNER_ROOT="${SCENARIO_RUNNER_ROOT:-}"

# CARLA_ROOT is only needed for PythonAPI/carla (the `agents` package and, on a
# source build, the egg). CARLA_TARGET is what the rest of this skill library
# uses, so accept it as a fallback rather than making the user set two vars.
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

# The checkout must precede site-packages: a `pip install scenario_runner` copy
# (or an older checkout) otherwise shadows the branch that is checked out here,
# which surfaces as scenarios that exist on disk but "are not supported".
_pp=""
[ -n "${SCENARIO_RUNNER_ROOT}" ] && _pp="${SCENARIO_RUNNER_ROOT}"
if [ -n "${CARLA_ROOT}" ] && [ -d "${CARLA_ROOT}/PythonAPI/carla" ]; then
  _pp="${_pp:+${_pp}:}${CARLA_ROOT}/PythonAPI/carla"
  # A source build keeps the client egg here; a release also ships one. Harmless
  # when `carla` is already a wheel — the wheel wins by import order.
  for _egg in "${CARLA_ROOT}"/PythonAPI/carla/dist/carla-*.egg; do
    [ -e "${_egg}" ] && _pp="${_pp}:${_egg}"
  done
fi
export PYTHONPATH="${_pp}${PYTHONPATH:+:${PYTHONPATH}}"
# --debug prints py_trees' unicode tick marks; ascii stdout (docker, cron, pipes)
# raises UnicodeEncodeError mid-run and kills the scenario.
export PYTHONIOENCODING="${PYTHONIOENCODING:-utf-8}"

# Which branch is checked out decides which scenarios, towns and blueprints are
# valid; every SR skill keys its advice off this. "detached"/"unknown" is fine.
carla_sr_branch() {
  [ -n "${SCENARIO_RUNNER_ROOT}" ] || { echo "unknown"; return; }
  git -C "${SCENARIO_RUNNER_ROOT}" rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown"
}

# Flavour, not branch name: several branches behave identically (SR
# leaderboard-2.0 and leaderboard-2.1 are the *same commit*), and the name is
# "HEAD" in a detached checkout, worktree, tarball or docker image — so fall back
# to a content probe. srunner/scenic/ exists only on ue5-master, and
# change_lane.py exists on every branch except ue5-master.
carla_sr_flavor() {
  case "$(carla_sr_branch)" in
    master|0.9.1[3-9]*)      echo "ue4"; return ;;
    ue5-master)              echo "ue5"; return ;;
    ue58-dev|ue58-*)         echo "ue58"; return ;;
    leaderboard-2.0|leaderboard-2.1) echo "lb2"; return ;;
    leaderboard|leaderboard-1.0)     echo "lb1"; return ;;
  esac
  if [ -d "${SCENARIO_RUNNER_ROOT}/srunner/scenic" ] \
     && [ ! -f "${SCENARIO_RUNNER_ROOT}/srunner/scenarios/change_lane.py" ]; then
    # ue58-dev retargeted every towns-1-9 config onto the _Opt map names, which
    # is what makes those 89 configs runnable at all; ue5-master still names the
    # non-_Opt towns, which do not exist in the UE5 content.
    if grep -rlq 'town="Town0[1-9]_Opt"' "${SCENARIO_RUNNER_ROOT}/srunner/examples" 2>/dev/null; then
      echo "ue58"
    else
      echo "ue5"
    fi
  elif [ -f "${SCENARIO_RUNNER_ROOT}/srunner/scenarios/route_scenario.py" ]; then
    echo "unknown"
  else
    echo "unknown"
  fi
}

echo "[env] SCENARIO_RUNNER_ROOT = ${SCENARIO_RUNNER_ROOT:-<unset>}  (branch $(carla_sr_branch), flavor $(carla_sr_flavor))"
echo "[env] CARLA_ROOT           = ${CARLA_ROOT:-<unset>}"
echo "[env] CARLA_HOST:PORT      = ${CARLA_HOST}:${CARLA_PORT}  (TM ${CARLA_TM_PORT})"
echo "[env] PYTHON               = ${PYTHON}"
