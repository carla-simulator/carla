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
echo "Autoware ROS2 Build Setup"
echo "========================================="

echo "[1/5] Verifying Autoware repository..."
if [ ! -d "$AUTOWARE_DIR/.git" ]; then
    echo "❌ Autoware repository clone not complete. Waiting..."
    sleep 60
fi

if [ ! -f "$AUTOWARE_DIR/README.md" ]; then
    echo "⏳ Clone still in progress. Please wait..."
    exit 1
fi
echo "✅ Autoware repository found"

echo "[2/5] Setting up ROS2 environment..."
source /opt/ros/kilted/setup.bash
echo "✅ ROS2 environment sourced"

echo "[3/5] Installing build dependencies..."
cd "$AUTOWARE_DIR"

rosdep update 2>/dev/null || echo "⚠️ rosdep update skipped"

if [ -f "$AUTOWARE_DIR/src/autoware/autoware.universe/package.xml" ]; then
    rosdep install -y --from-paths src --ignore-src \
        --rosdistro=$ROS_DISTRO 2>/dev/null || \
        echo "⚠️ Some dependencies may be missing, continuing..."
fi
echo "✅ Dependencies installed"

echo "[4/5] Building Autoware with colcon..."
mkdir -p build install log

colcon build \
    --symlink-install \
    --cmake-args -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    --event-handlers console_direct+ 2>&1 | tee build.log

BUILD_RESULT=${PIPESTATUS[0]}

if [ $BUILD_RESULT -eq 0 ]; then
    echo "✅ Autoware build successful"
else
    echo "⚠️ Autoware build completed with exit code: $BUILD_RESULT"
    echo "Check build.log for details"
fi

echo "[5/5] Setting up launch scripts..."

cat > "$AUTOWARE_DIR/setup.bash" << EOF
#!/bin/bash
source /opt/ros/kilted/setup.bash
source "$AUTOWARE_DIR/install/setup.bash"
export AUTOWARE_ROOT="$AUTOWARE_DIR"
EOF

chmod +x "$AUTOWARE_DIR/setup.bash"
echo "✅ Setup script created at $AUTOWARE_DIR/setup.bash"

cat > "$AUTOWARE_DIR/launch_autoware.sh" << 'EOF'
#!/bin/bash
# Launch Autoware from this location

AUTOWARE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$AUTOWARE_ROOT/setup.bash"

# Ensure map path exists
if [ -z "$AUTOWARE_MAP_PATH" ]; then
    AUTOWARE_MAP_PATH="$AUTOWARE_ROOT/map"
fi

# Launch command
ros2 launch autoware_launch planning_simulator.launch.xml \
    map_path:="$AUTOWARE_MAP_PATH" \
    vehicle_model:=sample_vehicle \
    sensor_model:=sample_sensor_kit \
    lanelet2_map_file:=lanelet2_mcity.osm
EOF

chmod +x "$AUTOWARE_DIR/launch_autoware.sh"
echo "✅ Launch helper created at $AUTOWARE_DIR/launch_autoware.sh"

echo "========================================="
echo "✅ Autoware setup complete!"
echo "========================================="
echo ""
echo "To use Autoware, source the setup:"
echo "  source $AUTOWARE_DIR/setup.bash"
echo ""
echo "Or run with:"
echo "  $AUTOWARE_DIR/launch_autoware.sh"
echo ""
