#!/usr/bin/env bash
# Reports whether this machine has the tools to enumerate listening sockets.
set -uo pipefail

fail=0

if command -v ss >/dev/null 2>&1; then
  echo "PASS  ss found ($(command -v ss)) — full port listing available"
elif command -v lsof >/dev/null 2>&1; then
  echo "WARN  ss not found; falling back to lsof. 'list' is unavailable, 'check' works"
else
  echo "FAIL  neither ss nor lsof found. Install iproute2 (ss) to use this skill"
  fail=1
fi

echo "INFO  RPC base ${PORT:-${CARLA_PORT:-2000}}, TM ${TM_PORT:-${CARLA_TM_PORT:-8000}}, MCP ${MCP_PORT:-6256}"
echo "INFO  no CARLA checkout or running server needed"

exit "$fail"
