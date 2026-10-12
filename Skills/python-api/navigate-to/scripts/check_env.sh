#!/usr/bin/env bash
# Prerequisite checks for navigate-to. Read-only, no sudo. Fast probe.
# Hard blockers: no `carla` module, no `agents` package (needs CARLA_ROOT), or no
# reachable server.
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null

"${PYTHON}" - "$CARLA_HOST" "$CARLA_PORT" <<'PY'
import sys

def fail(m): print(f"FAIL  {m}")
def warn(m): print(f"WARN  {m}")
def ok(m):   print(f"PASS  {m}")

try:
    import carla
except Exception as e:
    fail(f"cannot import carla — run the install-python-api skill for this interpreter ({e})"); print("\nprerequisites BLOCKED"); sys.exit(1)
ok("carla module importable")

try:
    import agents.navigation.basic_agent  # noqa: F401
    ok("agents package importable (CARLA_ROOT/PythonAPI/carla on PYTHONPATH)")
except Exception as e:
    fail(f"cannot import agents — set CARLA_ROOT to a carla checkout ({e})")
    print("\nprerequisites BLOCKED — resolve FAIL lines above"); sys.exit(1)

host, port = sys.argv[1], int(sys.argv[2])
try:
    client = carla.Client(host, port); client.set_timeout(4.0)
    sv, cv = client.get_server_version(), client.get_client_version()
    ok(f"server reachable at {host}:{port} (server {sv}, client {cv})")
    if sv != cv:
        # Not cosmetic: a mismatched client can abort mid-call
        # (std::bad_array_new_length) when deserialising snapshots or sensor data.
        warn(f"client/server version MISMATCH: client {cv} vs server {sv}")
        warn("  structured calls may abort — install a matching client with the"
             " install-python-api skill (its bundled-wheel source guarantees a match)")
except Exception as e:
    fail(f"no CARLA server at {host}:{port} ({e})")
    print("\nprerequisites BLOCKED — resolve FAIL lines above"); sys.exit(1)

print("\nprerequisites OK — hard blockers clear")
sys.exit(0)
PY
