#!/usr/bin/env bash
# Self-contained environment for the load-map skill (a RUNTIME/client skill).
# Source before the skill's other scripts:  source env.sh
#
# Unlike the build skills, load-map acts on a *running* CARLA server through the
# Python API — it needs an importable `carla` module and a reachable server, NOT
# UE4_ROOT or a source checkout. The target server is chosen at runtime:
#
#   CARLA_HOST     server address                       (default 127.0.0.1)
#   CARLA_PORT     server RPC port                      (default 2000)
#   CARLA_TIMEOUT  client connect timeout, seconds      (default 10.0)
#   PYTHON         interpreter whose `python3` imports carla (default python3)
#
# CARLA_ROOT is OPTIONAL here — only used to locate sample .xodr files and the
# stock config.py. It is not required to load a map.

set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"
export CARLA_TIMEOUT="${CARLA_TIMEOUT:-10.0}"
export PYTHON="${PYTHON:-python3}"

# --- Optional: locate a carla checkout for samples (never a hard requirement) --
# Precedence: explicit CARLA_ROOT > CARLA_UE4_ROOT (build skills' var) > $PWD if
# it looks like a checkout. Left empty when none is found; the skill still works.
if [ -z "${CARLA_ROOT:-}" ]; then
  if [ -n "${CARLA_UE4_ROOT:-}" ]; then
    CARLA_ROOT="${CARLA_UE4_ROOT}"
  elif [ -f "${PWD}/PythonAPI/util/config.py" ]; then
    CARLA_ROOT="${PWD}"
  fi
fi
export CARLA_ROOT="${CARLA_ROOT:-}"

echo "[env] CARLA_HOST:PORT = ${CARLA_HOST}:${CARLA_PORT}  (timeout ${CARLA_TIMEOUT}s)"
echo "[env] PYTHON          = ${PYTHON}"
echo "[env] CARLA_ROOT      = ${CARLA_ROOT:-<unset — only needed for sample .xodr / config.py>}"
