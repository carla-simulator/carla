# NVIDIA NuRec Integration for CARLA UE5

Replays neurally reconstructed real-world scenes (NVIDIA NuRec / NRE) inside
CARLA. The scene geometry (OpenDRIVE map) and actor tracks come from a `.usdz`
scenario artifact; photoreal frames are rendered by the NRE container and
fetched over gRPC, while CARLA hosts the actors, sensors, and simulation loop.

## How it works

1. `map.xodr` is extracted from the `.usdz` and loaded via
   `client.generate_opendrive_world(...)` — no named town is used.
2. Actors are spawned from the scenario's track data. Blueprint dimensions are
   probed from the **live server** at startup (cached per CARLA version), so
   the integration survives blueprint catalog changes between CARLA versions.
3. The NRE container (`serve-grpc`) renders RGB frames — and, new in NRE 26.04,
   lidar sweeps — at the poses CARLA reports each tick. All cameras due in a
   tick are rendered in a single `batch_render_rgb` round trip, transferred as
   raw `RGB_UINT8_PLANAR` (no JPEG encode/decode).

## Requirements

- CARLA UE5 (0.10.x) with its Python wheel installed (Python >= 3.10)
- Docker with the NVIDIA Container Toolkit
- NRE container: `nvcr.io/nvidia/nre/nre-ga:26.04.01` (public on NGC)
- A NuRec `.usdz` scenario, e.g. from the HuggingFace dataset
  `nvidia/PhysicalAI-Autonomous-Vehicles-NuRec` (sample sets are per-scene;
  the full dataset is ~1.5 TB)

Run `./install_nurec.sh` from this directory to set up Docker, the NVIDIA
container toolkit, the NRE image, the dataset, and the Python environment.

Scenarios from older dataset releases (e.g. 25.07) may need upgrading:

```sh
docker run --rm --gpus all -v $(pwd):$(pwd) $NUREC_IMAGE \
    upgrade-artifact --help
```

## Running

Terminal 1 — CARLA UE5 server (source build):

```sh
# Standalone binary
./Unreal/CarlaUnreal/Binaries/Linux/CarlaUnreal -carla-rpc-port=2000 -RenderOffScreen
# or the editor: cmake --build Build/Release --target launch-only
```

Terminal 2 — replay with image export:

```sh
python nurec_runner.py \
    -u /path/to/scenario.usdz \
    --saveimages --output-dir data
```

The NRE container is started automatically (`NUREC_IMAGE` env var overrides
the image; the gRPC port is auto-picked unless `-np/--nurec-port` is given).
Containers are reused between runs by default to skip scene loading; reuse is
verified over gRPC (server version + scene id), render flags, and GPU selection
before adoption.

Do not run this standalone runner beside Alpamayo's NuRec backend. Both would
own synchronous ticks. Use
`../../av_stacks/alpamayo/run_alpamayo_nurec.sh`, which composes the scenario
and policy under one tick owner.

## API highlights (`nurec_integration.py`)

```python
with NurecScenario(client, usdz_path) as scenario:
    scenario.add_camera("front_wide_120fov", on_image)        # NuRec camera
    scenario.add_lidar(on_lidar, lidar_type="PANDAR128")      # NuRec lidar (NRE >= 26.04)
    scenario.start_replay()
    while not scenario.is_done():
        scenario.tick()
```

- `NurecScenario(..., renderer_backend="nrend")` — fast C++/CUDA renderer
  backend; `"gsplat"` forces GSplat; default uses the artifact's trained renderer.
- `NurecScenario(..., image_format="jpeg")` — legacy JPEG transfer
  (needs `nvidia-nvimgcodec-cu12`); default is raw planar.
- `NurecScenario(..., enable_asset_editing=True)` +
  `scenario.renderer.edit_assets(...)` / `get_dynamic_objects()` — scenario
  variation via server-side asset replacement/insertion.
- `NuRecRenderService(..., extra_server_args=["--enable-harmonizer"])` — any
  additional `serve-grpc` flag can be passed through.


## Feeding a NuRec scene to Cosmos

`carla_cosmos.nurec` (in `../cosmos/client/`) turns an artifact into a Cosmos clip package: it
reads the rig calibration, the ego trajectory and the OpenDRIVE map straight out of the `.usdz`,
replays the drive in CARLA, and writes the neural RGB alongside CARLA's depth/segmentation AOVs
and a ClipGT scene package.  See `../cosmos/demos/nurec_to_cosmos.py`.

It talks to the render engine over plain gRPC at a configurable `host:port`, so the engine may
run on another machine — unlike `NurecScenario`, which hardcodes `localhost` and launches Docker
itself.  Start a standalone engine with:

```sh
docker run --rm --gpus all --net=host -v $(dirname <artifact>):$(dirname <artifact>):ro \
    $NUREC_IMAGE serve-grpc --artifact-glob <artifact>.usdz \
    --port=46435 --host=0.0.0.0 --test-scenes-are-valid --enable-editing-actors
```

then `--nre-endpoint <host>:46435`.  `--fake-nurec` needs none of this: it substitutes CARLA's
own RGB and exercises the rest of the pipeline on a machine with no NuRec install at all.

## Tests

```sh
python -m pytest tests/
```

`tests/test_coordinates.py` pins the CARLA<->NuRec coordinate convention
against the installed carla wheel's own transform math — if a CARLA release
changes conventions, these fail instead of silently mirroring the world.

## Files

| File | Purpose |
|---|---|
| `nurec_integration.py` | Core: `NurecScenario`, `NurecRenderer`, sensors |
| `nurec_render_service.py` | NRE container lifecycle + gRPC readiness |
| `scenario.py`, `track.py` | `.usdz` parsing, pose interpolation |
| `blueprint_library.py` | Live blueprint probing + dimension matching |
| `utils.py` | Coordinate conversions (see `tests/test_coordinates.py`) |
| `projection_functions.py` | ECEF/ENU georeference alignment |
| `nurec_runner.py` | End-to-end replay example |
| `nre/grpc/` | Protocol definitions + vendored generated stubs |
| `tools/extract_blueprint_sizes.py` | Regenerate fallback blueprint JSONs |
