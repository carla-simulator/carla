#!/usr/bin/env bash
# Prerequisite checks for manage-content-packs. Read-only, fast.
# Hard blockers: no carla module, no reachable server, a client or server without the content-pack API.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null

"${PYTHON}" - "$CARLA_HOST" "$CARLA_PORT" <<'PY'
import sys

def fail(msg):
    print(f"FAIL  {msg}\n\nprerequisites BLOCKED — resolve FAIL lines above")
    sys.exit(1)

try:
    import carla
except Exception as e:
    fail(f"cannot import carla — run the install-python-api skill for this interpreter ({e})")
print("PASS  carla module importable")
if not hasattr(carla.Client, "get_content_packs"):
    fail("this carla client has no get_content_packs(): content packs need the UE 5.8 (1.0+) client")
print("PASS  client has get_content_packs / mount_content_pack / unmount_content_pack")

host, port = sys.argv[1], int(sys.argv[2])
try:
    c = carla.Client(host, port)
    c.set_timeout(5.0)  # a healthy local server answers in well under a second
    print(f"PASS  server at {host}:{port} (server {c.get_server_version()}, client {c.get_client_version()})")
except Exception as e:
    fail(f"no CARLA server at {host}:{port} — start one (run-carla-ue58-server) or set CARLA_HOST/CARLA_PORT ({e})")
try:
    packs = c.get_content_packs()
    print(f"PASS  {len(packs)} pack(s) known: " + (", ".join(f"{p.name}{'' if p.mounted else ' (unmounted)'}" for p in packs) or "none"))
except Exception as e:
    fail(f"server does not answer get_content_packs ({e}): it predates content packs")
try:
    print(f"PASS  current map: {c.get_world().get_map().name}")
except Exception as e:
    print(f"WARN  could not read the current map ({e})")
print("\nprerequisites OK — hard blockers clear")
PY
