#!/usr/bin/env bash
# Environment for the apply-textures skill; source it first. Needs an importable carla module and a running server.
# CARLA_HOST (127.0.0.1), CARLA_PORT (2000), CARLA_TIMEOUT (10.0), PYTHON (python3).

set -euo pipefail

# Paths the install skills recorded fill unset keys; an exported variable wins (Skills/_common/env_common.sh).
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

export CARLA_HOST="${CARLA_HOST:-127.0.0.1}"
export CARLA_PORT="${CARLA_PORT:-2000}"
export CARLA_TIMEOUT="${CARLA_TIMEOUT:-10.0}"
export PYTHON="${PYTHON:-python3}"

echo "[env] CARLA_HOST:PORT = ${CARLA_HOST}:${CARLA_PORT}  (timeout ${CARLA_TIMEOUT}s)"
echo "[env] PYTHON          = ${PYTHON}"
