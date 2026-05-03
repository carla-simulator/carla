#!/bin/bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

set -e

INTERVAL=${1:-60}
DURATION=${2:-0}

keep_alive() {
    local method=$1
    case $method in
        "caffeine")
            if command -v caffeine &> /dev/null; then
                echo "[$(date '+%Y-%m-%d %H:%M:%S')] Keeping machine alive with Caffeine..."
                caffeine -a
                return 0
            fi
            ;;
        "xdotool")
            if command -v xdotool &> /dev/null; then
                echo "[$(date '+%Y-%m-%d %H:%M:%S')] Simulating user activity..."
                xdotool mousemove_relative 1 1 2>/dev/null
                sleep 0.5
                xdotool mousemove_relative -- -1 -1 2>/dev/null
                return 0
            fi
            ;;
        "systemctl")
            echo "[$(date '+%Y-%m-%d %H:%M:%S')] Disabling sleep target..."
            sudo systemctl mask sleep.target suspend.target hibernate.target hybrid-sleep.target 2>/dev/null || true
            return 0
            ;;
        "acpi")
            if command -v acpi &> /dev/null; then
                echo "[$(date '+%Y-%m-%d %H:%M:%S')] Checking ACPI status..."
                acpi -b 2>/dev/null || true
            fi
            ;;
    esac
    return 1
}

echo "════════════════════════════════════════════════════════════════"
echo "Machine Keep-Alive Script for CARLA Simulations"
echo "════════════════════════════════════════════════════════════════"
echo "Interval: ${INTERVAL} seconds"
if [ "$DURATION" -gt 0 ]; then
    echo "Duration: ${DURATION} seconds"
else
    echo "Duration: Indefinite (press Ctrl+C to stop)"
fi
echo "════════════════════════════════════════════════════════════════"

if command -v caffeine &> /dev/null; then
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] Starting Caffeine..."
    caffeine -a &
    CAFFEINE_PID=$!
    trap "kill $CAFFEINE_PID 2>/dev/null; exit 0" EXIT INT TERM
fi

elapsed_time=0

while true; do
    keep_alive "xdotool" || true

    if [ "$DURATION" -gt 0 ]; then
        elapsed_time=$((elapsed_time + INTERVAL))
        if [ "$elapsed_time" -ge "$DURATION" ]; then
            echo "[$(date '+%Y-%m-%d %H:%M:%S')] Duration reached. Stopping..."
            break
        fi
        echo "[$(date '+%Y-%m-%d %H:%M:%S')] Elapsed: ${elapsed_time}s / ${DURATION}s"
    fi
    
    sleep "$INTERVAL"
done

echo "[$(date '+%Y-%m-%d %H:%M:%S')] Machine keep-alive script stopped."
