#!/usr/bin/env bash
# Reports which CARLA-related TCP ports are in use on this machine and by what.
# Pure read: it never binds, kills or changes anything.
set -uo pipefail

# Recorded config (CARLA_PORT and friends) plus the shared port helpers.
# shellcheck disable=SC1091
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

RPC_PORT="${PORT:-${CARLA_PORT:-2000}}"
TM_PORT="${TM_PORT:-${CARLA_TM_PORT:-8000}}"
MCP_PORT="${MCP_PORT:-6256}"

usage() {
  cat <<'USAGE'
ports.sh [list|check|free PORT|role PORT]

  list          every listening TCP port this machine has, CARLA roles named
  check         only the CARLA-related ports, with their expected role    (default)
  free PORT     exit 0 if PORT is free, 1 if something listens on it
  role PORT     print what CARLA uses PORT for

Environment: PORT/CARLA_PORT (2000), TM_PORT/CARLA_TM_PORT (8000), MCP_PORT (6256).
USAGE
}

# Role of a port, given the configured bases. Echoes nothing for unknown ports.
role_of() {
  local p="$1"
  case "$p" in
    "$RPC_PORT")              echo "CARLA RPC server (-carla-rpc-port)" ;;
    "$((RPC_PORT + 1))")      echo "CARLA sensor streaming (RPC+1)" ;;
    "$((RPC_PORT + 2))")      echo "CARLA multi-GPU secondary router (RPC+2)" ;;
    "$TM_PORT")               echo "Traffic Manager RPC" ;;
    "$MCP_PORT")              echo "Unreal MCP server (editor only)" ;;
    8767)                     echo "Alpamayo 2 Super model service" ;;
    8010)                     echo "NVIDIA Cosmos example server" ;;
    7400|7401|7410|7411)      echo "ROS 2 / Fast DDS, domain 0" ;;
    1981|1989)                echo "UnrealTraceServer (engine, not CARLA)" ;;
    1985|8558)                echo "Unreal Zen Server (engine, not CARLA)" ;;
    *)                        echo "" ;;
  esac
}

# Who listens on a port: "name pid" or empty. ss, then lsof, then netstat on
# Windows, then a bare connect; see carla_port_listener in env_common.sh.
listener_of() {
  carla_port_listener "$1"
}

cmd_check() {
  local ports=("$RPC_PORT" "$((RPC_PORT + 1))" "$((RPC_PORT + 2))" "$TM_PORT" "$MCP_PORT" 8010 8767)
  printf '%-7s %-10s %-45s %s\n' PORT STATE ROLE LISTENER
  local p state who
  for p in "${ports[@]}"; do
    who="$(listener_of "$p")"
    [ -n "$who" ] && state="IN USE" || { state="free"; who="-"; }
    printf '%-7s %-10s %-45s %s\n' "$p" "$state" "$(role_of "$p")" "$who"
  done

  # A listener whose process cannot be the port's owner is the usual cause of a
  # bind error: UnrealEditor on the TM port means the MCP server took it first.
  local tm_who; tm_who="$(listener_of "$TM_PORT")"
  case "$tm_who" in
    UnrealEditor*)
      echo
      echo "CONFLICT: port $TM_PORT is the Traffic Manager default but is held by $tm_who."
      echo "  An editor built before the MCP port moved to $MCP_PORT still serves MCP on $TM_PORT."
      echo "  Restart that editor to pick up ServerPortNumber=$MCP_PORT, or pass --traffic-manager-port N."
      ;;
  esac

  echo
  echo "Reference: Docs/ref_ports.md"
}

# Every listening TCP port number, one per line, for machines without ss:
# lsof, or netstat on Windows (wildcard foreign address = listener; the state
# word is localized, so it is never matched).
listening_ports() {
  if command -v lsof >/dev/null 2>&1; then
    lsof -nP -iTCP -sTCP:LISTEN -Fn 2>/dev/null | sed -n 's/^n.*:\([0-9][0-9]*\)$/\1/p' | sort -un
  elif carla_is_windows && command -v netstat >/dev/null 2>&1; then
    { netstat -ano -p TCP; netstat -ano -p TCPv6; } 2>/dev/null | tr -d '\r' \
      | awk '($3 == "0.0.0.0:0" || $3 == "[::]:0") { n = split($2, a, ":"); print a[n] }' \
      | sort -un
  fi
}

cmd_list() {
  if ! command -v ss >/dev/null 2>&1; then
    local ports p r
    ports="$(listening_ports)"
    [ -n "$ports" ] || { echo "cannot enumerate listening ports: need ss (iproute2), lsof, or netstat on Windows" >&2; return 1; }
    printf '%-22s %-45s %s\n' PORT ROLE LISTENER
    for p in $ports; do
      r="$(role_of "$p")"
      printf '%-22s %-45s %s\n' "$p" "${r:--}" "$(listener_of "$p")"
    done
    return 0
  fi
  printf '%-22s %-45s %s\n' ADDRESS ROLE LISTENER
  local addr p r who
  while read -r addr who; do
    p="${addr##*:}"
    r="$(role_of "$p")"
    printf '%-22s %-45s %s\n' "$addr" "${r:--}" "$who"
  done < <(ss -ltnpH 2>/dev/null | awk '{
      addr=$4;
      who="-";
      if (match($0, /users:\(\("[^"]+/)) who=substr($0, RSTART+9, RLENGTH-9);
      if (match($0, /pid=[0-9]+/)) who=who" "substr($0, RSTART+4, RLENGTH-4);
      print addr, who
    }' | sort -t: -k2 -n)
}

case "${1:-check}" in
  check) cmd_check ;;
  list)  cmd_list ;;
  free)  [ $# -ge 2 ] || { usage >&2; exit 2; }
         if [ -n "$(listener_of "$2")" ]; then echo "$2 IN USE: $(listener_of "$2")"; exit 1
         else echo "$2 free"; fi ;;
  role)  [ $# -ge 2 ] || { usage >&2; exit 2; }
         r="$(role_of "$2")"; echo "${r:-no known CARLA role for port $2}" ;;
  -h|--help|help) usage ;;
  *)     usage >&2; exit 2 ;;
esac
