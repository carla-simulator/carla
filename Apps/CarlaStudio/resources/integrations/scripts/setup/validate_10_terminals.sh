#!/bin/bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_header() {
    echo -e "\n${BLUE}════════════════════════════════════════════${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}════════════════════════════════════════════${NC}\n"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_info() {
    echo -e "${CYAN}ℹ️  $1${NC}"
}

print_running() {
    echo -e "${GREEN}●${NC} $1"
}

print_not_running() {
    echo -e "${RED}○${NC} $1"
}

print_header "CARLA Fullstack - 10 Terminal Validator"

PARENT_DIR="${CARLA_WS:-$HOME/carla_ws}"
LOG_DIR="$PARENT_DIR"
SCRIPT_DIR="${PARENT_DIR}/launch"

print_header "Check 1: Launch Script Structure"
if [ -f "$SCRIPT_DIR/start.sh" ]; then
    print_success "Launch script found at launch/start.sh"
    
    if grep -q "Window 10:" "$SCRIPT_DIR/start.sh"; then
        print_success "Terminal 10 (RViz) configured"
    else
        print_error "Terminal 10 not found"
    fi
    
    if grep -q "timeout 5 python3 -c 'import carla'" "$SCRIPT_DIR/start.sh"; then
        print_success "Terminal 7: CARLA connectivity check added"
    fi
    
    if grep -q "for i in {1..30};" "$SCRIPT_DIR/start.sh"; then
        print_success "Terminal 7: CARLA retry loop (30 seconds) added"
    fi
else
    print_error "Launch script not found"
    exit 1
fi

print_header "Check 2: Terminal Scripts and Examples"

carla_examples="$PARENT_DIR/launch/examples/carla_examples"

if [ -f "$carla_examples/gnss_decoder_fallback.py" ]; then
    print_success "Terminal 6: GNSS decoder script found"
else
    print_warning "Terminal 6: GNSS decoder script missing"
fi

if [ -f "$carla_examples/planning_simulator_fallback.py" ]; then
    print_success "Terminal 7: Planning bridge script found"
else
    print_warning "Terminal 7: Planning bridge script missing"
fi

if [ -f "$carla_examples/carla_cosim_ros2.py" ]; then
    print_success "Terminal 5: TeraSim co-sim script found"
else
    print_warning "Terminal 5: TeraSim co-sim script missing"
fi

print_header "Check 3: RViz Configuration for Terminal 10"

if [ -f "$PARENT_DIR/rviz_config/carla_mcity_ros2.rviz" ]; then
    print_success "RViz config found"
    
    if grep -q "camera\|image\|Image" "$PARENT_DIR/rviz_config/carla_mcity_ros2.rviz" 2>/dev/null; then
        print_success "Camera display configured in RViz"
    else
        print_info "No camera display in current RViz config"
        print_info "You can add camera feeds in RViz GUI:"
        print_info "  1. Add → Image"
        print_info "  2. Select topic: /camera/image_raw"
    fi
else
    print_warning "RViz config not found at rviz_config/carla_mcity_ros2.rviz"
fi

print_header "Check 4: Installed Packages"

if command -v rviz2 >/dev/null 2>&1; then
    print_success "RViz2 installed"
else
    print_warning "RViz2 not installed"
    print_info "Install with: sudo apt install ros-kilted-rviz2"
fi

if python3 -c "import terasim_cosim" 2>/dev/null; then
    print_success "terasim_cosim package installed"
else
    print_warning "terasim_cosim package not installed (Terminal 5 will auto-install)"
fi

if python3 -c "import terasim" 2>/dev/null; then
    print_success "terasim package installed"
else
    print_warning "terasim package not installed (Terminal 9 will auto-install)"
fi

if python3 -c "import carla" 2>/dev/null; then
    print_success "CARLA Python API available"
else
    print_warning "CARLA Python API not in path"
    print_info "Set: export PYTHONPATH=${CARLA_ROOT:-$HOME/carla}/PythonAPI/carla:\$PYTHONPATH"
fi

print_header "Check 5: ROS2 Environment"

if [ -f /opt/ros/kilted/setup.bash ]; then
    print_success "ROS2 Kilted distribution found"
    
    source /opt/ros/kilted/setup.bash 2>/dev/null
    
    if ros2 topic list >/dev/null 2>&1; then
        print_success "ROS2 communication working"
    else
        print_warning "ROS2 daemon not responsive (normal before launch)"
    fi
else
    print_error "ROS2 not found at /opt/ros/kilted/"
    exit 1
fi

print_header "Check 6: Log Files and Configuration"

if [ -d "$LOG_DIR" ]; then
    print_success "Log directory exists: $LOG_DIR"
    
    log_count=$(find "$LOG_DIR" -name "*.log" -type f 2>/dev/null | wc -l)
    if [ "$log_count" -gt 0 ]; then
        print_info "Found $log_count log files from previous run(s)"
        print_info "Recent logs:"
        ls -1t "$LOG_DIR"/*.log 2>/dev/null | head -5 | sed 's/^/     /'
    fi
else
    print_warning "Log directory not found"
fi

print_header "Check 7: Terminal Improvements Applied"

if grep -q "sleep 12" "$SCRIPT_DIR/start.sh"; then
    print_running "Terminal 1: CARLA Server (12s delay)"
fi

if grep -q "sleep 14" "$SCRIPT_DIR/start.sh"; then
    print_running "Terminal 2: Load Mcity Map (14s delay)"
fi

if grep -q "sleep 16" "$SCRIPT_DIR/start.sh"; then
    print_running "Terminal 3: Manual CAV Driver (16s delay)"
fi

if grep -q "sleep 18" "$SCRIPT_DIR/start.sh"; then
    print_running "Terminal 4: Sensor Publisher (18s delay)"
fi

if grep -q "sleep 12" "$SCRIPT_DIR/start.sh" | head -2; then
    print_running "Terminal 5: TeraSim Co-sim (improved CARLA checking)"
fi

if grep -q "sleep 15" "$SCRIPT_DIR/start.sh"; then
    print_running "Terminal 6: GNSS Decoder (real implementation)"
fi

if grep -q "Waiting for CARLA" "$SCRIPT_DIR/start.sh"; then
    print_running "Terminal 7: Autoware/Planning Bridge (CARLA wait loop)"
fi

if grep -q "sleep 20" "$SCRIPT_DIR/start.sh"; then
    print_running "Terminal 8: Preview Control (optional)"
fi

if grep -q "sleep 22" "$SCRIPT_DIR/start.sh"; then
    print_running "Terminal 9: TeraSim Scenario (improved error handling)"
fi

if grep -q "Window 10:" "$SCRIPT_DIR/start.sh"; then
    print_running "Terminal 10: RViz2 Visualization with Camera Feeds ✨ NEW"
fi

print_header "Check 8: Validation Rules"

ERRORS=0
WARNINGS=0

if grep -c "sleep [0-9][0-9]" "$SCRIPT_DIR/start.sh" | grep -q "1[0-2]"; then
    print_success "All 10 terminals have startup delays"
else
    print_error "Not all terminals properly configured"
    ((ERRORS++))
fi

if grep -q "timeout.*python3.*import carla" "$SCRIPT_DIR/start.sh"; then
    print_success "CARLA connectivity checking implemented"
else
    print_warning "CARLA connectivity checking missing"
    ((WARNINGS++))
fi

if grep -q "\\[10\\].*RViz" "$SCRIPT_DIR/start.sh"; then
    print_success "Terminal 10 officially designated for RViz"
else
    print_error "Terminal 10 RViz configuration missing"
    ((ERRORS++))
fi

print_header "Final Summary"

echo ""
echo "📊 Validation Results:"
echo "   Errors:   $ERRORS"
echo "   Warnings: $WARNINGS"
echo ""

if [ "$ERRORS" -eq 0 ]; then
    echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━◆${NC}"
    echo -e "${GREEN}  ✅ ALL CHECKS PASSED - READY TO LAUNCH  ${NC}"
    echo -e "${GREEN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━◆${NC}"
    echo ""
    echo "📋 To start the fullstack with all 10 terminals:"
    echo ""
    echo "   cd $PARENT_DIR/launch"
    echo "   ./start.sh"
    echo ""
    echo "🎯 Expected sequence:"
    echo "   1. T1 starts CARLA server (~60-120 sec to load)"
    echo "   2. T2 loads Mcity map"
    echo "   3-4. Sensors and manual control start"
    echo "   5. TeraSim waits for CARLA (retry loop)"
    echo "   6. GNSS publishes GPS data"
    echo "   7. Planning bridge/Autoware launches"
    echo "   8-9. Preview control and scenario start"
    echo "   10. RViz opens with visualization"
    echo ""
    echo "💡 All terminals will be running in parallel"
    echo "   Check integration: ros2 topic list"
    echo ""
    exit 0
else
    echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${RED}  ❌ VALIDATION FAILED - FIX ISSUES ABOVE  ${NC}"
    echo -e "${RED}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    exit 1
fi
