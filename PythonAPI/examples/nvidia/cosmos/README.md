# NVIDIA Cosmos Integration for CARLA UE5

Turns CARLA drives into inputs for NVIDIA's Cosmos video world models
(Cosmos 3 Nano/Super transfer, Cosmos Transfer 2.5 general and the 7-camera
Transfer 2.5 `auto/multiview` path) and, once the server component ships,
submits generation jobs and collects the results.

The client captures **frame-exact multi-camera clips** (RGB plus depth,
segmentation and edge control videos), exports the ground truth as an NVIDIA
**ClipGT Parquet scene package** (consumed unmodified by NVIDIA's
world-scenario renderer for `wsm` / `hdmap_bbox` controls), and validates job
requests against per-backend contracts before anything is uploaded.

## Status

| Component | Status |
|---|---|
| Client capture core (`client/`) | **Available** |
| HTTP client, `carla-cosmos` CLI, mock server (`client/`, `server/`) | **Available** (Phase 2) |
| Server API: auth, blobs, job queue, scheduler, worker protocol | **Available** — runs today with the CPU `mock` worker |
| Cosmos 3 (vLLM-Omni), Transfer 2.5, Transfer 2.5 AV workers, server-side world-scenario rendering | **Code complete**, validated against fake engines on CPU; GPU validation pending on the target node |
| Docker image with baked weights (`server/Dockerfile`, `build_image.sh`, `artifacts.lock`) | **Available**; `:nano` ≈ 96 GB weights, `:full` ≈ 230 GB |

## Layout

```
client/     pip package `carla-cosmos`: contracts, clip format, capture (extra), HTTP client, CLI
server/     pip package `carla-cosmos-server` (API) + `carla-cosmos-workers` (worker protocol, adapters)
            profiles/*.yaml decide which workers start on which GPUs
scripts/    install_client.sh · run_server.sh (docker one-liner) · smoke_test.sh
demos/      single_view_live.py (capture; submit lands with the real workers)
```

## Try the whole pipeline without a GPU (mock worker)

```sh
./scripts/install_client.sh                       # venv + carla wheel + carla-cosmos[capture]
client/.venv/bin/pip install -e server/workers -e server   # API + mock worker into the same venv
./scripts/smoke_test.sh                           # starts a mock server, submits one job per backend
```

Or interactively:

```sh
client/.venv/bin/carla-cosmos serve --mock --port 8000    # prints COSMOS_URL / COSMOS_TOKEN exports
export COSMOS_URL=http://127.0.0.1:8000 COSMOS_TOKEN=...
carla-cosmos models
carla-cosmos synthetic-clip --out ./clips --frames 16 --scene
carla-cosmos submit --clip clips/synthetic_xxxx --backend cosmos3-nano \
    --prompt "overcast, wet road" --control depth=clip --control seg=clip --control edge=derive --out ./results
```

The mock worker echoes the input RGB after a short delay with progress events;
everything else — token auth, content-addressed uploads with dedup, contract
validation, queueing with priorities, cancellation, result manifests — is the
production code path.

## Client quickstart (capture)

```sh
# one-time setup: venv beside the example + carla wheel + carla-cosmos
./scripts/install_client.sh

# CARLA server running on :2000, then:
client/.venv/bin/python demos/single_view_live.py \
    --port 2000 --frames 60 --vehicles 10 --out ./clips --capture-only
```

This spawns a Traffic-Manager-driven hero, captures 60 frames at 30 fps with
the `single_720p` rig and writes a clip:

```
clips/<clip_id>/
  manifest.json                      CARLA version, map, weather, rig, fps, seed, ...
  rgb_camera_front_wide_120fov.mp4   H.264 crf 14
  depth_camera_front_wide_120fov.mp4 8-bit inverse depth, near = bright, H.264 4:4:4 lossless
  seg_camera_front_wide_120fov.mp4   instance-coloured segmentation (deterministic palette)
  edge_camera_front_wide_120fov.mp4  optional: semantic-masked Canny (--edge)
  scene/                             ClipGT Parquet package (ego, obstacles, calibration,
                                     lane lines, boundaries, crosswalks, poles, lights,
                                     signs, wait lines) + per-frame traffic-light states
```

The scene package renders directly with NVIDIA's
`scripts/generate_control_videos.py` (cosmos-transfer2.5 repo):

```sh
python generate_control_videos.py -i clips/<clip_id>/scene -o out \
    --cameras camera_front_wide_120fov      # or "all" for the nvidia_av7 rig
```

### Rigs

* `Rig.single()` / `client/rigs/single_720p.yaml` — one forward 90° camera in
  the `camera:front:wide:120fov` renderer slot.
* `Rig.nvidia_av7()` / `client/rigs/nvidia_av7.yaml` — the seven NVIDIA RDS-HQ
  cameras (120/120/120/70/70/30/30°) with NVIDIA's extrinsics.

Cameras are specified as FLU poses relative to the rear-axle-on-ground point
(NVIDIA's convention).  By default the **roofline mounting rule** applies:
any camera that would sit inside the ego body is lifted to the roof line and
clamped to the bounding-box footprint (NVIDIA's SUV positions land inside a
CARLA sedan's cabin); the *actual* extrinsics are written to the calibration.
Use `mount: exact` in the rig YAML to disable.

### Python API

```python
from carla_cosmos import Capture, Rig, COSMOS3_NANO, ReplayTicks

cap = Capture(world, hero, Rig.single(), COSMOS3_NANO, frames=60, fps=30, edge=True)
clip = cap.run("./clips", "my_clip", seed=7)          # frame-exact, restores settings
assert clip.validate() == []

# deterministic capture from a recorder log:
with ReplayTicks(client, "/abs/path/drive.log", start=5.0, duration=3.0) as ticks:
    hero = ticks.find_ego()
    clip = Capture(world, hero, Rig.single(), frames=60, fps=30, ticks=ticks) \
        .run("./clips", "replay_clip")
```

Backend contracts (`carla_cosmos.contracts`) encode what each backend accepts
and produce actionable errors before upload:

| backend | controls | views | fps (clip → model) | frames (at model fps) | resolutions |
|---|---|---|---|---|---|
| `cosmos3-nano` / `-super` | edge, blur, depth, seg, wsm | 1 | 10/16/24/30 → same | 5–300 (wsm: 101·k) | 256/480/720 |
| `transfer2.5` | edge, vis, depth, seg | 1 | 16 → 16 | 93·k | 480/720 |
| `transfer2.5-av` | hdmap_bbox (required) | 1–7 fixed | 30 → 10 | 29 + 28·(k−1) | 720 |

### Tests

```sh
cd client && .venv/bin/python -m pytest tests            # client unit + mock-server e2e, no CARLA needed
cd server && ../client/.venv/bin/python -m pytest tests  # API, scheduler, protocol against the mock worker
CARLA_COSMOS_TEST_PORT=2000 .venv/bin/python -m pytest tests -m integration   # needs a CARLA server
```

## Demos

| script | what it does |
|---|---|
| `demos/single_view_live.py` | TM-driven hero, single 720p camera, capture → submit (`--backend`, `--control`) → download → viewer |
| `demos/single_view_replay.py` | deterministic clip from a recorder log, then `batch.yaml` prompts × seeds as `batch` jobs |
| `demos/av7_world_scenario.py` | NVIDIA 7-camera rig, ClipGT scene export, Transfer 2.5 AV (`hdmap_bbox` rendered server-side); `--also-cosmos3` adds a Cosmos 3 `wsm` job |
| `demos/viewer.py` | input \| control \| result side by side, per camera, scrubbing |

## Server

One Docker image (Phase 7) with every model artifact baked in; until then the
same server runs from source with the mock worker (above).  Full reference:
[`server/README_SERVER.md`](server/README_SERVER.md).

```sh
server/build_image.sh --nano --hf-token-file ~/.hf_token   # once, on a host with ~300 GB free
./scripts/run_server.sh                 # docker run with defaults, prints the token
carla-cosmos serve                      # same from Python (adopts a running container by label)
```

Backends and where they run (profiles auto-select from the GPUs seen):

| backend | worker | stack | GPUs |
|---|---|---|---|
| `cosmos3-nano` / `cosmos3-super` | `cosmos_workers.cosmos3` → `vllm serve --omni` | vLLM-Omni | 1 / TP 2–8 |
| `transfer2.5` | `cosmos_workers.transfer25` (in-process `Control2WorldInference`, 4 branches) | cosmos-transfer2.5 v1.5.4 | 1 |
| `transfer2.5-av` | `cosmos_workers.transfer25_av` (persistent `torchrun` rank loop) | cosmos-transfer2.5 v1.5.4 | ≥ active views (7 → 8) |
| scene packages → `hdmap_bbox` / `wsm` | `cosmos_workers.wsm_renderer` (NVIDIA renderer, EGL) | transfer25 venv | shares one |

* **Auth**: `Authorization: Bearer <token>`.  The first boot mints a token, prints
  it once and stores it in `<state>/initial_token.txt`.  Tokens are hashed at
  rest; manage them on the host with `carla-cosmos-tokens {list,add,revoke}`.
  There are no rate limits or quotas — a valid token is the only gate.
* **API** (`/v1`): `models`, `blobs/{sha256}` (PUT raw body, `blobs/check`),
  `jobs` (POST → 202, GET, DELETE = cancel/delete), `jobs/{id}/result[/{file}]`,
  `health/{live,ready}`, `status`, `metrics` (Prometheus), `/ui` status page,
  `/v1/docs` (OpenAPI).
* **Jobs**: `queued → preparing → running → done | failed | cancelled`,
  `interactive` before `batch`, one job per worker, every job writes a
  `manifest.json` (request, clip manifest, worker facts, timings, file hashes).
* **Profiles** (`server/profiles/*.yaml`): auto-selected from `nvidia-smi`
  (`mock` on CPU, `nano-1gpu`, `nano-2gpu`, `full-8gpu`; `av-8gpu` manual) or
  forced with `COSMOS_PROFILE`.

### Python API (client side)

```python
from carla_cosmos import Clip, CosmosClient

client = CosmosClient("http://node:8000", token="cc_...")   # or COSMOS_URL / COSMOS_TOKEN
client.wait_ready()
job = client.submit_clip(Clip.load("clips/abc"), "cosmos3-nano", prompt="golden hour",
                         controls={"depth": "clip", "seg": "clip", "edge": "derive"})
info = job.wait(on_progress=lambda i: print(i.progress, i.message))
paths = job.result().download("results/abc")
```

`controls` values: `"clip"` (upload the clip's control video), `"derive"`
(server derives from RGB), `"scene"` (upload the ClipGT scene package, rendered
server-side), `("scene", 0.8)` for a weight, or a `ControlInput`.  The request
is validated against the backend contract before any byte is uploaded, and
uploads are skipped for blobs the server already has.

## Latency expectations (placeholder)

Filled in with measured numbers per backend/GPU once the workers land
(Phase 0 reference points: Transfer 2.5, 93 frames @ 720p on one H100 PCIe
≈ 264 s; world-scenario rendering of 300 frames ≈ 72 s on an RTX 5090).
