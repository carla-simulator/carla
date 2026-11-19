#!/usr/bin/env bash
# Environment for manage-content-packs: a client skill, it needs an importable carla and a running server.
# Source before the skill's other scripts: source scripts/env.sh
set -euo pipefail

# Paths the install skills recorded, for keys with no value yet; an export wins.
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"
export CARLA_TIMEOUT="${CARLA_TIMEOUT:-60.0}"
export PYTHON="${PYTHON:-python3}"

echo "[env] CARLA_HOST:PORT = ${CARLA_HOST}:${CARLA_PORT}  (timeout ${CARLA_TIMEOUT}s)"
echo "[env] PYTHON          = ${PYTHON}"
