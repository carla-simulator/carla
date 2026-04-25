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

KEEP_ALIVE_INTERVAL=${KEEP_ALIVE_INTERVAL:-60}
DISABLE_SYSTEM_SLEEP=${DISABLE_SYSTEM_SLEEP:-true}
VERBOSE=${VERBOSE:-false}

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log() {
    echo -e "${BLUE}[CARLA-LAUNCHER]${NC} $*"
}

error() {
    echo -e "${RED}[ERROR]${NC} $*" >&2
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $*"
}

warning() {
    echo -e "${YELLOW}[WARNING]${NC} $*"
}

cleanup() {
    log "Cleaning up..."
    
    if [ -n "$KEEP_ALIVE_PID" ] && kill -0 "$KEEP_ALIVE_PID" 2>/dev/null; then
        log "Stopping keep_alive script..."
        kill "$KEEP_ALIVE_PID" 2>/dev/null || true
        wait "$KEEP_ALIVE_PID" 2>/dev/null || true
    fi
    
    if [ "$DISABLE_SYSTEM_SLEEP" = true ]; then
        log "Re-enabling system sleep..."
        "$SCRIPT_DIR/manage_sleep.sh" enable 2>/dev/null || \
            warning "Could not re-enable system sleep"
    fi
    
    success "Cleanup complete"
}

trap cleanup EXIT INT TERM

validate_scripts() {
    if [ ! -f "$SCRIPT_DIR/keep_alive.sh" ]; then
        error "keep_alive.sh not found in $SCRIPT_DIR"
        return 1
    fi
    
    if [ ! -f "$SCRIPT_DIR/manage_sleep.sh" ]; then
        error "manage_sleep.sh not found in $SCRIPT_DIR"
        return 1
    fi
    
    return 0
}

start_keep_alive() {
    log "Starting keep-alive services..."
    log "  - Keep alive interval: ${KEEP_ALIVE_INTERVAL}s"
    log "  - Disable system sleep: ${DISABLE_SYSTEM_SLEEP}"
    
    if [ "$DISABLE_SYSTEM_SLEEP" = true ]; then
        "$SCRIPT_DIR/manage_sleep.sh" disable || \
            warning "Could not disable system sleep (may require sudo)"
    fi
    
    "$SCRIPT_DIR/keep_alive.sh" "$KEEP_ALIVE_INTERVAL" 0 &
    KEEP_ALIVE_PID=$!
    
    if [ "$VERBOSE" = true ]; then
        log "keep_alive process started with PID: $KEEP_ALIVE_PID"
    fi
    
    success "Keep-alive services started"
}

show_status() {
    log "════════════════════════════════════════════════════════════════"
    log "CARLA Keep-Alive Status"
    log "════════════════════════════════════════════════════════════════"
    
    if [ -n "$KEEP_ALIVE_PID" ] && kill -0 "$KEEP_ALIVE_PID" 2>/dev/null; then
        success "Keep-alive script is running (PID: $KEEP_ALIVE_PID)"
    else
        warning "Keep-alive script is not running"
    fi
    
    if command -v caffeine &> /dev/null; then
        if pgrep -x "caffeine" > /dev/null 2>&1; then
            success "Caffeine is running"
        else
            warning "Caffeine is installed but not running"
        fi
    else
        warning "Caffeine is not installed"
    fi
    
    "$SCRIPT_DIR/manage_sleep.sh" status
    log "════════════════════════════════════════════════════════════════"
}

usage() {
    cat << EOF
CARLA Launcher with Keep-Alive Support

USAGE: $(basename "$0") [OPTIONS]

OPTIONS:
    -h, --help              Show this help message
    -s, --status            Show keep-alive status
    -i, --interval SECS     Keep-alive interval in seconds (default: 60)
    --no-disable-sleep      Don't disable system sleep
    --disable-sleep         Disable system sleep (default: enabled)
    -v, --verbose           Enable verbose output

ENVIRONMENT VARIABLES:
    KEEP_ALIVE_INTERVAL     Set keep-alive interval (default: 60)
    DISABLE_SYSTEM_SLEEP    Set to 'false' to skip system sleep disable (default: true)
    VERBOSE                 Set to 'true' for verbose output (default: false)

EXAMPLES:
    # Run with default settings
    $(basename "$0")
    
    # Check status without starting services
    $(basename "$0") --status
    
    # Start with custom interval
    $(basename "$0") --interval 30
    
    # Don't disable system sleep
    $(basename "$0") --no-disable-sleep
    
    # Run with verbose output
    $(basename "$0") --verbose

EOF
}

main() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                usage
                exit 0
                ;;
            -s|--status)
                validate_scripts || exit 1
                show_status
                exit 0
                ;;
            -i|--interval)
                KEEP_ALIVE_INTERVAL="$2"
                shift 2
                ;;
            --no-disable-sleep)
                DISABLE_SYSTEM_SLEEP=false
                shift
                ;;
            --disable-sleep)
                DISABLE_SYSTEM_SLEEP=true
                shift
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            *)
                error "Unknown option: $1"
                usage
                exit 1
                ;;
        esac
    done
    
    validate_scripts || exit 1
    
    log "════════════════════════════════════════════════════════════════"
    log "CARLA Launcher with Keep-Alive Support"
    log "════════════════════════════════════════════════════════════════"
    log "Press Ctrl+C to stop"
    log ""
    
    start_keep_alive
    log ""
    
    show_status
    log ""
    log "Ready to run CARLA. Keep-alive services are active."
    log "Services will be automatically stopped when this launcher exits."
    log ""
    
    log "Press Ctrl+C to stop all services and exit..."
    while true; do
        sleep 10
    done
}

main "$@"
