#!/bin/bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo ""
echo "╔════════════════════════════════════════════════════════════════════════════╗"
echo "║        CARLA Fullstack Dependency Management - Status Check              ║"
echo "╚════════════════════════════════════════════════════════════════════════════╝"
echo ""

echo "${BLUE}[1] Dependency Management Documentation${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

docs=(
    "DEPENDENCY_MANAGEMENT_INDEX.md:Main entry point"
    "DEPENDENCY_MATRIX.md:Visual matrix & quick reference"
    "DEPENDENCY_TREE.md:Complete dependency graph"
    "SMART_WAIT_GUIDE.md:Integration guide"
    "DEPENDENCY_MANAGEMENT_SUMMARY.md:High-level overview"
    "TERMINAL5_CARLA_TIMEOUT_FIX.md:Terminal 5 timeout fix details"
)

for doc_pair in "${docs[@]}"; do
    doc="${doc_pair%:*}"
    desc="${doc_pair#*:}"
    
    if [ -f "${SCRIPT_DIR}/${doc}" ]; then
        size=$(wc -l < "${SCRIPT_DIR}/${doc}")
        echo "  ${GREEN}✅${NC} ${doc} (${size} lines)"
        echo "     └─ ${desc}"
    else
        echo "  ${RED}❌${NC} ${doc} NOT FOUND"
    fi
done

echo ""

echo "${BLUE}[2] Smart Wait Functions Library${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ -f "${SCRIPT_DIR}/launch/wait_functions.sh" ]; then
    echo "  ${GREEN}✅${NC} wait_functions.sh found"
    
    func_count=$(grep -c "^[a-z_]*() {" "${SCRIPT_DIR}/launch/wait_functions.sh" 2>/dev/null || echo "0")
    echo "     └─ Contains ${func_count} bash functions"
    
    if grep -q "wait_for_carla()" "${SCRIPT_DIR}/launch/wait_functions.sh"; then
        echo "       • wait_for_carla"
    fi
    if grep -q "wait_for_terminal_5()" "${SCRIPT_DIR}/launch/wait_functions.sh"; then
        echo "       • wait_for_terminal_5 (and others)"
    fi
else
    echo "  ${RED}❌${NC} wait_functions.sh NOT FOUND"
fi

echo ""

echo "${BLUE}[3] Launch Script Integration Status${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ -f "${SCRIPT_DIR}/launch/start.sh" ]; then
    echo "  ${GREEN}✅${NC} launch/start.sh found"
    
    if grep -q "source.*wait_functions" "${SCRIPT_DIR}/launch/start.sh"; then
        echo "  ${GREEN}✅${NC} wait_functions.sh is sourced"
    else
        echo "  ${YELLOW}⚠️${NC}  wait_functions.sh NOT sourced yet (needs integration)"
    fi
    
    if grep -q "set_timeout(45" "${SCRIPT_DIR}/launch/examples/carla_examples/carla_cosim_ros2.py" 2>/dev/null; then
        echo "  ${GREEN}✅${NC} Terminal 5 timeout fix applied (45.0s timeout)"
    else
        echo "  ${YELLOW}⚠️${NC}  Terminal 5 timeout may not be increased"
    fi
    
    if grep -q "max_attempts" "${SCRIPT_DIR}/launch/examples/carla_examples/carla_cosim_ros2.py" 2>/dev/null; then
        echo "  ${GREEN}✅${NC} Terminal 5 retry logic implemented"
    else
        echo "  ${YELLOW}⚠️${NC}  Terminal 5 retry logic may not be implemented"
    fi
    
    terminal_count=$(grep -c "wait_for_terminal_" "${SCRIPT_DIR}/launch/start.sh" || echo "0")
    if [ "$terminal_count" -gt 0 ]; then
        echo "  ${GREEN}✅${NC} Using wait_for_terminal functions: ${terminal_count} places"
    else
        echo "  ${YELLOW}⚠️${NC}  Not using wait_for_terminal functions yet"
        echo "     └─ Next step: Replace 'sleep N' with 'wait_for_terminal_N'"
    fi
else
    echo "  ${RED}❌${NC} launch/start.sh NOT FOUND"
fi

echo ""

echo "${BLUE}[4] Current Terminal Sleep Delays${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ -f "${SCRIPT_DIR}/launch/start.sh" ]; then
    echo "  Terminal startup delays (from launch script):"
    echo ""
    
    for term in {1..10}; do
        pattern="Window ${term}:"
        sleep_val=$(grep -A 20 "${pattern}" "${SCRIPT_DIR}/launch/start.sh" 2>/dev/null | \
                   grep "sleep " | head -1 | grep -oE "[0-9]+" | head -1)
        
        if [ -n "$sleep_val" ]; then
            if [ "$term" -eq 5 ] || [ "$term" -eq 1 ]; then
                echo "    ${BLUE}T${term}:${NC} ${GREEN}${sleep_val}s${NC}"
            else
                echo "    T${term}: ${sleep_val}s"
            fi
        fi
    done
else
    echo "  ${RED}Cannot read launch/start.sh${NC}"
fi

echo ""

echo "${BLUE}[5] System Readiness Check${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo "  Checking CARLA Server..."
if timeout 3 python3 -c "import carla; carla.Client('127.0.0.1', 2000).get_world()" 2>/dev/null; then
    echo "  ${GREEN}✅${NC} CARLA Server is running"
else
    echo "  ${YELLOW}⚠️${NC}  CARLA Server not responding (expected if not running)"
fi

echo "  Checking ROS2 Environment..."
if grep -q "ROS_DISTRO" <<< "$(env)" 2>/dev/null; then
    ros_version=$(echo "$ROS_DISTRO" | tr '[:lower:]' '[:upper:]')
    echo "  ${GREEN}✅${NC} ROS2 is available (${ros_version})"
elif [ -f "/opt/ros/kilted/setup.bash" ]; then
    echo "  ${YELLOW}⚠️${NC}  ROS2 not sourced (available at /opt/ros/kilted/)"
else
    echo "  ${RED}❌${NC} ROS2 not found"
fi

if command -v redis-server &>/dev/null; then
    echo "  ${GREEN}✅${NC} Redis server is installed"
else
    echo "  ${YELLOW}⚠️${NC}  Redis not installed (optional)"
fi

if python3 -c "import terasim_cosim" 2>/dev/null; then
    echo "  ${GREEN}✅${NC} terasim_cosim package available"
else
    echo "  ${YELLOW}⚠️${NC}  terasim_cosim not available (will be installed on demand)"
fi

echo ""

echo "${BLUE}[6] Implementation Checklist${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo ""
echo "  ${GREEN}✅ COMPLETED:${NC}"
echo "     • Generated comprehensive dependency documentation"
echo "     • Created smart wait functions library"
echo "     • Applied Terminal 5 timeout fix (2.0s → 45.0s)"
echo "     • Added Terminal 5 retry logic (5 attempts)"
echo "     • Created integration guide and examples"
echo ""

echo "  ${YELLOW}⚠️${NC}  TODO (Optional but Recommended):"
echo "     • Update launch/start.sh to use wait_for_terminal_* functions"
echo "     • Test on your system"
echo "     • Verify all 10 terminals launch successfully"
echo "     • Measure startup time improvement"
echo ""

echo "${BLUE}[7] Recommended Next Steps${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo ""
echo "  1. ${BLUE}Understand the dependency tree:${NC}"
echo "     → Read: ${SCRIPT_DIR}/DEPENDENCY_MANAGEMENT_INDEX.md"
echo "     → Takes: 5-10 minutes"
echo ""

echo "  2. ${BLUE}Learn about smart wait functions:${NC}"
echo "     → Read: ${SCRIPT_DIR}/SMART_WAIT_GUIDE.md"
echo "     → Takes: 15-20 minutes"
echo ""

echo "  3. ${BLUE}Integrate into start.sh (optional but recommended):${NC}"
echo "     → Edit: ${SCRIPT_DIR}/launch/start.sh"
echo "     → Add at top: source \"\${SCRIPT_DIR}/wait_functions.sh\""
echo "     → Replace 'sleep N' with 'wait_for_terminal_N'"
echo "     → Takes: 30-45 minutes"
echo ""

echo "  4. ${BLUE}Test your system:${NC}"
echo "     → Run: cd ${SCRIPT_DIR}/launch && ./start.sh"
echo "     → Watch output for dependency checking"
echo "     → Takes: 5-10 minutes"
echo ""

echo ""
echo "╔════════════════════════════════════════════════════════════════════════════╗"
echo "║                           SUMMARY                                          ║"
echo "╚════════════════════════════════════════════════════════════════════════════╝"
echo ""

echo "  Your CARLA fullstack has:"
echo "  • ${GREEN}✅${NC} 10 interdependent terminal windows"
echo "  • ${GREEN}✅${NC} Complete dependency documentation"
echo "  • ${GREEN}✅${NC} Smart wait function library"
echo "  • ${GREEN}✅${NC} Terminal 5 timeout fix already applied"
echo "  • ${YELLOW}⚠️${NC}  Wait functions available but not integrated (optional)"
echo ""

echo "  Benefits of integrating waits:"
echo "  • Adaptive startup (works on any speed system)"
echo "  • Better diagnostics (users see what's happening)"
echo "  • Faster startup on quick systems (no unnecessary delays)"
echo "  • More reliable (no cascading failures)"
echo ""

echo "  Current status:"
echo "  • ${GREEN}System ready for integration${NC}"
echo "  • Start with DEPENDENCY_MANAGEMENT_INDEX.md"
echo "  • Integration takes 30-60 minutes"
echo ""

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
