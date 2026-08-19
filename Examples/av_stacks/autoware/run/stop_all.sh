#!/usr/bin/env bash
#
# stop_all.sh -- tear down everything run_carla_autoware.sh started.
#
# Reads the pidfile written by run_carla_autoware.sh (<log-dir>/carla_autoware.pids,
# lines of "<name> <pid>") and kills ONLY those exact process groups, in reverse
# start order: SIGTERM first, SIGKILL after a grace period. Never pkills by name.
#
# Usage:
#   stop_all.sh                       # default pidfile: <this dir>/logs/carla_autoware.pids
#   stop_all.sh --log-dir DIR         # pidfile: DIR/carla_autoware.pids
#   stop_all.sh --pidfile FILE        # explicit pidfile
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PIDFILE="$SCRIPT_DIR/logs/carla_autoware.pids"
GRACE_SECONDS=15

usage() {
    grep '^#' "$0" | sed 's/^# \{0,1\}//' | sed -n '2,14p'
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --log-dir) PIDFILE="$2/carla_autoware.pids"; shift 2 ;;
        --pidfile) PIDFILE="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) echo "ERROR: unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
done

if [[ ! -f "$PIDFILE" ]]; then
    echo "[stop_all] no pidfile at '$PIDFILE' -- nothing to stop (already clean?)"
    exit 0
fi

lines=()
while IFS= read -r line; do
    [[ -n "$line" ]] && lines+=("$line")
done <"$PIDFILE"

if [[ ${#lines[@]} -eq 0 ]]; then
    echo "[stop_all] pidfile '$PIDFILE' is empty -- removing it"
    rm -f "$PIDFILE"
    exit 0
fi

echo "[stop_all] stopping ${#lines[@]} process group(s) from $PIDFILE (reverse start order)"

for ((idx=${#lines[@]}-1; idx>=0; idx--)); do
    name="${lines[$idx]%% *}"; pid="${lines[$idx]##* }"
    if kill -0 "$pid" 2>/dev/null; then
        echo "[stop_all]   SIGTERM -> '$name' (pgid $pid)"
        kill -TERM -- "-$pid" 2>/dev/null || kill -TERM "$pid" 2>/dev/null || true
    else
        echo "[stop_all]   '$name' (pid $pid) already gone"
    fi
done

waited=0
while ((waited < GRACE_SECONDS)); do
    alive=false
    for line in "${lines[@]}"; do
        pid="${line##* }"
        kill -0 "$pid" 2>/dev/null && alive=true
    done
    $alive || break
    sleep 1
    ((waited++)) || true
done

for ((idx=${#lines[@]}-1; idx>=0; idx--)); do
    name="${lines[$idx]%% *}"; pid="${lines[$idx]##* }"
    if kill -0 "$pid" 2>/dev/null; then
        echo "[stop_all]   SIGKILL -> '$name' (pgid $pid) [survived ${GRACE_SECONDS}s grace]"
        kill -KILL -- "-$pid" 2>/dev/null || kill -KILL "$pid" 2>/dev/null || true
    fi
done

rm -f "$PIDFILE"
echo "[stop_all] done. Logs remain in $(dirname "$PIDFILE")"
