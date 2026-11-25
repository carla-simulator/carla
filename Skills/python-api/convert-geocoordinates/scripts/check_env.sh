#!/usr/bin/env bash
# Prerequisite checks for convert-geocoordinates: read-only, exits non-zero only without carla or a reachable server.
# The banner names the skill from its directory, so a recording shows which skill ran.
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
    fail(f"cannot import carla — run the install-python-api skill for this interpreter ({e})")
    print("\nprerequisites BLOCKED — resolve FAIL lines above")
    sys.exit(1)
ok("carla module importable")

host, port = sys.argv[1], int(sys.argv[2])
try:
    client = carla.Client(host, port)
    client.set_timeout(4.0)  # short: a healthy local server answers in <1s
    sv, cv = client.get_server_version(), client.get_client_version()
    ok(f"server reachable at {host}:{port} (server {sv}, client {cv})")
    if sv != cv:
        # Not cosmetic: a mismatched client can abort mid-call
        # (std::bad_array_new_length) when deserialising snapshots or sensor data.
        warn(f"client/server version MISMATCH: client {cv} vs server {sv}")
        warn("  structured calls may abort — install a matching client with the"
             " install-python-api skill (its bundled-wheel source guarantees a match)")
except Exception as e:
    fail(f"no CARLA server at {host}:{port} — start one first, or set CARLA_HOST/CARLA_PORT ({e})")
    print("\nprerequisites BLOCKED — resolve FAIL lines above")
    sys.exit(1)

print("\nprerequisites OK — hard blockers clear")
sys.exit(0)
PY
