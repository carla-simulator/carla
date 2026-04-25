#!/bin/bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

wait_for_carla() {
    local max_wait=${1:-60}
    local host=${2:-localhost}
    local port=${3:-2000}
    local elapsed=0
    
    echo "⏳ Waiting for CARLA on $host:$port (max ${max_wait}s)..."
    
    while [ $elapsed -lt $max_wait ]; do
        if timeout 3 python3 -c "
import carla
try:
    client = carla.Client('$host', $port)
    client.set_timeout(5.0)
    world = client.get_world()
    print('✅ CARLA responding', flush=True)
    exit(0)
except:
    exit(1)
" 2>/dev/null; then
            echo "✅ CARLA is ready after ${elapsed}s"
            return 0
        fi
        
        if [ $((elapsed % 10)) -eq 0 ]; then
            echo "  Still waiting... ($elapsed/${max_wait}s)"
        fi
        
        sleep 1
        elapsed=$((elapsed + 1))
    done
    
    echo "⚠️  CARLA not responding after ${max_wait}s (will retry with fallback)"
    return 1
}

wait_for_carla_world() {
    local map_name=${1:-McityMap_Main}
    local max_wait=${2:-60}
    local elapsed=0
    
    echo "⏳ Waiting for map '$map_name' (max ${max_wait}s)..."
    
    while [ $elapsed -lt $max_wait ]; do
        if timeout 3 python3 -c "
import carla
try:
    client = carla.Client('localhost', 2000)
    client.set_timeout(5.0)
    world = client.get_world()
    current_map = world.get_map().name.split('/')[-1]
    if '$map_name' in current_map:
        print(f'✅ Map loaded: {current_map}', flush=True)
        exit(0)
    else:
        print(f'Current map: {current_map}', flush=True)
        exit(1)
except Exception as e:
    print(f'Error: {e}', flush=True)
    exit(1)
" 2>/dev/null; then
            echo "✅ Map '$map_name' is loaded after ${elapsed}s"
            return 0
        fi
        
        if [ $((elapsed % 10)) -eq 0 ]; then
            echo "  Still loading... ($elapsed/${max_wait}s)"
        fi
        
        sleep 1
        elapsed=$((elapsed + 1))
    done
    
    echo "⚠️  Map not loaded after ${max_wait}s (proceeding anyway)"
    return 1
}

wait_for_ros2() {
    local max_wait=${1:-30}
    local elapsed=0
    
    echo "⏳ Checking ROS2 environment (max ${max_wait}s)..."
    
    if ! . /opt/ros/*/setup.bash 2>/dev/null; then
        echo "⚠️  ROS2 not installed or not in standard location"
        return 1
    fi
    
    while [ $elapsed -lt $max_wait ]; do
        if timeout 3 ros2 node list >/dev/null 2>&1; then
            echo "✅ ROS2 is responsive after ${elapsed}s"
            return 0
        fi
        
        if [ $((elapsed % 5)) -eq 0 ] && [ $elapsed -gt 0 ]; then
            echo "  Waiting for ROS2... ($elapsed/${max_wait}s)"
        fi
        
        sleep 1
        elapsed=$((elapsed + 1))
    done
    
    echo "⚠️  ROS2 not responsive after ${max_wait}s (may not be running)"
    return 1
}

wait_for_topic() {
    local topic=${1}
    local max_wait=${2:-60}
    local elapsed=0
    
    if [ -z "$topic" ]; then
        echo "❌ Usage: wait_for_topic <topic_name> [max_seconds]"
        return 1
    fi
    
    echo "⏳ Waiting for ROS2 topic '$topic' (max ${max_wait}s)..."
    
    . /opt/ros/*/setup.bash 2>/dev/null || true
    
    while [ $elapsed -lt $max_wait ]; do
        if timeout 2 ros2 topic list 2>/dev/null | grep -q "^${topic}$"; then
            echo "✅ Topic '$topic' is available after ${elapsed}s"
            return 0
        fi
        
        if [ $((elapsed % 10)) -eq 0 ] && [ $elapsed -gt 0 ]; then
            echo "  Still waiting for topic... ($elapsed/${max_wait}s)"
        fi
        
        sleep 1
        elapsed=$((elapsed + 1))
    done
    
    echo "⚠️  Topic '$topic' not found after ${max_wait}s"
    return 1
}

wait_for_redis() {
    local max_wait=${1:-30}
    local host=${2:-localhost}
    local port=${3:-6379}
    local elapsed=0
    
    echo "⏳ Waiting for Redis on $host:$port (max ${max_wait}s)..."
    
    while [ $elapsed -lt $max_wait ]; do
        if timeout 2 python3 -c "
import redis
try:
    r = redis.StrictRedis(host='$host', port=$port, decode_responses=True)
    r.ping()
    print('✅ Redis is responding', flush=True)
    exit(0)
except:
    exit(1)
" 2>/dev/null; then
            echo "✅ Redis is ready after ${elapsed}s"
            return 0
        fi
        
        if [ $((elapsed % 5)) -eq 0 ] && [ $elapsed -gt 0 ]; then
            echo "  Still waiting... ($elapsed/${max_wait}s)"
        fi
        
        sleep 1
        elapsed=$((elapsed + 1))
    done
    
    echo "⚠️  Redis not responding after ${max_wait}s (optional - continuing)"
    return 1
}

wait_for_python_package() {
    local package=${1}
    local max_wait=${2:-30}
    local elapsed=0
    
    if [ -z "$package" ]; then
        echo "❌ Usage: wait_for_python_package <package_name>"
        return 1
    fi
    
    echo "⏳ Checking Python package '$package' (max ${max_wait}s)..."
    
    while [ $elapsed -lt $max_wait ]; do
        if python3 -c "import ${package}" 2>/dev/null; then
            echo "✅ Package '$package' is available"
            return 0
        fi
        
        if [ $elapsed -gt 5 ] && [ $elapsed -lt 10 ]; then
            echo "  Attempting to install $package..."
            pip install "${package}" >/dev/null 2>&1 && return 0
        fi
        
        sleep 1
        elapsed=$((elapsed + 1))
    done
    
    echo "⚠️  Package '$package' not available after ${max_wait}s"
    return 1
}

wait_for_autoware() {
    local max_wait=${1:-30}
    
    echo "⏳ Checking Autoware availability (max ${max_wait}s)..."
    
    . /opt/ros/*/setup.bash 2>/dev/null || true
    
    if [ -f "${PARENT_DIR}/autoware/install/setup.bash" ]; then
        echo "✅ Found compiled Autoware"
        source "${PARENT_DIR}/autoware/install/setup.bash" 2>/dev/null
        return 0
    fi
    
    if timeout 5 ros2 pkg list 2>/dev/null | grep -q "autoware_launch"; then
        echo "✅ Found system Autoware packages"
        return 0
    fi
    
    echo "⚠️  Autoware not found - will use fallback bridge"
    return 1
}

wait_for_dependency() {
    local term_name=${1}
    local condition=${2}
    local args=${@:3}
    
    case "$condition" in
        carla)
            wait_for_carla $args || return 1
            ;;
        carla_world)
            wait_for_carla_world $args || return 1
            ;;
        ros2)
            wait_for_ros2 $args || return 1
            ;;
        topic)
            wait_for_topic $args || return 1
            ;;
        redis)
            wait_for_redis $args || return 1
            ;;
        package)
            wait_for_python_package $args || return 1
            ;;
        autoware)
            wait_for_autoware $args || return 1
            ;;
        *)
            echo "❌ Unknown dependency condition: $condition"
            return 1
            ;;
    esac
    
    return 0
}

wait_for_terminal_2() {
    local map_name=${1:-McityMap_Main}
    
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "📍 Terminal 2: Load Mcity Map"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "Dependencies:"
    echo "  1. CARLA Server (Terminal 1) must be running"
    echo ""
    
    if ! wait_for_carla 60; then
        echo "⚠️  CARLA not responding but proceeding..."
    fi
    
    echo ""
}

wait_for_terminal_3() {
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "🚗 Terminal 3: Manual CAV Driver"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "Dependencies:"
    echo "  1. CARLA Server (Terminal 1) must be running"
    echo "  2. Map loaded (Terminal 2)"
    echo "  3. ROS2 environment must be ready"
    echo ""
    
    wait_for_carla 60
    wait_for_ros2 30
    
    echo ""
}

wait_for_terminal_4() {
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "📡 Terminal 4: Sensor Publisher"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "Dependencies:"
    echo "  1. CARLA Server (Terminal 1) must be running"
    echo "  2. Map loaded (Terminal 2)"
    echo "  3. ROS2 environment must be ready"
    echo ""
    
    wait_for_carla 60
    wait_for_ros2 30
    
    echo ""
}

wait_for_terminal_5() {
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "🔄 Terminal 5: TeraSim Co-simulation"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "Dependencies:"
    echo "  1. CARLA Server (Terminal 1) FULLY INITIALIZED"
    echo "     (may take 60-120+ seconds)"
    echo "  2. Redis server (for SUMO bridge)"
    echo ""
    
    if ! wait_for_carla 120 localhost 2000; then
        echo "⚠️  CARLA slow to initialize - will retry with backoff"
    fi
    
    if ! wait_for_redis 15 localhost 6379; then
        echo "💡 Redis optional - TerraSim can work without it"
    fi
    
    echo ""
}

wait_for_terminal_6() {
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "🛰️  Terminal 6: GNSS Decoder"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "Dependencies:"
    echo "  1. Sensor Publisher (Terminal 4) publishing /carla/gnss"
    echo "  2. ROS2 environment must be ready"
    echo ""
    
    wait_for_ros2 30
    
    wait_for_topic "/carla/gnss" 60
    
    echo ""
}

wait_for_terminal_7() {
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "🤖 Terminal 7: Autoware Planning Bridge"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "Dependencies:"
    echo "  1. CARLA Server (Terminal 1) must be running"
    echo "  2. ROS2 environment must be ready"
    echo "  3. Autoware packages (compiled or system) OPTIONAL"
    echo ""
    
    wait_for_carla 60
    wait_for_ros2 30
    wait_for_autoware 30 || true
    
    echo ""
}

wait_for_terminal_8() {
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "🎮 Terminal 8: Preview Control"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "Dependencies:"
    echo "  1. Autoware Bridge (Terminal 7) publishing /cmd_vel"
    echo "  2. ROS2 environment must be ready"
    echo ""
    
    wait_for_ros2 30
    wait_for_topic "/cmd_vel" 60
    
    echo ""
}

wait_for_terminal_9() {
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "🎬 Terminal 9: TeraSim Scenario"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "Dependencies:"
    echo "  1. TeraSim co-simulation (Terminal 5) running"
    echo "  2. Redis server (for SUMO bridge)"
    echo "  3. TeraSim Python packages"
    echo ""
    
    wait_for_redis 15 localhost 6379 || true
    wait_for_python_package "terasim" 30 || true
    
    echo ""
}

wait_for_terminal_10() {
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "📊 Terminal 10: RViz2 Visualization"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "Dependencies:"
    echo "  1. All ROS2 nodes publishing (Sensors, Bridge, GNSS)"
    echo "  2. ROS2 environment must be ready"
    echo "  3. rviz2 application must be installed"
    echo ""
    
    wait_for_ros2 30
    
    echo "⏳ Waiting for sensor topics to be published..."
    for topic in "/camera/camera_info" "/carla/lidar" "/gnss/fix" "/odom"; do
        wait_for_topic "$topic" 60 || echo "  ⚠️  Topic $topic may not be available"
    done
    
    echo ""
}

export -f wait_for_carla
export -f wait_for_carla_world
export -f wait_for_ros2
export -f wait_for_topic
export -f wait_for_redis
export -f wait_for_python_package
export -f wait_for_autoware
export -f wait_for_dependency

export -f wait_for_terminal_2
export -f wait_for_terminal_3
export -f wait_for_terminal_4
export -f wait_for_terminal_5
export -f wait_for_terminal_6
export -f wait_for_terminal_7
export -f wait_for_terminal_8
export -f wait_for_terminal_9
export -f wait_for_terminal_10
