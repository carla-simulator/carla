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
ROOT_DIR="$(dirname "$PARENT_DIR")"
LOG_FILE="${PARENT_DIR}/preview_control.log"

echo "[INFO] Terminal 8: preview control check"
set +u
. /opt/ros/*/setup.bash 2>/dev/null || true
[ -f "${ROOT_DIR}/autoware/install/setup.bash" ] && . "${ROOT_DIR}/autoware/install/setup.bash" 2>/dev/null || true
set -u

if ! command -v ros2 >/dev/null 2>&1; then
  echo "[INFO] ROS2 command not available; preview control inactive"
  exit 0
fi

if ros2 pkg list 2>/dev/null | grep -q '^preview_control$'; then
  echo "[INFO] Starting preview control"
  ros2 launch preview_control carla_control.launch.py >> "$LOG_FILE" 2>&1
  RC=$?
  if [ "$RC" -eq 0 ]; then
    echo "[INFO] Preview control finished"
  else
    echo "[INFO] Preview control stopped (details in ${LOG_FILE})"
  fi
else
  echo "[INFO] Preview control package not installed on this host"
fi

exit 0
