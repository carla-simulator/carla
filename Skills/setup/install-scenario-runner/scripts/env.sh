#!/usr/bin/env bash
# Self-contained environment for the install-scenario-runner skill (SETUP).
# Source before the skill's other scripts:  source scripts/env.sh
#
# This skill *creates* SCENARIO_RUNNER_ROOT, so unlike the scenario-runner group
# it treats an unset value as normal, not as an error.
#
#   SCENARIO_RUNNER_ROOT  existing checkout to switch/verify (optional)
#   SR_INSTALL_DIR        where to clone when there is none  (default ~/scenario_runner)
#   CARLA_ROOT            CARLA release/checkout — source of the `agents` package
#                         (falls back to CARLA_UE58_ROOT / CARLA_UE5_ROOT when unset)
#   CARLA_HOST/CARLA_PORT a running server, used only to read its version
#   PYTHON                interpreter that will import carla (default python3)

set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See Skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export SCENARIO_RUNNER_ROOT="${SCENARIO_RUNNER_ROOT:-}"
export SR_INSTALL_DIR="${SR_INSTALL_DIR:-${HOME}/scenario_runner}"

if [ -z "${CARLA_ROOT:-}" ]; then
  # CARLA_UE58_ROOT (UE 5.8 tree) and CARLA_UE5_ROOT (UE 5.5 tree) are what
  # set_config records for UE5 source checkouts.
  for _c in "${CARLA_TARGET:-}" "${CARLA_PACKAGE_ROOT:-}" "${CARLA_UE4_ROOT:-}" \
            "${CARLA_UE58_ROOT:-}" "${CARLA_UE5_ROOT:-}"; do
    if [ -n "${_c}" ] && [ -d "${_c}/PythonAPI/carla" ]; then CARLA_ROOT="${_c}"; break; fi
  done
fi
export CARLA_ROOT="${CARLA_ROOT:-}"
export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"
export PYTHON="${PYTHON:-python3}"
export PYTHONIOENCODING="${PYTHONIOENCODING:-utf-8}"

echo "[env] SCENARIO_RUNNER_ROOT = ${SCENARIO_RUNNER_ROOT:-<unset: will install to ${SR_INSTALL_DIR}>}"
echo "[env] CARLA_ROOT           = ${CARLA_ROOT:-<unset>}"
echo "[env] CARLA_HOST:PORT      = ${CARLA_HOST}:${CARLA_PORT}"
echo "[env] PYTHON               = ${PYTHON}"
