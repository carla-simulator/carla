#!/usr/bin/env bash
# Self-contained environment for the install-leaderboard skill (SETUP).
# Source before the skill's other scripts:  source scripts/env.sh
#
# This skill *creates* LEADERBOARD_ROOT / SCENARIO_RUNNER_ROOT, so an unset value
# is normal here rather than an error.
#
#   LEADERBOARD_ROOT      existing leaderboard checkout      (optional)
#   SCENARIO_RUNNER_ROOT  existing scenario_runner checkout  (optional)
#   LB_INSTALL_DIR        parent dir for new clones          (default $HOME)
#   CARLA_ROOT            CARLA build matching the LB version
#   CARLA_HOST/CARLA_PORT a running server, used only to read version + maps
#   PYTHON                interpreter that will import carla (default python3)

set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export LEADERBOARD_ROOT="${LEADERBOARD_ROOT:-}"
export SCENARIO_RUNNER_ROOT="${SCENARIO_RUNNER_ROOT:-}"
export LB_INSTALL_DIR="${LB_INSTALL_DIR:-${HOME}}"

if [ -z "${CARLA_ROOT:-}" ]; then
  for _c in "${CARLA_TARGET:-}" "${CARLA_PACKAGE_ROOT:-}" "${CARLA_UE4_ROOT:-}"; do
    if [ -n "${_c}" ] && [ -d "${_c}/PythonAPI/carla" ]; then CARLA_ROOT="${_c}"; break; fi
  done
fi
export CARLA_ROOT="${CARLA_ROOT:-}"
export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"
export PYTHON="${PYTHON:-python3}"
export PYTHONIOENCODING="${PYTHONIOENCODING:-utf-8}"

echo "[env] LEADERBOARD_ROOT     = ${LEADERBOARD_ROOT:-<unset: will install under ${LB_INSTALL_DIR}>}"
echo "[env] SCENARIO_RUNNER_ROOT = ${SCENARIO_RUNNER_ROOT:-<unset>}"
echo "[env] CARLA_ROOT           = ${CARLA_ROOT:-<unset>}"
echo "[env] CARLA_HOST:PORT      = ${CARLA_HOST}:${CARLA_PORT}"
echo "[env] PYTHON               = ${PYTHON}"
