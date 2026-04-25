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
INSTALL_DIR="$AUTOWARE_DIR/install"

echo "========================================="
echo "Autoware ROS2 Advanced Build"
echo "========================================="

if [ ! -d "$AUTOWARE_DIR/src" ]; then
    echo "❌ Autoware src directory not found"
    exit 1
fi

cd "$AUTOWARE_DIR"

echo "[1/4] Setting up ROS2 environment..."
source /opt/ros/kilted/setup.bash
export ROS_DISTRO=kilted
echo "✅ ROS2 environment configured"

echo "[2/4] Preparing build directories..."
mkdir -p build install log

echo "[3/4] Building Autoware (allowing missing dependencies)..."
echo "This may take 10-30 minutes depending on system resources..."
echo ""

colcon build \
    --symlink-install \
    --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    --continue-on-error \
    --event-handlers console_direct+ \
    2>&1 | tee -a build.log

BUILD_EXIT=${PIPESTATUS[0]}

echo ""
echo "=================================================="
if [ $BUILD_EXIT -eq 0 ]; then
    echo "✅ Build completed successfully!"
else
    echo "⚠️  Build completed with exit code: $BUILD_EXIT"
    echo "⚠️  This is normal if some optional packages are missing"
    echo "Check build.log for details: tail -100 build.log"
fi
echo "=================================================="
echo ""

echo "[4/4] Creating setup helpers..."

cat > "$AUTOWARE_DIR/setup.bash" << 'SETUP_EOF'
#!/bin/bash
# Source ROS2 and Autoware installation

# Source ROS2 if not already sourced
if [ -z "$ROS_DISTRO" ]; then
    source /opt/ros/kilted/setup.bash
fi

# Source Autoware installation
AUTOWARE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$AUTOWARE_ROOT/install/setup.bash"

export AUTOWARE_ROOT
export AUTOWARE_MAP_PATH="${AUTOWARE_ROOT}/map"
export ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-0}

echo "✅ Autoware environment loaded from: $AUTOWARE_ROOT"
SETUP_EOF

chmod +x "$AUTOWARE_DIR/setup.bash"

cat > "$AUTOWARE_DIR/run_autoware.sh" << 'RUN_EOF'
#!/bin/bash
# Run Autoware with proper environment setup

AUTOWARE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$AUTOWARE_ROOT/setup.bash"

# Validate map directory
if [ ! -d "$AUTOWARE_MAP_PATH" ]; then
    echo "⚠️  Autoware map directory not found at: $AUTOWARE_MAP_PATH"
    echo "Creating directory..."
    mkdir -p "$AUTOWARE_MAP_PATH"
fi

# List available launch files
echo "📦 Available Autoware packages:"
ros2 pkg list | grep -i autoware | head -10 || echo "⚠️  No autoware packages found"

echo ""
echo "🚀 To launch Autoware, try:"
echo "  ros2 launch autoware_launch planning_simulator.launch.xml"
echo ""
RUN_EOF

chmod +x "$AUTOWARE_DIR/run_autoware.sh"

echo "✅ Setup scripts created"
echo ""
echo "========================================="
echo "✅ Autoware build setup complete!"
echo "========================================="
echo ""
echo "Quick Start:"
echo "1. Source Autoware: source $AUTOWARE_DIR/setup.bash"
echo "2. List packages: ros2 pkg list | grep autoware"
echo "3. View launch files: ros2 pkg list-contains launch_xml autoware_launch"
echo ""
echo "Build Logs:"
echo "  Full log: $AUTOWARE_DIR/build.log"
echo "  Latest build output:"
tail -20 "$AUTOWARE_DIR/build.log" 2>/dev/null

echo ""
