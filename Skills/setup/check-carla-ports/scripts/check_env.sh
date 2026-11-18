#!/usr/bin/env bash
# Reports whether this machine has the tools to enumerate listening sockets.
set -uo pipefail
# shellcheck disable=SC1091
. "$(dirname "${BASH_SOURCE[0]}")/../../../_common/env_common.sh"

fail=0

if command -v ss >/dev/null 2>&1; then
  echo "PASS  ss found ($(command -v ss)) — full port listing available"
elif command -v lsof >/dev/null 2>&1; then
  echo "WARN  ss not found; falling back to lsof. 'list' shows ports without addresses, 'check' works"
elif carla_is_windows && command -v netstat >/dev/null 2>&1; then
  echo "PASS  Windows: netstat + tasklist found — 'check', 'free' and 'list' work"
else
  echo "FAIL  none of ss, lsof (or netstat on Windows) found. Install iproute2 (ss) to use this skill"
  echo "      without them a busy port still reads 'in use (owner unknown)', never 'free'"
  fail=1
fi

echo "INFO  RPC base ${PORT:-${CARLA_PORT:-2000}}, TM ${TM_PORT:-${CARLA_TM_PORT:-8000}}, MCP ${MCP_PORT:-6256}"
echo "INFO  no CARLA checkout or running server needed"

exit "$fail"
