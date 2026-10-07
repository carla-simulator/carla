#!/usr/bin/env bash
# Self-contained environment for the bounding-boxes skill (RUNTIME/client).
# Source before the skill's other scripts:  source env.sh
#
# Acts on a *running* CARLA server through the Python API — needs an importable
# `carla` module and a reachable server, NOT UE4_ROOT or a source checkout.
#
#   CARLA_HOST     server address                       (default 127.0.0.1)
#   CARLA_PORT     server RPC port                      (default 2000)
#   CARLA_TIMEOUT  client connect timeout, seconds      (default 10.0)
#   PYTHON         interpreter whose python3 imports carla (default python3)

set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an exported
# variable still wins. See skills/_common/env_common.sh.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"
export CARLA_TIMEOUT="${CARLA_TIMEOUT:-10.0}"
export PYTHON="${PYTHON:-python3}"

echo "[env] CARLA_HOST:PORT = ${CARLA_HOST}:${CARLA_PORT}  (timeout ${CARLA_TIMEOUT}s)"
echo "[env] PYTHON          = ${PYTHON}"
