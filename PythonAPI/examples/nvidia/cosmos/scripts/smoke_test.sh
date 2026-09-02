#! /usr/bin/env bash
# End-to-end smoke test of the job API against a server.
#
#   ./smoke_test.sh                     # starts a plain-Python mock server (no docker, no GPU)
#   COSMOS_URL=... COSMOS_TOKEN=... ./smoke_test.sh --external   # against a running server
#
# For every backend the server reports as available it submits a 1-clip job
# with a synthetic clip that satisfies that backend's contract, waits for it,
# downloads the result and checks the manifest.
set -euo pipefail

COSMOS_ROOT=$(realpath "$(dirname "$(realpath "${BASH_SOURCE[0]}")")/..")
PY=${PYTHON:-$COSMOS_ROOT/client/.venv/bin/python}
[ -x "$PY" ] || PY=python3
EXTERNAL=0
[ "${1:-}" = "--external" ] && EXTERNAL=1

WORK=$(mktemp -d "${TMPDIR:-/tmp}/cosmos-smoke.XXXXXX")
trap 'rm -rf "$WORK"' EXIT

exec "$PY" - "$COSMOS_ROOT" "$WORK" "$EXTERNAL" <<'PYEOF'
import os
import sys
import time
from pathlib import Path

root, work, external = Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3] == "1"
sys.path.insert(0, str(root / "client"))
from carla_cosmos import Clip, CosmosClient
from carla_cosmos.serve import MockServer
from carla_cosmos.synthetic import av7_clip, make_clip

srv = None
if external:
    client = CosmosClient()
    print(f"using external server {client.url}")
else:
    srv = MockServer(work / "state", delay=1.0, log_file=work / "server.log")
    client = srv.start()
    print(f"mock server at {srv.url}")

failures = 0
try:
    client.wait_ready(timeout=600)
    models = client.models()
    avail = [m for m in models.values() if m.available]
    print(f"available backends: {[m.contract.id for m in avail]}")
    if not avail:
        print("FAIL: no backend available"); sys.exit(1)

    clips = {}
    def clip_for(c):
        key = c.family
        if key not in clips:
            if key == "transfer2.5-av":
                clips[key] = av7_clip(work / "clips", seconds=1)
            elif key == "transfer2.5":
                clips[key] = make_clip(work / "clips", frames=93, fps=16)
            else:
                clips[key] = make_clip(work / "clips", frames=16, fps=16, scene=True)
        return clips[key]

    for m in avail:
        c = m.contract
        clip = clip_for(c)
        if c.family == "transfer2.5-av":
            controls = {"hdmap_bbox": "scene"}
        elif c.family == "transfer2.5":
            controls = {"depth": "clip", "seg": "clip", "edge": "derive"}
        else:
            controls = {"depth": "clip", "seg": "clip"}
        t0 = time.time()
        job = client.submit_clip(clip, c.id, prompt="smoke test: same scene, overcast", controls=controls)
        info = job.wait(poll=0.5, timeout=1800)
        res = job.result()
        out = res.download(work / "results" / job.id)
        ok = info.status == "done" and res.manifest.backend == c.id and all(p.exists() for p in out.values())
        expected_views = job.info.views or clip.manifest.camera_names
        video_views = {f.view for f in res.manifest.files if f.kind == "video"}
        ok = ok and set(expected_views) <= video_views
        failures += 0 if ok else 1
        print(f"{'ok  ' if ok else 'FAIL'} {c.id:<16} job {job.id} {time.time() - t0:5.1f}s  "
              f"files={len(out)} views={sorted(video_views)} timings={ {k: round(v, 2) for k, v in res.manifest.timings.items()} }")

    # auth must reject without a token, before reading any body
    import httpx
    r = httpx.put(f"{client.url}/v1/blobs/{'0' * 64}", content=b"x" * 1024)
    ok = r.status_code == 401
    failures += 0 if ok else 1
    print(f"{'ok  ' if ok else 'FAIL'} unauthenticated upload -> {r.status_code}")
finally:
    if srv:
        srv.stop()

print("SMOKE TEST", "PASSED" if failures == 0 else f"FAILED ({failures})")
sys.exit(1 if failures else 0)
PYEOF
