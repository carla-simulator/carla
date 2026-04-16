#!/bin/bash
# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Compute the REP-2011 RIHS01 type hash for a ROS 2 message type by building
# the .msg file inside an osrf/ros:jazzy-desktop Docker container.
# No local ROS 2 installation is required.
#
# Usage:
#   compute_type_hash.sh <pkg/msg/TypeName> <path/to/TypeName.msg>
#
# Arguments:
#   <pkg/msg/TypeName>      Full ROS 2 type name, e.g. sensor_msgs/msg/Imu
#   <path/to/TypeName.msg>  Path to the .msg file on the host
#
# Output (stdout):
#   RIHS01_<64 hex chars>  ready to paste into CdrTopicInfo.h
#
# Exit codes:
#   0  hash printed to stdout
#   1  invalid arguments, file not found, or build failure
#
# Requirements:
#   Docker must be installed and running.
#
# Package dependencies (std_msgs, geometry_msgs, ...) are detected
# automatically from field type declarations in the .msg file.
#
# Examples:
#   # Standard ROS 2 type (hash matches what is already in CdrTopicInfo.h):
#   ./compute_type_hash.sh sensor_msgs/msg/Imu \
#       /opt/ros/jazzy/share/sensor_msgs/msg/Imu.msg
#
#   # New carla_msgs type:
#   ./compute_type_hash.sh carla_msgs/msg/CarlaStatus \
#       /path/to/carla_msgs/msg/CarlaStatus.msg

set -euo pipefail

# ---------------------------------------------------------------------------
# Usage
# ---------------------------------------------------------------------------
usage() {
    cat <<'EOF'
Usage: compute_type_hash.sh <pkg/msg/TypeName> <path/to/TypeName.msg>

Arguments:
  <pkg/msg/TypeName>      Full ROS 2 type name, e.g. sensor_msgs/msg/Imu
  <path/to/TypeName.msg>  Path to the .msg file on the host

Output (stdout):
  RIHS01_<64 hex chars>  ready to paste into CdrTopicInfo.h

Requirements:
  Docker must be installed and running.

EOF
    exit "${1:-0}"
}

if [[ $# -eq 1 && ( "$1" == "-h" || "$1" == "--help" ) ]]; then
    usage 0
fi

[[ $# -ne 2 ]] && usage 1

ROS_TYPE="$1"
MSG_FILE="$2"

# ---------------------------------------------------------------------------
# Validate arguments
# ---------------------------------------------------------------------------
if ! [[ "$ROS_TYPE" =~ ^[a-zA-Z_][a-zA-Z0-9_]*/msg/[a-zA-Z_][a-zA-Z0-9_]*$ ]]; then
    echo "ERROR: type must be 'pkg/msg/TypeName', got: $ROS_TYPE" >&2
    exit 1
fi

if [[ ! -f "$MSG_FILE" ]]; then
    echo "ERROR: .msg file not found: $MSG_FILE" >&2
    exit 1
fi

PKG_NAME="${ROS_TYPE%%/msg/*}"
TYPE_NAME="${ROS_TYPE##*/}"

# ---------------------------------------------------------------------------
# Create a temporary colcon workspace
# ---------------------------------------------------------------------------
WS=$(mktemp -d)
trap 'rm -rf "$WS"' EXIT

PKG_DIR="$WS/src/$PKG_NAME"
mkdir -p "$PKG_DIR/msg"
cp "$MSG_FILE" "$PKG_DIR/msg/${TYPE_NAME}.msg"

# Generate package.xml and CMakeLists.txt.
# Dependencies are detected by scanning the .msg file for "pkg/TypeName" field
# type declarations (e.g. "std_msgs/Header header").
python3 - "$MSG_FILE" "$PKG_NAME" "$TYPE_NAME" "$PKG_DIR" <<'PYEOF'
import re, sys, os

msg_file, pkg_name, type_name, pkg_dir = sys.argv[1:]

with open(msg_file) as f:
    content = f.read()

deps = set()
for line in content.splitlines():
    line = line.strip().split('#')[0].strip()   # strip inline comments
    # Match "pkg_name/TypeName[optional_array] field_name"
    m = re.match(r'^([a-zA-Z_][a-zA-Z0-9_]*)/[a-zA-Z_][a-zA-Z0-9_]*(\[.*?\])?\s+\w', line)
    if m:
        dep = m.group(1)
        if dep != pkg_name:
            deps.add(dep)

deps = sorted(deps)

xml_depends = '\n'.join(f'  <depend>{d}</depend>' for d in deps)

with open(os.path.join(pkg_dir, 'package.xml'), 'w') as f:
    f.write(f"""<?xml version="1.0"?>
<package format="3">
  <name>{pkg_name}</name>
  <version>0.0.1</version>
  <description>Temporary package for RIHS01 hash computation</description>
  <maintainer email="tmp@tmp.com">tmp</maintainer>
  <license>MIT</license>
  <buildtool_depend>ament_cmake</buildtool_depend>
  <depend>rosidl_default_generators</depend>
{xml_depends}
  <member_of_group>rosidl_interface_packages</member_of_group>
</package>
""")

find_pkgs = '\n'.join(f'find_package({d} REQUIRED)' for d in deps)
rosidl_deps = ('  DEPENDENCIES ' + ' '.join(deps)) if deps else ''

with open(os.path.join(pkg_dir, 'CMakeLists.txt'), 'w') as f:
    f.write(f"""cmake_minimum_required(VERSION 3.8)
project({pkg_name})
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
{find_pkgs}
rosidl_generate_interfaces(${{PROJECT_NAME}}
  "msg/{type_name}.msg"
{rosidl_deps}
)
ament_package()
""")
PYEOF

# ---------------------------------------------------------------------------
# Build inside Docker and extract the hash
# ---------------------------------------------------------------------------
echo "[hash] Building ${ROS_TYPE} inside osrf/ros:jazzy-desktop ..." >&2

docker run --rm \
    --volume="${WS}:/ws" \
    osrf/ros:jazzy-desktop \
    bash -c "
        set -euo pipefail
        source /opt/ros/jazzy/setup.bash
        cd /ws
        colcon build \
            --packages-select ${PKG_NAME} \
            --cmake-args -DCMAKE_BUILD_TYPE=Release \
            --log-base /tmp/colcon-log \
            > /tmp/colcon-out.txt 2>&1 \
        || { echo 'ERROR: colcon build failed:' >&2; cat /tmp/colcon-out.txt >&2; exit 1; }
        JSON_FILE=/ws/install/${PKG_NAME}/share/${PKG_NAME}/msg/${TYPE_NAME}.json
        if [[ ! -f \"\$JSON_FILE\" ]]; then
            echo 'ERROR: generated JSON not found: '\"\$JSON_FILE\" >&2
            exit 1
        fi
        jq -re --arg t '${ROS_TYPE}' \
            '.type_hashes[] | select(.type_name == \$t) | .hash_string' \
            \"\$JSON_FILE\" \
        || { echo \"ERROR: '${ROS_TYPE}' not found in \$JSON_FILE\" >&2; exit 1; }
    "
