#!/bin/bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"
LOG_FILE="${PARENT_DIR}/carla_mcity.log"
TERMINAL_LAUNCH_MAP="${PARENT_DIR}/launch/configs/terminal_launch_map.json"

CARLA_DIR="${CARLA_ROOT:-$HOME/carla}"
CARLA_PORT="${CARLA_PORT:-2000}"
DEFAULT_MAP_NAME="${CARLA_MAP:-McityMap_Main}"
PYTHON_BIN="${PYTHON_BIN:-/usr/bin/python3}"
TERASIM_SCENARIO="${TERASIM_SCENARIO:-default_sumo}"
CARLA_VEHICLE_TYPE="${CARLA_VEHICLE_TYPE:-vehicle.lincoln.mkz_2020}"
DISPLAY="${DISPLAY:-:0}"
export DISPLAY
LAUNCH_MODE="gnome"
AUTO_PRE_STOP="${AUTO_PRE_STOP:-true}"

log() {
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

error() {
    echo "[ERROR] $1" | tee -a "$LOG_FILE" >&2
}

success() {
    echo "[SUCCESS] $1" | tee -a "$LOG_FILE"
}

usage() {
    cat <<USAGE
Usage: $0 [options]

Options:
  -g, --gnome-terminal   Launch with GNOME Terminal windows (default)
  -t, --terminator       Launch with Terminator tabs/panes
    --vehicle-type TYPE    Vehicle blueprint (default: vehicle.lincoln.mkz_2020)
                                                 Example: --vehicle-type vehicle.lincoln.mkz_2017
  -h, --help             Show this help

Config source:
  launch/configs/terminal_launch_map.json
USAGE
}

ensure_config_exists() {
    if [ ! -f "$TERMINAL_LAUNCH_MAP" ]; then
        error "Terminal launch map not found: $TERMINAL_LAUNCH_MAP"
        exit 1
    fi
}

run_pre_start_cleanup() {
    if [ "$AUTO_PRE_STOP" != "true" ]; then
        log "Skipping pre-start cleanup because AUTO_PRE_STOP=${AUTO_PRE_STOP}"
        return
    fi

    local stop_script="${SCRIPT_DIR}/stop.sh"
    if [ ! -f "$stop_script" ]; then
        error "Pre-start cleanup requested but stop script not found: $stop_script"
        exit 1
    fi

    log "Running pre-start cleanup"
    if ! bash "$stop_script" --force --quiet; then
        log "Pre-start cleanup needed fallback process cleanup"
        pkill -f "CarlaUE4-Linux-Shipping|CarlaUE4.sh" 2>/dev/null || true
        pkill -f "carla_av_ros2.py|carla_sensor_ros2.py|carla_cosim_ros2.py|gnss_decoder_fallback.py|planning_simulator_fallback.py" 2>/dev/null || true
        pkill -x "rviz2" 2>/dev/null || true
    fi
    success "Pre-start cleanup completed (best-effort)"
}

load_terminal_rows() {
    local mode="$1"

    /usr/bin/python3 - "$TERMINAL_LAUNCH_MAP" "$mode" "$PARENT_DIR" "$SCRIPT_DIR" "$CARLA_DIR" "$CARLA_PORT" "$DEFAULT_MAP_NAME" "$PYTHON_BIN" "$TERASIM_SCENARIO" "$CARLA_VEHICLE_TYPE" <<'PY'
import base64
import json
import sys

cfg_path = sys.argv[1]
mode = sys.argv[2]
parent_dir = sys.argv[3]
script_dir = sys.argv[4]
carla_dir = sys.argv[5]
carla_port = sys.argv[6]
default_map_name = sys.argv[7]
python_bin = sys.argv[8]
terasim_scenario = sys.argv[9]
carla_vehicle_type = sys.argv[10]

with open(cfg_path, "r", encoding="utf-8") as f:
    cfg = json.load(f)

tokens = {
    "__PARENT_DIR__": parent_dir,
    "__SCRIPT_DIR__": script_dir,
    "__CARLA_DIR__": carla_dir,
    "__CARLA_PORT__": carla_port,
    "__DEFAULT_MAP_NAME__": default_map_name,
    "__PYTHON_BIN__": python_bin,
    "__TERASIM_SCENARIO__": terasim_scenario,
    "__CARLA_VEHICLE_TYPE__": carla_vehicle_type,
}

for term in sorted(cfg.get("terminals", []), key=lambda t: t.get("id", 0)):
    if not term.get("modes", {}).get(mode, False):
        continue

    cmd = term.get("command", "")
    for key, value in tokens.items():
        cmd = cmd.replace(key, value)

    row = [
        str(term.get("id", 0)),
        term.get("title", ""),
        str(term.get("tab", 0)),
        str(term.get("pane", 0)),
        str(term.get("post_spawn_sleep", 0)),
        base64.b64encode(cmd.encode("utf-8")).decode("ascii"),
    ]
    print("\t".join(row))
PY
}

generate_terminator_layout_from_map() {
    local output_file="$1"

    /usr/bin/python3 - "$TERMINAL_LAUNCH_MAP" "$output_file" "$PARENT_DIR" "$SCRIPT_DIR" "$CARLA_DIR" "$CARLA_PORT" "$DEFAULT_MAP_NAME" "$PYTHON_BIN" "$TERASIM_SCENARIO" "$CARLA_VEHICLE_TYPE" <<'PY'
import json
import sys

cfg_path = sys.argv[1]
output_file = sys.argv[2]
parent_dir = sys.argv[3]
script_dir = sys.argv[4]
carla_dir = sys.argv[5]
carla_port = sys.argv[6]
default_map_name = sys.argv[7]
python_bin = sys.argv[8]
terasim_scenario = sys.argv[9]
carla_vehicle_type = sys.argv[10]

with open(cfg_path, "r", encoding="utf-8") as f:
    cfg = json.load(f)

tokens = {
    "__PARENT_DIR__": parent_dir,
    "__SCRIPT_DIR__": script_dir,
    "__CARLA_DIR__": carla_dir,
    "__CARLA_PORT__": carla_port,
    "__DEFAULT_MAP_NAME__": default_map_name,
    "__PYTHON_BIN__": python_bin,
    "__TERASIM_SCENARIO__": terasim_scenario,
    "__CARLA_VEHICLE_TYPE__": carla_vehicle_type,
}

terms = [t for t in cfg.get("terminals", []) if t.get("modes", {}).get("terminator", False)]
for term in terms:
    cmd = term.get("command", "")
    for key, value in tokens.items():
        cmd = cmd.replace(key, value)

    if cmd in {"bash", "htop", "nvtop"}:
        term["resolved_command"] = cmd
    else:
        escaped = cmd.replace("\\", "\\\\").replace('"', '\\"')
        term["resolved_command"] = f'bash -c "{escaped}; exec bash"'

tabs_cfg = {t.get("id"): t.get("label", f"Tab {t.get('id')}") for t in cfg.get("tabs", [])}
tab_ids = sorted({t.get("tab") for t in terms})
labels = [tabs_cfg.get(tab_id, f"Tab {tab_id}") for tab_id in tab_ids]

tabs = {tab_id: [] for tab_id in tab_ids}
for term in sorted(terms, key=lambda t: t.get("id", 0)):
    tabs[term.get("tab")].append(term)

for tab_id, tab_terms in tabs.items():
    if len(tab_terms) > 4:
        raise SystemExit(f"Tab {tab_id} has {len(tab_terms)} terminals; max supported is 4 per tab")

lines = []
lines.append("[global_config]")
lines.append("  window_state = maximise")
lines.append("  suppress_multiple_term_dialog = True")
lines.append("  tab_position = top")
lines.append("")
lines.append("[profiles]")
lines.append("  [[default]]")
lines.append("    scrollback_lines = 50000")
lines.append("    login_shell = True")
lines.append("")
lines.append("[layouts]")
lines.append("  [[CARLA_MCityFullStack]]")
lines.append("    [[[notebook0]]]")
lines.append("      active_page = 0")
lines.append(f"      labels = {', '.join(labels)}")
lines.append("      order = 0")
lines.append("      parent = window0")
lines.append("      type = Notebook")

tab_index_map = {tab_id: idx for idx, tab_id in enumerate(tab_ids, start=1)}
for tab_id in tab_ids:
    idx = tab_index_map[tab_id]
    tab_node = f"tab{idx}"
    left_node = f"tab{idx}_left"
    right_node = f"tab{idx}_right"

    lines.append(f"    [[[{tab_node}]]]")
    lines.append(f"      order = {idx - 1}")
    lines.append("      parent = notebook0")
    lines.append("      position = 632")
    lines.append("      type = HPaned")

    lines.append(f"    [[[{left_node}]]]")
    lines.append("      order = 0")
    lines.append(f"      parent = {tab_node}")
    lines.append("      position = 354")
    lines.append("      type = VPaned")

    lines.append(f"    [[[{right_node}]]]")
    lines.append("      order = 1")
    lines.append(f"      parent = {tab_node}")
    lines.append("      position = 354")
    lines.append("      type = VPaned")

    for term in sorted(tabs[tab_id], key=lambda t: t.get("pane", 0)):
        pane = int(term.get("pane", 1))
        if pane == 1:
            parent = left_node
            order = 0
        elif pane == 2:
            parent = left_node
            order = 1
        elif pane == 3:
            parent = right_node
            order = 0
        else:
            parent = right_node
            order = 1

        node_name = f"terminal_t{tab_id}_p{pane}"
        lines.append(f"    [[[{node_name}]]]")
        lines.append(f"      command = {term['resolved_command']}")
        lines.append(f"      order = {order}")
        lines.append(f"      parent = {parent}")
        lines.append("      profile = default")
        lines.append("      type = Terminal")
        lines.append(f"      title = {term.get('title', '')}")

    lines.append("")

lines.append("    [[[window0]]]")
lines.append("      order = 0")
lines.append("      parent = \"\"")
lines.append("      position = 0:25")
lines.append("      size = 1920, 1080")
lines.append("      type = Window")
lines.append("")
lines.append("[plugins]")

with open(output_file, "w", encoding="utf-8") as f:
    f.write("\n".join(lines) + "\n")
PY
}

launch_with_gnome_terminal() {
    log "Launching GNOME terminals from JSON config"

    if ! command -v gnome-terminal >/dev/null 2>&1; then
        error "gnome-terminal is not installed"
        exit 1
    fi

    local launched_count=0
    while IFS=$'\t' read -r term_id title _tab _pane post_spawn_sleep cmd_b64; do
        [ -z "$term_id" ] && continue
        local command
        command="$(printf '%s' "$cmd_b64" | base64 -d)"

        log "Window ${term_id}: ${title}"
        gnome-terminal --title="$title" -- bash -c "
            ${command}
            read -p 'Press Enter to close...'
        " &

        launched_count=$((launched_count + 1))
        sleep "${post_spawn_sleep}"
    done < <(load_terminal_rows gnome)

    success "Launched ${launched_count} GNOME windows from ${TERMINAL_LAUNCH_MAP}"
    wait
}

launch_with_terminator() {
    if ! command -v terminator >/dev/null 2>&1; then
        error "Terminator is not installed. Install with: sudo apt install terminator"
        error "Falling back to GNOME mode"
        launch_with_gnome_terminal
        return
    fi

    local config_file
    config_file="$(mktemp /tmp/carla_terminator_layout.XXXXXX)"

    if ! generate_terminator_layout_from_map "$config_file"; then
        error "Failed to generate Terminator layout from JSON"
        launch_with_gnome_terminal
        return
    fi

    log "Launching Terminator from JSON config"
    terminator -g "$config_file" -l CARLA_MCityFullStack &
    local terminator_pid=$!
    sleep 2

    if ! kill -0 "$terminator_pid" 2>/dev/null; then
        error "Terminator failed to start with generated layout"
        error "Falling back to GNOME mode"
        launch_with_gnome_terminal
        return
    fi

    success "Terminator launched with shared JSON layout config"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -g|--gnome-terminal)
            LAUNCH_MODE="gnome"
            shift
            ;;
        -t|--terminator)
            LAUNCH_MODE="terminator"
            shift
            ;;
        --vehicle-type)
            if [[ $# -lt 2 ]]; then
                error "--vehicle-type requires an argument"
                usage
                exit 1
            fi
            CARLA_VEHICLE_TYPE="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            error "Unknown argument: $1"
            usage
            exit 1
            ;;
    esac
done

ensure_config_exists
run_pre_start_cleanup

if [ "$LAUNCH_MODE" = "terminator" ]; then
    launch_with_terminator
else
    launch_with_gnome_terminal
fi
