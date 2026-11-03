#!/usr/bin/env bash
# Prerequisite checks for load-map. Read-only, no sudo. Fast (short connect
# timeout) so the MCP check_prerequisites(name) tool never hangs.
# Exits non-zero ONLY on hard blockers: no `carla` module, or no reachable server.
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null

# All checks run inside one python process: the module import and the server
# ping are exactly the two hard blockers, and doing them in-process keeps it to a
# single interpreter start. A 4s client timeout is well under the tool's 120s cap
# yet enough for a healthy local server to answer get_server_version().
"${PYTHON}" - "$CARLA_HOST" "$CARLA_PORT" <<'PY'
import sys

def fail(msg): print(f"FAIL  {msg}")
def warn(msg): print(f"WARN  {msg}")
def ok(msg):   print(f"PASS  {msg}")

try:
    import carla
except Exception as e:
    fail(f"cannot import carla — run the install-python-api skill for this interpreter ({e})")
    print("\nprerequisites BLOCKED — resolve FAIL lines above")
    sys.exit(1)
ok("carla module importable")

host, port = sys.argv[1], int(sys.argv[2])
try:
    client = carla.Client(host, port)
    client.set_timeout(4.0)  # short: a healthy local server answers in <1s
    sv = client.get_server_version()
    cv = client.get_client_version()
    ok(f"server reachable at {host}:{port} (server {sv}, client {cv})")
    if sv != cv:
        warn(f"version mismatch: client {cv} != server {sv} — API calls may misbehave")
    try:
        current = client.get_world().get_map().name
        ok(f"current map: {current}")
    except Exception as e:
        warn(f"connected but could not read current map ({e})")
except Exception as e:
    fail(f"no CARLA server at {host}:{port} — start one first, or set CARLA_HOST/CARLA_PORT ({e})")
    print("\nprerequisites BLOCKED — resolve FAIL lines above")
    sys.exit(1)

print("\nprerequisites OK — hard blockers clear")
sys.exit(0)
PY
