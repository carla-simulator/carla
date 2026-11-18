#!/usr/bin/env bash
# Self-contained environment for the navigate-to skill (RUNTIME/client).
# Source before the skill's other scripts:  source env.sh
#
# Acts on a running CARLA server AND uses CARLA's navigation `agents` package
# (BasicAgent/BehaviorAgent/GlobalRoutePlanner). That package ships in the CARLA
# checkout's PythonAPI/carla/agents — NOT in the installed `carla` wheel — so this
# skill needs CARLA_ROOT to put it on PYTHONPATH.
#
#   CARLA_HOST     server address                       (default 127.0.0.1)
#   CARLA_PORT     server RPC port                      (default 2000)
#   CARLA_TIMEOUT  client connect timeout, seconds      (default 10.0)
#   CARLA_ROOT     carla checkout (for PythonAPI/carla/agents)
#   PYTHON         interpreter whose python3 imports carla (default python3)

set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See Skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"
export CARLA_TIMEOUT="${CARLA_TIMEOUT:-10.0}"
export PYTHON="${PYTHON:-python3}"

# Resolve a carla checkout that contains the agents package.
# Same candidates, in the same order, as the scenario-runner skills: a
# standalone package (CARLA_TARGET / CARLA_PACKAGE_ROOT) ships agents too.
if [ -z "${CARLA_ROOT:-}" ]; then
  for _c in "${CARLA_TARGET:-}" "${CARLA_PACKAGE_ROOT:-}" "${CARLA_UE4_ROOT:-}" \
            "${CARLA_UE58_ROOT:-}" "${CARLA_UE5_ROOT:-}" "${PWD}"; do
    if [ -n "${_c}" ] && [ -d "${_c}/PythonAPI/carla/agents" ]; then CARLA_ROOT="${_c}"; break; fi
  done
fi
export CARLA_ROOT="${CARLA_ROOT:-}"
if [ -n "${CARLA_ROOT}" ] && [ -d "${CARLA_ROOT}/PythonAPI/carla" ]; then
  carla_pythonpath_prepend "${CARLA_ROOT}/PythonAPI/carla"
fi

echo "[env] CARLA_HOST:PORT = ${CARLA_HOST}:${CARLA_PORT}  (timeout ${CARLA_TIMEOUT}s)"
echo "[env] CARLA_ROOT      = ${CARLA_ROOT:-<unset — needed for the agents package>}"
echo "[env] PYTHON          = ${PYTHON}"
