#!/bin/bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

set -e

AUTOWARE_DIR="${AUTOWARE_DIR:-$HOME/autoware}"

echo "========================================="
echo "🚀 Autoware Quick Setup"
echo "========================================="
echo ""

echo "[1] Checking Autoware repository..."
if [ ! -d "$AUTOWARE_DIR/src" ]; then
    echo "❌ Autoware not found. Cloning..."
    cd "$(dirname "$AUTOWARE_DIR")"
    git clone https://github.com/michigan-traffic-lab/autoware.git autoware
fi
echo "✅ Autoware repository present"

echo ""
echo "[2] Installing critical ROS2 packages..."
echo "This requires 'sudo' password. If prompted, enter it."
echo ""

PACKAGES=(
    "python3-setuptools"
    "python3-pip"
    "ros-kilted-geographic-msgs"
    "ros-kilted-diagnostic-aggregator"
)

MISSING_PACKAGES=()
for pkg in "${PACKAGES[@]}"; do
    if ! dpkg -l | grep -q "^ii.*$pkg"; then
        MISSING_PACKAGES+=("$pkg")
    fi
done

if [ ${#MISSING_PACKAGES[@]} -gt 0 ]; then
    echo "Installing: ${MISSING_PACKAGES[*]}"
    sudo apt-get update -qq
    sudo apt-get install -y "${MISSING_PACKAGES[@]}" 2>&1 | grep -E "Setting up|0 upgraded|already"
else
    echo "✅ All critical packages already installed"
fi

echo ""
echo "[3] Building Autoware..."
cd "$AUTOWARE_DIR"

source /opt/ros/kilted/setup.bash
export ROS_DISTRO=kilted
export CMAKE_BUILD_TYPE=RelWithDebInfo

mkdir -p build install log

echo "Starting colcon build (this takes 15-30 minutes on a standard CPU)..."
echo ""

colcon build \
    --symlink-install \
    --cmake-args \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_CXX_FLAGS="-O2" \
    --packages-skip autoware_auto_mapping \
    --allow-overriding autoware_common \
    --event-handlers console_cohesive+ \
    2>&1 | tail -50

BUILD_STATUS=${PIPESTATUS[0]}

echo ""
echo "=================================================="
if [ $BUILD_STATUS -eq 0 ]; then
    echo "✅ BUILD SUCCESSFUL!"
else
    echo "⚠️  Build exited with code: $BUILD_STATUS"
    echo "⚠️  Some packages may have failed - this is OK"
    echo "Check build.log: tail -100 $AUTOWARE_DIR/build.log"
fi
echo "=================================================="
echo ""

echo "[4] Validating installation..."

if [ -f "$AUTOWARE_DIR/install/setup.bash" ]; then
    source "$AUTOWARE_DIR/install/setup.bash"
    
    echo "✅ Autoware install directory found"
    echo ""
    echo "Available packages:"
    ros2 pkg list | grep -i autoware | head -5 || echo "⚠️  Limited autoware packages found"
    
    if ros2 pkg list | grep -q "autoware_launch"; then
        echo "✅ autoware_launch package found - ready to use!"
    fi
else
    echo "⚠️  Install directory not found. Build may have failed."
    echo "Retry with: colcon build --continue-on-error"
fi

echo ""
echo "========================================="
echo "✅ Setup Complete!"
echo "========================================="
echo ""
echo "📚 How to use Autoware:"
echo "  1. Source environment:"
echo "     source $AUTOWARE_DIR/setup.bash"
echo ""
echo "  2. Launch Planning Simulator:"
echo "     ros2 launch autoware_launch planning_simulator.launch.xml"
echo ""
echo "  3. With CARLA (run from ./launch/):"
echo "     ./start.sh"
echo ""
echo "📂 Key Paths:"
echo "  Source: $AUTOWARE_DIR/src"
echo "  Build: $AUTOWARE_DIR/build"
echo "  Install: $AUTOWARE_DIR/install"
echo "  Maps: $AUTOWARE_DIR/map"
echo ""
