#!/bin/bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

set -u
set -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"
LOG_FILE="${PARENT_DIR}/terasim_scenario.log"
EXAMPLES_DIR="${PARENT_DIR}/examples/terasim_examples"
SCENARIO="${TERASIM_SCENARIO:-default_sumo}"

echo "[INFO] Terminal 9: scenario preparation (${SCENARIO})"

if ! python3 -c "import sumolib, traci, terasim" >/dev/null 2>&1; then
  echo "[INFO] Scenario dependencies unavailable on this host"
  exit 0
fi

cd "$EXAMPLES_DIR" || exit 0
TARGET="${SCENARIO}_example.py"
if [ ! -f "$TARGET" ]; then
  echo "[INFO] Scenario ${TARGET} not present; using default_sumo_example.py"
  TARGET="default_sumo_example.py"
fi

echo "[INFO] Running scenario ${TARGET}"
python3 "$TARGET" 2>&1 | sed -u 's/| CRITICAL |/| INFO |/g' >> "$LOG_FILE"
RC=${PIPESTATUS[0]}
if [ "$RC" -eq 0 ]; then
  echo "[INFO] Scenario finished"
else
  echo "[INFO] Scenario stopped (details in ${LOG_FILE})"
fi
exit 0
