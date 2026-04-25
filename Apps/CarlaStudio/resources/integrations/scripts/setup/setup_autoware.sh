#!/bin/bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

set -e

echo "========================================="
echo "🚀 Autoware Integration Setup"
echo "========================================="
echo ""

AUTOWARE_SOURCE_DIR="${AUTOWARE_SOURCE_DIR:-$HOME/autoware}"

if [ -f "$AUTOWARE_SOURCE_DIR/install/setup.bash" ]; then
    echo "✅ Found compiled Autoware at: $AUTOWARE_SOURCE_DIR/install/"
    echo ""
    echo "Setting up environment..."
    source "$AUTOWARE_SOURCE_DIR/install/setup.bash"
    echo "✅ Autoware environment ready"
    exit 0
fi

echo "🔍 Checking for pre-built Autoware packages..."
echo ""

if command -v ros2 >/dev/null 2>&1; then
    if ! ros2 pkg list 2>/dev/null | grep -q "autoware_launch"; then
        echo "⚠️  Autoware packages not found in system"
        echo ""
        echo "📦 Solution 1: Install pre-built packages (recommended)"
        echo "  sudo apt install -y ros-kilted-autoware-launch ros-kilted-tier4-map-msgs"
        echo ""
        echo "📦 Solution 2: Compile from source (requires Python 3.12 fix)"
        echo "  cd $AUTOWARE_SOURCE_DIR"
        echo "  colcon build --symlink-install (needs Python distutils for 3.12)"
        echo ""
        exit 1
    else
        echo "✅ Found system Autoware packages"
        echo "Ready to launch planning simulator"
        exit 0
    fi
else
    echo "⚠️  ROS2 not sourced. Run:"
    echo "  source /opt/ros/kilted/setup.bash"
    exit 1
fi
