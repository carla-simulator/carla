# UE5.8 hybrid cinematic rendering

CARLA supplies simulation state; Unreal Sequencer/Movie Render Graph supplies
actor beauty and integration passes; NuRec supplies calibrated environment
plates. This branch includes the late pedestrian crossing, contact corrections,
six-camera native F-theta rig, compositing, and both diffusion review variants.

## Current entry points

- `prepare_traffic_crossing.py`, `crossing_unreal.py`: recorded Scenic traffic.
- `export_mary.py`, `build_mary.py`, `deform_mary_contact.py`: external pedestrian
  cache, native texture timing and terrain contact. `diagnostics/` preserves
  shot-specific preparation/audit scripts.
- `prepare_multicam.py SOURCE_SHOT OUTPUT_RIG --width 960`: derive calibrated
  views from `OUTPUT_RIG/source-cameras.json` and the source shot capture.
- `render_multicam.py OUTPUT_RIG --stage plates|cinematic --spp 8`: plate or
  cinematic rendering. `check_camera_markers.py` checks native projection.
- `make_multicam_review.py OUTPUT_RIG`: synchronized camera wall.
- `diffuse_multicam.py OUTPUT_RIG`: independent temporal and single-frame
  harmonization, individual videos and both camera walls. See
  `harmonizer_support/` for the tested external model runtime changes.

The demonstrated rig has IDs 0,1,2,3,5,6: the Alpamayo2 Super trajectory profile,
not its different VQA camera profile. All204 frames share simulation timing.
Review output is960x540 at24fps; rolling shutter, exact10Hz model context and
radiometric sensor equivalence are not implemented. Engine companion branch
`ue58-hybrid-rendering` fixes calibrated lens UVs for padded render targets and
preserves translucent vehicle ownership in geometry AOVs.

The six-view raw render passed sampled MVP review and numeric marker gates.
Diffusion is experimental: both variants can invent background geometry and
are not jointly multiview-consistent. Temporal conditioning even made phantom
cars persist in blurred foliage. Preserve raw/calibration/depth as authoritative.
No generated media, checkpoints, paid pedestrian assets or NuRec datasets are
included. SeedVR2 testing remains future work.

The tools currently assume sibling `carla-ue58-dev`, `UnrealEngine_5`,
`harmonizer` directories and `.venv-rtaov` in their workspace parent. Configure
`UE_ROOT`, `DISPLAY` and `DLSS_SDK` for the target machine. Existing shot manifests,
source camera calibration and local licensed assets are required inputs.

## Historical first-shot setup and detailed rendering notes

### Initial sh010 prototype

Separate from `hybrid_video.py`: CARLA supplies recorded simulation state;
Sequencer and Movie Render Graph supply final Unreal pixels. The accepted highway
videos are not inputs to this prototype and are not overwritten.

The first shot is a 48-frame, 24 fps crossing with the existing German MetaHuman
in PhysicalAI scene `a2a4322c-3f99-40c3-94df-17a67f56f55d`. Captures and diagnostic
renders live under `artifacts/hybrid-cinematic-mvp/sh010` in the workspace.
Generated Unreal assets are isolated under `/Game/HybridCinematicMVP/sh010`.
This is not yet an accepted cinematic result. Consult the shot's validation
reports and manifest; missing or pending gates are not passes.

The current diagnostic deliverable is `sh010/comp/review.mp4`: 48 frames at
24 fps, 960×540. All 192 UE layer/utility EXRs passed the file-level checks.
The NNE alpha-aware denoiser, fitted receiver, bounded mesh-height correction,
and local shaded lighting are included. The renderer completed with exit code
zero. Color calibration, rendered markers, exact replay, motion blur, foreground
occlusion tests, and final-resolution temporal/character review remain pending.

## Commands

From the workspace root, using the CARLA-compatible Python environment:

```sh
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/validate.py artifacts/hybrid-cinematic-mvp/sh010
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/run_unreal.py build artifacts/hybrid-cinematic-mvp/sh010 --spp 32
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/run_unreal.py render artifacts/hybrid-cinematic-mvp/sh010 --limit 1
```

To reproduce the current diagnostic after the saved shot is built:

```sh
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/run_unreal.py render artifacts/hybrid-cinematic-mvp/sh010 --limit 48 --spp 32 --denoiser nne
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/check_render.py artifacts/hybrid-cinematic-mvp/sh010
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/composite.py artifacts/hybrid-cinematic-mvp/sh010 --preview-srgb-assumption
```

Build uses an editor commandlet; rendering uses a full offscreen editor and
Movie Render Graph. The launcher holds the workspace Unreal lock. It enables
MovieRenderPipeline for that process without modifying the project descriptor.
`--spp` configures sampling during build or render. `--limit` must fit the shot's
manifest range. `--start-frame` selects a later diagnostic frame. A full render
must cover the complete manifest range before temporal review.

The renderer runs beauty, integration, and clean as separate jobs to bound
Vulkan ray-tracing allocation peaks. Three simultaneous layers exhausted this
machine's 32 GB VRAM. Raw 32-sample, 48-frame output has completed successfully;
higher sampling must be tested before a full job. `--denoiser nne` enables the
NNE alpha-aware OIDN model for the process; its temporal quality needs review.
On this Linux/Vulkan build it runs through ORT on the CPU (about 1.2 seconds
per 960×540 output layer after initialization). The GPU NNE runtime is not
enabled by the platform's shader capabilities; this is distinct from the GPU
path tracing itself.
The interactive project's forced DLSS-RR preset is overridden because it made
the beauty alpha fully opaque. The cinematic filter width is explicitly 1.

Utility materials are loaded and compiled before rendering. In this 5.8 build,
`MaterialEditingLibrary.get_statistics` submits missing permutations and waits
for compilation. Merely loading a material or waiting produced fallback beauty
RGB in files labelled as depth. `check_render.py SHOT` checks the actual EXRs,
including scalar depth, coverage, dimensions, and freshness when a run manifest
is available. Successful utility checks do not certify all production gates.

`fit_proxy.py SHOT` fits the road near the actor from high-opacity NuRec depth.
After an uncorrected 48-frame render, `grounding.py SHOT` can create a bounded,
recorded mesh-height correction from visible shoe depth. The original capture
remains immutable. This is a cinematic alignment derivative, not deterministic
simulation replay or foot IK. Rerender and inspect contact and gait before
acceptance; existing correction files are protected against accidental reuse.

`capture.py --help` and `plates.py --help` describe fresh captures. Capture needs
a dedicated CARLA server and replaces that server's world with the source
OpenDRIVE map. It records camera poses, actor transforms, relative bone poses,
traffic-light state, simulation frame/time, and exact rational frame timestamps.
It restores simulation settings and destroys its walker on exit. It never
captures CARLA RGB for final beauty. Existing captures are protected against
accidental overwrite.

`plates.py` needs the NuRec server with `../engine_patch/sitecustomize.py` loaded.
The new `image_quality=-8` / `HYF1` transport carries native unclamped float32 RGB,
ray distance, and opacity. Existing `-7` / `HYBR` 8-bit clients retain their format.
The shot's EXRs preserve beauty/coverage as half float and depth as float32.

`composite.py SHOT --preview-srgb-assumption` explicitly permits an approximate
preview conversion. Without that option, unverified source color semantics cause
an error. Never interpret the preview override as passing the production color
gate. The compositor expects the beauty, integration, clean, and depth render
files from the saved graph. It emits ACEScg EXRs and separate sRGB preview PNGs.

## Mary 4D asset comparison

`sh010_mary` reuses the original shot's calibrated camera, NuRec plates, receiver
fit, and lighting. Its character is the supplied Renderpeople
`rp_mary_4d_006_walkingCalling_BLD` performance, with changing mesh topology and
4K texture frames. It is a cinematic geometry cache, not a CARLA skeletal walker.
The supplied blend includes object locomotion as well as Alembic deformation;
importing its ABC alone loses the forward motion. `export_mary.py` evaluates the
blend and exports frames 1–61, including both, as a separate centimeter cache.
Original source files are never saved or overwritten.

The completed diagnostic is `sh010_mary/comp/review.mp4`; the comparison with
the preserved MetaHuman render is `sh010_mary/comp/comparison.mp4`. Both contain
48 frames at 24 fps. All 192 Mary UE EXRs passed the file-level checks, and the
renderer exited successfully. `sh010_mary/REVIEW.md` and `status.json` record
the inspected frames, validation evidence, and remaining limitations.

```sh
HYBRID_SHOT="$PWD/artifacts/hybrid-cinematic-mvp/sh010_mary" blender -b /home/german/Downloads/rp_mary_4d_006_walkingCalling_BLD/rp_mary_4d_006_walkingCalling_4k.blend --disable-autoexec --python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/export_mary.py
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/run_unreal.py build artifacts/hybrid-cinematic-mvp/sh010_mary
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/run_unreal.py render artifacts/hybrid-cinematic-mvp/sh010_mary --limit 48 --spp 32 --denoiser nne
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/check_render.py artifacts/hybrid-cinematic-mvp/sh010_mary
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/composite.py artifacts/hybrid-cinematic-mvp/sh010_mary --preview-srgb-assumption
```

The existing Mary manifest and original sh010 assets are prerequisites. The
launcher dispatches `actor_render_type: geometry_cache` to `build_mary.py` and
enables the GeometryCache and AlembicImporter plugins for that process. This
builder is deliberately specific to this asset and template shot. Existing
imports are reused; a changed source export requires a fresh asset name or
explicit reimport before rebuilding.

Playback preserves the supplied performance speed. At each 24 fps output time,
the mesh and material use source frame `1 + floor(30 * output_time + 0.5)` (nearest sample). Mesh
interpolation/extrapolation are disabled because topology and UVs change.
Sequencer uses one texture material instance per selected source frame. A one
tick (1/24000 second) cache offset avoids float rounding below exact half-sample
boundaries. Before rendering, all requested Sequencer times are evaluated and
checked against `mary-frame-map.json`; `mary-sequence-validation.json` records
the observed cache times and bound materials.

The root's height follows the fitted road using sampled cache bounds, with
3 mm clearance. This is a placement adjustment, not foot IK. The MetaHuman's
CARLA motion and its `grounding.json` are not applied to Mary. Different native
gaits and heights remain visible in the comparison. The geometry-cache material
uses the supplied sRGB color textures with roughness 0.95 and specular 0.1;
there are no separate supplied skin/hair/normal layers in this test.
The preview color, shutter, resolution, and production-gate limitations above
also apply to Mary. She does not satisfy the recipe's strand-groom or MetaHuman
close-up requirements simply because the source asset is marketed for VFX.

## Conventions and checks

### Scenic traffic crossing shot

`sh020_traffic_crossing` uses `urban_traffic_crossing.scenic` telemetry from the
same a2a4322c source log. It contains 370 frames at 24 fps: ego approaches queued
traffic, stops, and Mary crosses in front of the lead car. The actual CARLA
walker is a simulation proxy; her cinematic root follows its recorded XY path
while a synchronized geometry cache supplies the VFX mesh and textures.

`prepare_traffic_crossing.py BEHAVIOR_JSON MARY_TEMPLATE NEW_SHOT` creates the
capture derivative using the running NuRec service on port 46439. It retains
the original calibrated source camera at the start of the drive and moves it
with the simulated ego, using yaw stabilization. NuRec timestamps select the
nearest recorded rig pose; the simulation and delivery clock remain 24 fps.
Original telemetry remains immutable and its hash is in the manifest.

The longer `export_mary.py` export takes `mary_export_end` from the manifest.
The builder uses one exact cache sample per delivery frame to hold her pose
before and after crossing. Native root motion is replaced by Scenic root
motion, preserving the local gait. Vehicles use the native Lincoln, Mini, and
Nissan assets with isolated paint materials. Ground placement uses vehicle mesh
bounds, excluding CARLA's large trigger volumes.

The completed review is `sh020_traffic_crossing/comp/review.mp4`. All 370 frames
were rendered and composited; the 15.4167-second MP4 passed full decoding and
frame-count/rate checks. The shot's `REVIEW.md`, `video-validation.json`, and
contact sheet record the visual review and remaining quality limits.

```sh
# After preparing the new shot and exporting Mary with HYBRID_SHOT set:
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/plates.py artifacts/hybrid-cinematic-mvp/sh020_traffic_crossing --limit 370
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/fit_proxy.py artifacts/hybrid-cinematic-mvp/sh020_traffic_crossing
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/run_unreal.py build artifacts/hybrid-cinematic-mvp/sh020_traffic_crossing
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/run_unreal.py render artifacts/hybrid-cinematic-mvp/sh020_traffic_crossing --limit 370 --spp 32 --denoiser nne
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/check_render.py artifacts/hybrid-cinematic-mvp/sh020_traffic_crossing --limit 370
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/composite.py artifacts/hybrid-cinematic-mvp/sh020_traffic_crossing --limit 370 --preview-srgb-assumption
.venv-rtaov/bin/python carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/make_review.py artifacts/hybrid-cinematic-mvp/sh020_traffic_crossing
```

This is a 540p diagnostic render, with the same approximate source-color and
lighting assumptions as the earlier Mary test. The road receiver is a local
plane fit, not a full reconstructed collision mesh. Mary's held waiting pose is
not a separately authored idle animation. These limits remain distinct from
the independently validated Scenic behavior and deterministic replay.

### Shared conventions

- CARLA world: metres, X forward / Y right / Z up. Unreal assets: centimetres.
- Camera: OpenCV optical X right / Y down / Z forward; camera-to-world matrices
  act on column vectors. Image Y increases downwards; integer pixel centres.
- The scene-to-CARLA transform is recorded, including handedness conversion.
- MVP importer accepts only centred square-pixel pinhole K, with no distortion.
  Unsupported intrinsics fail validation rather than silently changing framing.
- The engine's own view-projection matrix is compared against K for three
  non-coplanar markers. `camera-numeric-gate.json` is a numeric check, not a claim
  that rendered marker images or sliding have passed visual inspection.
- Capture checksum, consecutive simulation frames, rational timestamps and EXR
  decoding are checked by `validate.py`. This does not certify lighting or hair.
- Alpha is a premultiplied foreground coverage signal. NuRec opacity modulates
  foreground occlusion; it is not applied a second time to the complete plate.
- NuRec ray distance is converted to optical Z for merging. Its internal depth
  weighting remains unverified; these exports are not labelled gsplat ED.
- The integration layer uses signed differences between paired proxy renders,
  scaled to local plate luminance and gated by receiver/depth agreement. This preserves the possibility of reflected
  light instead of treating every interaction as a black multiply.

## Outstanding acceptance work

The initial transport/projection stage is 960×540 with sharp samples (zero
shutter), not the recipe's finished 4K/180-degree shutter output. Shared shutter
sampling, verified motion vectors, rendered marker alignment, replay comparison,
proxy fit, contact, groom/skin quality, and temporal validation still need evidence.
Longer scenario previews remain diagnostic until those gates pass; do not label
them film-quality.

The source model saturates RGB at 1.0 internally and can apply learned image
processing. Removing the exporter's uint8 conversion does not recover clipped
highlights or establish scene-linear radiometry. The environment HDR container
currently holds an inverse-sRGB approximation from an LDR NuRec panorama;
it is not a captured HDR light probe. Lighting/exposure remain shot parameters.

Utility depth uses a high-precision SceneDepth post-process material. Its geometry
and edge agreement with path-traced beauty must be checked, especially for hair
and transparency. A foreground matte is not a full per-object ID solution.

## Recovery

After an interruption, check processes before relaunching. Run `validate.py` to
verify the durable capture and plates. Reuse complete plate sequences. The Unreal
launcher archives old status/log files before starting so an old success cannot
certify a failed restart. `unreal-render-status.json` must report success, and
output counts/decoding must still be checked separately.

## Tests and references

```sh
.venv-rtaov/bin/python -m unittest discover -s carla-ue58-dev/PythonAPI/examples/nvidia/nurec/hybrid/cinematic/tests -v
```

Tests cover calibrated axes, ray-to-Z conversion, HDR-preserving float transport,
malformed payload rejection, partial-alpha occlusion, explicit color assumptions,
and EXR precision. Unreal API calls are validated in the installed 5.8 build;
the engine's MovieGraph examples and C++ headers are the version-specific reference.

- [Epic Path Tracer](https://dev.epicgames.com/documentation/unreal-engine/path-tracer-in-unreal-engine)
- [NVIDIA NuRec render outputs](https://docs.nvidia.com/nurec/nurec/render-command.html)
- [NVIDIA NuRec gRPC API](https://docs.nvidia.com/nurec/api/grpc_api_guide.html)

## Approved late-crossing derivative

`sh030_late_crossing` derives from the critic-approved `late_crossing/iterations/005/main/behavior.json`.
It retains the full 312-sample source and delivers the first 204 samples (8.5 s)
at 24 fps, with a moving/slowing Lincoln lead and Mary
crossing right to left. Camera and actor XY/yaw are captured; cinematic Z
placement follows local NuRec ground samples, including the starting curb.
`prepare_traffic_crossing.py` accepts the actual recorded cast, without requiring
queue/standing actors. `crossing_unreal.py` keys all vehicle transforms and
bakes wheel rotation from travelled distance and the asset's wheel radii.

After plates, run `fit_proxy.py SHOT` and `fit_contact.py SHOT` before build.
The contact fitter tracks the previous local ground surface to avoid switching
to adjacent planter tops near the crossing endpoint, then smooths heights over
nine frames. Review `contact-validation.json` and source imagery before render;
small plane residuals alone do not prove surface identity. Actor XY and event
timing remain unchanged. `proxy-fit.json` can lower the large receiver using
`receiver_vertical_offset_m`; actor grounding still uses local contact fits.

Render output resolution now follows the shot manifest, including copied graph
assets. `traffic-sequence-validation.json` compares evaluated Sequencer vehicle
XY with capture samples; Mary cache/texture checks remain separate.

The intended delivery is 1920×1080 at 24 fps for 8.5 seconds, 64 spatial samples with NNE
denoising. Approximate NuRec color conversion and sharp shutter sampling still
apply; this is an MVP cinematic composite, not a claim of fully calibrated
film-production output. Original scenario captures and prior videos remain intact.

The sh030 edit cuts after measured ego-lane clearance (7.7917 s), before Mary
reaches a held walking pose. Its receiver follows local contact height offsets
per frame, and signed negative integration deltas are graded to local plate
chromaticity when `match_shadow_color_to_plate` is enabled. Positive reflection
deltas retain their rendered color.
