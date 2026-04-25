#!/bin/bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"
PID_FILE="${PARENT_DIR}/carla_mcity.pid"
LOG_FILE="${PARENT_DIR}/carla_mcity.log"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log() {
    echo -e "${BLUE}[$(date +'%Y-%m-%d %H:%M:%S')]${NC} $1" | tee -a "$LOG_FILE"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1" | tee -a "$LOG_FILE"
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1" | tee -a "$LOG_FILE"
}

warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1" | tee -a "$LOG_FILE"
}

check_docker_container() {
    if docker ps -q -f name=carla-mcity-server | grep -q .; then
        return 0
    else
        return 1
    fi
}

check_local_process() {
    if [ -f "$PID_FILE" ] && [ -s "$PID_FILE" ]; then
        local pid=$(cat "$PID_FILE")
        if ps -p "$pid" > /dev/null 2>&1; then
            return 0
        fi
    fi

    if pgrep -f "CarlaUE4-Linux-Shipping|CarlaUE4.sh" >/dev/null 2>&1; then
        return 0
    fi

    return 1
}

stop_docker_containers() {
    log "Stopping Docker containers..."
    
    if check_docker_container; then
        log "Stopping CARLA Docker container..."
        docker stop carla-mcity-server 2>/dev/null || true
        docker rm carla-mcity-server 2>/dev/null || true
        success "CARLA Docker container stopped"
    fi
    
    local containers=$(docker ps -q --filter "ancestor=carlasim/carla")
    if [ ! -z "$containers" ]; then
        log "Stopping additional CARLA containers..."
        echo "$containers" | xargs -r docker stop
        echo "$containers" | xargs -r docker rm
        success "Additional CARLA containers stopped"
    fi
}

stop_local_processes() {
    log "Stopping local processes..."
    
    if [ -f "$PID_FILE" ] && [ -s "$PID_FILE" ]; then
        while IFS= read -r pid; do
            if [ ! -z "$pid" ] && ps -p "$pid" > /dev/null 2>&1; then
                log "Stopping process PID: $pid"
                
                kill -TERM "$pid" 2>/dev/null || true
                
                sleep 3
                
                if ps -p "$pid" > /dev/null 2>&1; then
                    warning "Force killing process PID: $pid"
                    kill -KILL "$pid" 2>/dev/null || true
                fi
                
                success "Process $pid stopped"
            fi
        done < "$PID_FILE"
    fi
    
    pkill -f "CarlaUE4" 2>/dev/null || true
    pkill -f "carla" 2>/dev/null || true
}

stop_terasim() {
    log "Stopping TeraSim processes..."
    
    if [ -f "${SCRIPT_DIR}/TeraSim-Mcity-2.0/terasim_stop.sh" ]; then
        log "Running TeraSim stop script..."
        cd "${SCRIPT_DIR}/TeraSim-Mcity-2.0"
        ./terasim_stop.sh 2>/dev/null || true
        cd "$SCRIPT_DIR"
    fi
    
    pkill -f "terasim" 2>/dev/null || true
    pkill -f "TeraSim" 2>/dev/null || true
    
    success "TeraSim processes stopped"
}

stop_ros2() {
    log "Stopping ROS2 processes..."
    
    pkill -f "ros2" 2>/dev/null || true
    pkill -f "rviz" 2>/dev/null || true
    pkill -f "carla_av_ros2.py|carla_sensor_ros2.py|carla_cosim_ros2.py" 2>/dev/null || true
    pkill -f "gnss_decoder_fallback.py|planning_simulator_fallback.py" 2>/dev/null || true
    
    if command -v ros2 &> /dev/null; then
        ros2 daemon stop >/dev/null 2>&1 || true
    fi
    
    success "ROS2 processes stopped"
}

stop_sumo_and_visualization() {
    log "Stopping SUMO and visualization processes..."

    pkill -x "rviz2" 2>/dev/null || true
    pkill -f "sumo-gui|sumo |sumo$|libsumo|traci" 2>/dev/null || true
    pkill -f "default_sumo_example.py|safetest_nade_example.py|pedestrian_example.py|cyclist_example.py|construction_example.py|multi_cav_example.py" 2>/dev/null || true
    pkill -f "python3 .*terasim_examples/.*_example.py|python3 .*_example.py" 2>/dev/null || true
    pkill -f "terminal9_scenario_runner.sh" 2>/dev/null || true

    success "SUMO and visualization processes stopped"
}

cleanup_network() {
    log "Cleaning up network resources..."
    
    if docker network ls | grep -q "carla-net"; then
        local network_containers=$(docker network inspect carla-net --format='{{range .Containers}}{{.Name}} {{end}}' 2>/dev/null || true)
        if [ -z "$network_containers" ]; then
            docker network rm carla-net 2>/dev/null || true
            success "Removed empty Docker network"
        else
            warning "Docker network still has containers, not removing"
        fi
    fi
}

cleanup_files() {
    log "Cleaning up temporary files..."
    
    if [ -f "$PID_FILE" ]; then
        rm -f "$PID_FILE"
        success "Removed PID file"
    fi
    
    find "$SCRIPT_DIR" -name "*.tmp" -delete 2>/dev/null || true
    find "$SCRIPT_DIR" -name "carla_*.png" -mtime +1 -delete 2>/dev/null || true
    
    success "Temporary files cleaned up"
}

cleanup_old_scripts() {
    log "Starting comprehensive cleanup..."
    
    if [ "$CREATE_BACKUP" = "true" ]; then
        create_cleanup_backup
    fi
    
    if [ "$DRY_RUN" = "true" ]; then
        warning "DRY RUN MODE - No files will be deleted"
        show_cleanup_list
        return 0
    fi
    
    if [ "$QUIET" != "true" ]; then
        show_cleanup_list
        
        if [ "$FORCE_STOP" != "true" ]; then
            echo ""
            read -p "Do you want to proceed with cleanup? (y/N): " -n 1 -r
            echo ""
            if [[ ! $REPLY =~ ^[Yy]$ ]]; then
                log "Cleanup cancelled by user"
                return 0
            fi
        fi
    fi
    
    log "Performing cleanup..."
    local removed_count=0
    
    local empty_scripts=(
        "${PARENT_DIR}/TeraSim-Mcity-2.0/terasim_launch.sh"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/setup_carla_0916.sh"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/setup_env.sh"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/terasim_status.sh"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/install.sh"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/test_ros2_sensors.sh"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/info.sh"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/terasim_stop.sh"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/start_simple.sh"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/test_pygame_rviz.sh"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/stop.sh"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/start.sh"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/setup_launcher.sh"
    )
    
    for script in "${empty_scripts[@]}"; do
        if [ -f "$script" ]; then
            rm -f "$script"
            log "Removed: ${script#$SCRIPT_DIR/}"
            removed_count=$((removed_count + 1))
        fi
    done
    
    if [ -d "$PARENT_DIR/TeraSim-Mcity-2.0" ]; then
        while IFS= read -r -d '' file; do
            rm -f "$file"
            log "Removed empty Python: ${file#$PARENT_DIR/}"
            removed_count=$((removed_count + 1))
        done < <(find "$PARENT_DIR/TeraSim-Mcity-2.0" -name "*.py" -empty -print0 2>/dev/null)
    fi
    
    if [ -d "$PARENT_DIR/mcity-digital-twin" ]; then
        while IFS= read -r -d '' file; do
            rm -f "$file"
            log "Removed empty Python: ${file#$PARENT_DIR/}"
            removed_count=$((removed_count + 1))
        done < <(find "$PARENT_DIR/mcity-digital-twin" -name "*.py" -empty -print0 2>/dev/null)
    fi
    
    if [ -d "$PARENT_DIR/TeraSim-Mcity-2.0/source_code" ]; then
        while IFS= read -r -d '' file; do
            rm -f "$file"
            log "Removed empty make.sh: ${file#$PARENT_DIR/}"
            removed_count=$((removed_count + 1))
        done < <(find "$PARENT_DIR/TeraSim-Mcity-2.0/source_code" -name "make.sh" -empty -print0 2>/dev/null)
    fi
    
    while IFS= read -r -d '' file; do
        rm -f "$file"
        log "Removed temp file: ${file#$PARENT_DIR/}"
        removed_count=$((removed_count + 1))
    done < <(find "$PARENT_DIR" -name "*.tmp" -print0 2>/dev/null)
    
    local safe_dirs=(
        "${PARENT_DIR}/TeraSim-Mcity-2.0/examples/terasim_examples/envs"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/examples/terasim_examples"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/examples/lidar_perception"
        "${PARENT_DIR}/TeraSim-Mcity-2.0/examples/carla_examples"
    )
    
    for dir in "${safe_dirs[@]}"; do
        if [ -d "$dir" ] && [ -z "$(ls -A "$dir" 2>/dev/null)" ]; then
            rmdir "$dir" 2>/dev/null && {
                log "Removed empty directory: ${dir#$PARENT_DIR/}"
                removed_count=$((removed_count + 1))
            } || true
        fi
    done
    
    success "Cleaned up $removed_count files and directories"
}

force_cleanup() {
    warning "Performing force cleanup..."
    
    pkill -9 -f "CarlaUE4" 2>/dev/null || true
    pkill -9 -f "carla" 2>/dev/null || true
    pkill -9 -f "terasim" 2>/dev/null || true
    pkill -9 -f "TeraSim" 2>/dev/null || true
    pkill -9 -x "rviz2" 2>/dev/null || true
    pkill -9 -f "sumo-gui|sumo |sumo$|libsumo|traci" 2>/dev/null || true
    pkill -9 -f "default_sumo_example.py|safetest_nade_example.py|pedestrian_example.py|cyclist_example.py|construction_example.py|multi_cav_example.py" 2>/dev/null || true
    pkill -9 -f "carla_av_ros2.py|carla_sensor_ros2.py|carla_cosim_ros2.py" 2>/dev/null || true
    pkill -9 -f "gnss_decoder_fallback.py|planning_simulator_fallback.py" 2>/dev/null || true
    pkill -9 -f "python3 .*terasim_examples/.*_example.py|python3 .*_example.py" 2>/dev/null || true
    pkill -9 -f "terminal9_scenario_runner.sh" 2>/dev/null || true
    
    docker ps -a --filter "ancestor=carlasim/carla" -q | xargs -r docker rm -f 2>/dev/null || true
    
    docker network rm carla-net 2>/dev/null || true
    
    success "Force cleanup completed"
}

display_shutdown_status() {
    echo ""
    echo "=================================================="
    echo "🛑 CARLA Mcity Environment Shutdown Complete"
    echo "=================================================="
    echo ""
    echo "✅ All processes stopped"
    echo "✅ Resources cleaned up"
    echo "✅ Environment ready for restart"
    echo ""
    echo "To restart: ./start.sh"
    echo "=================================================="
}

main() {
    echo "🛑 Stopping CARLA Mcity Environment"
    echo "=================================="
    
    echo "CARLA Mcity shutdown log - $(date)" >> "$LOG_FILE"
    
    local docker_running=false
    local local_running=false
    
    if check_docker_container; then
        docker_running=true
        log "Docker container detected"
    fi
    
    if check_local_process; then
        local_running=true
        log "Local processes detected"
    fi
    
    if [ "$docker_running" = false ] && [ "$local_running" = false ]; then
        warning "No CARLA processes detected; continuing cleanup for ROS2/RViz/SUMO/TeraSim"
    fi
    
    if [ "$docker_running" = true ]; then
        stop_docker_containers
    fi
    
    if [ "$local_running" = true ]; then
        stop_local_processes
    fi
    
    stop_terasim
    stop_ros2
    stop_sumo_and_visualization
    
    cleanup_network
    cleanup_files
    
    if [ "$CLEANUP_OLD_SCRIPTS" = "true" ]; then
        cleanup_old_scripts
    fi
    
    sleep 2
    
    if check_docker_container || check_local_process; then
        if [ "$FORCE_STOP" = "true" ]; then
            force_cleanup
        else
            error "Some processes may still be running"
            warning "Use --force to perform aggressive cleanup"
            exit 1
        fi
    fi
    
    success "CARLA Mcity environment stopped successfully"
    
    if [ "$CLEANUP_OLD_SCRIPTS" = "true" ]; then
        show_active_scripts
        echo ""
        success "✨ Cleanup completed! Your environment is now consolidated."
    else
        display_shutdown_status
    fi
}

create_cleanup_backup() {
    if [ "$CREATE_BACKUP" = "true" ]; then
        local backup_dir="${SCRIPT_DIR}/backup_$(date +%Y%m%d_%H%M%S)"
        log "Creating backup at $backup_dir..."
        mkdir -p "$backup_dir"
        
        local empty_scripts=(
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/terasim_launch.sh"
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/setup_carla_0916.sh"
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/setup_env.sh"
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/terasim_status.sh"
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/install.sh"
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/test_ros2_sensors.sh"
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/info.sh"
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/terasim_stop.sh"
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/start_simple.sh"
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/test_pygame_rviz.sh"
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/stop.sh"
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/start.sh"
            "${SCRIPT_DIR}/TeraSim-Mcity-2.0/setup_launcher.sh"
        )
        
        for script in "${empty_scripts[@]}"; do
            if [ -f "$script" ]; then
                local rel_path=$(realpath --relative-to="$SCRIPT_DIR" "$script" 2>/dev/null || echo "${script#$SCRIPT_DIR/}")
                local backup_file="$backup_dir/$rel_path"
                mkdir -p "$(dirname "$backup_file")" 
                cp "$script" "$backup_file" 2>/dev/null || true
            fi
        done
        
        success "Backup created at $backup_dir"
    fi
}

show_cleanup_list() {
    echo "Files to be cleaned up:"
    echo "======================="
    
    local count=0
    
    echo "Scripts and files:"
    local empty_scripts=(
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/terasim_launch.sh"
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/setup_carla_0916.sh"
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/setup_env.sh"
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/terasim_status.sh"
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/install.sh"
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/test_ros2_sensors.sh"
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/info.sh"
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/terasim_stop.sh"
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/start_simple.sh"
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/test_pygame_rviz.sh"
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/stop.sh"
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/start.sh"
        "${SCRIPT_DIR}/TeraSim-Mcity-2.0/setup_launcher.sh"
    )
    
    for script in "${empty_scripts[@]}"; do
        if [ -f "$script" ]; then
            local size=$(stat -c%s "$script" 2>/dev/null || echo "0")
            echo "  - ${script#$SCRIPT_DIR/} (${size} bytes)"
            count=$((count + 1))
        fi
    done
    
    echo ""
    echo "Empty Python Files:"
    local python_count=0
    if [ -d "$SCRIPT_DIR/TeraSim-Mcity-2.0" ]; then
        python_count=$(find "$SCRIPT_DIR/TeraSim-Mcity-2.0" -name "*.py" -empty 2>/dev/null | wc -l)
    fi
    if [ -d "$SCRIPT_DIR/mcity-digital-twin" ]; then
        local mcity_count=$(find "$SCRIPT_DIR/mcity-digital-twin" -name "*.py" -empty 2>/dev/null | wc -l)
        python_count=$((python_count + mcity_count))
    fi
    echo "  - $python_count empty Python files"
    count=$((count + python_count))
    
    echo ""
    echo "Temporary files:"
    local temp_count=$(find "$SCRIPT_DIR" -name "*.tmp" 2>/dev/null | wc -l)
    echo "  - $temp_count temporary files"
    count=$((count + temp_count))
    
    echo ""
    echo "Total files to clean: $count"
    echo "======================="
}

show_active_scripts() {
    echo ""
    echo "Active Scripts After Cleanup:"
    echo "============================="
    echo "Main Scripts:"
    echo "  ✅ start.sh          - Complete CARLA Mcity environment (all-in-one)"
    echo "  ✅ stop.sh           - Stop CARLA Mcity environment with cleanup"
    echo ""
    echo "Utility Scripts:"
    if [ -d "${SCRIPT_DIR}/run" ]; then
        find "${SCRIPT_DIR}/run" -name "*.sh" -type f | while read script; do
            echo "  ✅ $(basename "$script")         - $(grep '^# ' "$script" | head -1 | cut -c3-)"
        done
    fi
    echo ""
    echo "Usage:"
    echo "  ./start.sh           - Start the complete environment (local mode)"
    echo "  ./start.sh --docker  - Start with Docker mode"
    echo "  ./stop.sh            - Normal shutdown"
    echo "  ./stop.sh --cleanup  - Stop with full cleanup"
    echo "  ./stop.sh --force    - Force stop with aggressive cleanup"
    echo "============================="
}

show_help() {
    cat << EOF
CARLA Mcity Environment Stopper

Usage: $0 [OPTIONS]

Options:
    --force, -f           Force stop all processes aggressively
    --cleanup, -c         Also cleanup old/empty scripts after stopping
    --dry-run, -n         Show what would be cleaned without actually cleaning
    --backup, -b          Create backup before cleanup
    --quiet, -q           Suppress output except errors
    --help, -h            Show this help message

Examples:
    $0                    Normal shutdown
    $0 --force            Force shutdown (aggressive cleanup)
    $0 --cleanup          Stop and cleanup old files
    $0 --cleanup --dry-run Preview what cleanup would do
    $0 --cleanup --backup Cleanup with backup

EOF
}

FORCE_STOP=false
QUIET=false
CLEANUP_OLD_SCRIPTS=false
DRY_RUN=false
CREATE_BACKUP=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --force|-f)
            FORCE_STOP=true
            shift
            ;;
        --cleanup|-c)
            CLEANUP_OLD_SCRIPTS=true
            shift
            ;;
        --dry-run|-n)
            DRY_RUN=true
            CLEANUP_OLD_SCRIPTS=true
            shift
            ;;
        --backup|-b)
            CREATE_BACKUP=true
            CLEANUP_OLD_SCRIPTS=true
            shift
            ;;
        --quiet|-q)
            QUIET=true
            shift
            ;;
        --help|-h)
            show_help
            exit 0
            ;;
        *)
            error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

if [ "$QUIET" = "true" ]; then
    exec >/dev/null 2>&1
fi

main
