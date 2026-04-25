#!/bin/bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"
LOG_FILE="${PARENT_DIR}/terasim_cosim.log"
EXAMPLES_DIR="${PARENT_DIR}/examples/carla_examples"

echo "[INFO] Terminal 5: preparing co-simulation"

if ! python3 -c "import carla, terasim_cosim" >/dev/null 2>&1; then
  echo "[INFO] Co-simulation dependencies unavailable on this host"
  exit 0
fi

if ! pgrep -x redis-server >/dev/null 2>&1; then
  if command -v redis-server >/dev/null 2>&1; then
    nohup redis-server > "${PARENT_DIR}/redis.log" 2>&1 &
    echo "[INFO] Redis service started"
  else
    echo "[INFO] Redis service not present; co-simulation will not run"
    exit 0
  fi
fi

echo "[INFO] Waiting for CARLA readiness"
READY=0
for _ in $(seq 1 90); do
  if python3 - <<'PY' >/dev/null 2>&1
import carla
c = carla.Client('localhost', 2000)
c.set_timeout(2.0)
c.get_world()
PY
  then
    READY=1
    break
  fi
  sleep 1
done

if [ "$READY" -ne 1 ]; then
  echo "[INFO] CARLA not ready yet; skipping co-simulation for now"
  exit 0
fi

echo "[INFO] Starting co-simulation"
cd "$EXAMPLES_DIR" || exit 0
python3 carla_cosim_ros2.py >> "$LOG_FILE" 2>&1
RC=$?
if [ "$RC" -eq 0 ]; then
  echo "[INFO] Co-simulation finished"
else
  echo "[INFO] Co-simulation stopped (details in ${LOG_FILE})"
fi
exit 0
