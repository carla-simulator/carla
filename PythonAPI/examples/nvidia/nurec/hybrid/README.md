# Hybrid rendering: CARLA actors inside a NuRec scene

One command renders a clip of a NuRec scene with CARLA-driven vehicles composited in, occluded by the
neural scene's own geometry (per-pixel distance from the engine), in the recorded camera lens, lit by the
sun found in the scene, harmonized:

```bash
python PythonAPI/examples/nvidia/nurec/hybrid/hybrid_run.py --scene 7c2cf6cd --out /path/to/run
```

Outputs in `--out`: `hybrid_final.mp4` (single-frame Harmonizer), `hybrid_temporal.mp4` (temporal
Harmonizer), `neural_only.mp4`, `naive_paste.mp4`, `compare_*.mp4`, per-frame PNGs under `frames/`
(`comp_`, `harm_`, `temporal_`, `naive_`, `neural_`), the raw layers (`A_*.npz` catchers only, `B_*.npz` actors,
`E_*.npz` engine colour/distance/opacity), `illum.json` (sun and sky measured in the scene, with the sun's confidence and every rejected candidate), `illum_vis.jpg`
(sky mask, skyline, candidates), `exposure_calib.json` / `.jpg` (the solved exposure) and `run.log`.

Useful options: `--frames 60` (short test), `--harmonizer single|temporal|both|none`, `--timestep 250`
(temporal Harmonizer strength), `--cars "bp:r,g,b:ahead_m:lane|opp:speed_kmh[:delay_s]"` (repeatable,
`--cars none` for no cars), `--walkers "bp:ahead_m:R|L|lateral_m:speed_kmh[:delay_s[:along|back|cross]]"`
(repeatable; pedestrians on the right/left shoulder walking with, against, or across the traffic),
`--start-frac 0.3` (start later in the clip), `--keep-world` (skip regenerating the proxy world),
`--no-start` (fail instead of starting servers). A CARLA server the driver started is stopped once the capture is
done (the harmonizers need its ~9 GB of GPU memory); one that was already up is left alone.

## What it needs

- The NuRec engine container running (`nre_carla_cosmos`, `nvcr.io/nvidia/nre/nre-ga:26.04.01`) with the
  scene directory mounted at the same path as on the host. The driver starts a second, patched `serve-grpc`
  inside it on `--nre-port` (default 46436): `engine_patch/sitecustomize.py` makes `render_rgb` return
  colour, distance and opacity for `image_quality == -7`.
- CARLA built from this tree: the `carla.OpenDrive.StreetFurniture` console variable (strips the proxy
  world of PCG trees, lamps and signs), the `show_only_*` / `shadow_catcher_tags` sensor attributes, the
  `ftheta` camera model and `exposure_mode` / `exposure_compensation` on `sensor.camera.rt_lens` (engine
  shader `LensModels.ush` carries the model), its AOV twins `rt_lens_instance` / `rt_lens_distance`, and the
  `set_sky_light_map` RPC (the Python wheel must match). A server without them makes `hybrid_video.py` exit with
  the missing blueprint / attribute named. The driver starts `UnrealEditor -game -RenderOffScreen` on
  `--carla-port` (default 3000) if nothing answers there. `UE_ROOT`, `DLSS_SDK`, `DISPLAY` are honoured.
- Scenes under `NUREC_SAMPLES` (default `../nurec_samples/sample_set/26.04_release`), one directory per
  scene with the `.usdz` inside.
- For the temporal Harmonizer: `HARMONIZER_DIR` (default `../harmonizer`) with the NVIDIA/harmonizer
  checkout, `models/diffusion_harmonizer.pkl`, the `harmonizer-cosmos-env` docker image and the two
  patches that let it run without the gated base checkpoint (`text2image_patched.py` bind-mounted over the
  installed pipeline, tokenizer weights exported from the .pkl).
- Python with `carla`, `grpc`, `numpy`, `scipy`, `PIL`, `msgpack`; `ffmpeg`; `docker`; `systemd-run`.

## Pipeline

1. `illum_probe.py`: six engine faces at the ego, stitched into a rig-frame panorama; sun direction from the
   brightest blob that lies in the sky (a mask grown from the top of the band the real cameras saw through
   blue / neutral-white smooth pixels), above the local skyline by 3 deg and above the ground-plane horizon by
   5 deg, not an opaque surface within 300 m by the engine's own distance/opacity, cross-checked against the
   sky's brightness gradient (aureole + cosine fit); a veiled sun's fragmented aureole is merged; without an
   accepted blob the gradient azimuth is used at low confidence. `illum.json` carries `sun.confidence`,
   `sun.method`, `sky_gradient` and `sun_candidates` with the rejection reason of each; `illum_vis.jpg` shows
   them; `illum_aux.npz` keeps distance/opacity so `--offline illum.jpg` re-runs the detection without the
   engine. Before this (2026-09-08) the probe locked onto a sunlit facade at 9.5 deg on a2a4322c (urban);
   the sky rule puts the sun at az 87 / el 21 in the rig frame there, and leaves 7c2cf6cd (highway) unchanged.
   Sky and ground statistics as before.
2. `hybrid_video.py`: generates the OpenDRIVE proxy world without furniture, sets the CARLA sun from the
   probe (the calibrated convention is: bearing toward the sun in CARLA world = `sun_azimuth_angle` +
   180°) and the sky light from the probe panorama (`world.set_sky_light_map`, see `skymap.py` for the
   frame conventions), then captures the CARLA layer: colour from
   `sensor.camera.rt_lens` (path traced, DLSS Ray Reconstruction) rendering the recorded lens directly,
   `camera_model=ftheta` with NuRec's pixel-distance-to-angle polynomial as `distortion_coeffs` (fx = 1/W so
   r is in pixels), `exposure_mode=manual` + `exposure_compensation` solved per scene against the neural render
   (`--rt-exposure-comp auto`, the default; a number pins it, 5.0 was the old raster-layer match -- 1.8 EV too bright
   on the highway scene). `exposure_calib.py` renders `--calib-frames` (8) neural frames spread over the clip and a
   catchers-only `rt_lens` + tag + distance capture at the same poses before the passes; road pixels = CARLA tag
   `Roads` (no `RoadLines`), 4..80 m, neural distance within max(0.5 m, 3 %) of the proxy distance (so the neural
   pixel is road surface too), opacity >= 0.9, no clipped channel; per pixel log2 of the linear-luminance ratio,
   per-frame median re-taken after a +-1 EV trim (real cast shadows and markings under proxy road pixels), median
   over frames; secant iteration on the sensor EV (the 8-bit response is tonemapped, not 2^EV) until the residual is
   below `--calib-tol` (0.05 EV). Writes `exposure_calib.json` (solved EV, residual, MAD over frames, per-frame
   statistics, the `RoadLines` cross-check) and `exposure_calib.jpg` (layer | neural | road mask), and the solved
   value + residual into `meta.json` (`rt.exposure_comp`, `rt.exposure_calib`). Matching the road luminance folds the
   real-asphalt / proxy-asphalt albedo ratio into the exposure, which is what the actors then inherit. Measured
   2026-09-08 on 7c2cf6cd (highway): 5.0 EV left the proxy road +1.8 EV over the neural road (MAD 0.03-0.05 EV
   over the clip), solved 3.51-3.56 EV; two recorded vehicles rendered by the neural scene against CARLA cars of
   a similar colour at the same poses read +1.5..+2.1 EV at 5.0 EV and +0.3..+0.9 EV (mean luminance; -0.3 EV by
   median: the sharp CARLA car has sun-lit paint and dark underside the motion-blurred neural car has not) at the
   solved value, i.e. the actors end within the scatter of that check, slightly on the bright side. The
   `RoadLines` cross-check reads +1.0 EV where the proxy's white/yellow lines coincide with real markings (few
   pixels, +-1 EV per frame on the highway) and is reported, not used. On a2a4322c (urban) 5.0 EV already
   matched (-0.02 EV), but with the probe's sun at 9.5 deg altitude (a bright facade, not the sun): the solved EV
   absorbs the illumination error of a scene, which is why it must be per scene. `--calib-only` runs the
   measurement and exits (restores the server); fewer than 2000 shared road pixels per frame is NaN for that frame (the
   median over frames ignores it) and is logged with the road set at every stage of the mask; every frame below
   the count is a fatal error naming the counts. Seen on a2a4322c (three runs, 2026-09-08): the first sensor set
   after the world load returns one or two entirely empty tag AOV frames 3-6 ticks after spawning, after the
   warm-up has already seen the road tagged on consecutive ticks (`tag: 0` in the log); later iterations are
   clean and the solved EV agrees to 0.01 EV. The same dip is why `hybrid_video.py` warms pass B up until the
   set of actor ids the tag AOV labels is unchanged for 3 lit ticks (8 ticks on a2a4322c; with 3 fixed ticks the
   walkers and most car pixels were missing from frames 0-1 in colour, tag and distance alike) and logs the ids
   of the first 8 frames (`tag@k`) and the tick counts (`meta.json: warmup_ticks`). `tests/exposure_calib_unit_test.py` checks the mask and the solver on synthetic pairs
   without a server. Instance and Euclidean distance of the actors come from the path tracer's own primary-hit
   AOVs, `sensor.camera.rt_lens_instance` and `sensor.camera.rt_lens_distance`: the *same* lens as `rt_lens`,
   100 % of the frame, no resampling -- see below. Pass B shows the actors over the shadow catchers
   (`show_only_actor_ids` + `shadow_catcher_tags`), pass A the catchers alone (`show_only_tags`) for the
   cast-shadow ratio.
   In synchronous mode the path-traced sensors deliver the render of the same tick (blocking batched
   readback, `carla.RTLens.SyncModeBlockingReadback`), so no pairing or pacing is needed. `--rt-spp` 16
   by default; the path tracer finishes at most `r.PathTracing.MaxFramePassCount` sample passes per tick
   (64 on this server, engine default 16) and the sensor clamps to it with a warning, since a count above
   the cap never reaches the denoise gate and comes back as raw samples (four times the noise). Measured
   frame-to-frame noise on car pixels, motion blur off: 8 spp 5.7, 16 spp 5.0, 32 spp 4.8, 64 spp 3.7
   (54 / 54 / 74 / 116 ms per tick for the colour and instance sensors).
   Synthetic cars are settled under physics for `--settle-ticks` (20) before the capture, late starters one at a time at
   their start pose and re-parked, so nothing floats in the first frames it is visible; drive with physics along the lanes, pedestrians walk kinematically
   (`WalkerControl`) on the shoulders; every recorded actor is parked underground
   in the engine, which renders with the native f-theta camera. The cube-map `*_fisheye` capture and the
   pinhole-warp capture were removed from the pipeline on 2026-09-08 (the sensors stay in CARLA;
   `tests/lut1d_fisheye_test.py` still covers them). A missing sensor frame, a missing blueprint, an engine
   error or a failed calibration ends the run with a message; every exit restores the server (asynchronous
   mode, sky light map, cvars, actors).
3. `hybrid_video_finish.py`: depth-tested alpha (engine distance vs layer distance, opacity < 0.6 counts as
   empty) from the mask eroded by `--erode 2`, colours of the silhouette band pushed in from the mask
   interior (`--edge-extend`: the layer's edge pixels are anti-aliased against CARLA's own sky, which
   otherwise shows as a bright rim, worst on thin figures), cast shadow from the catchers-only colour,
   writes `comp_`, `naive_`, `neural_` frames.
4. Harmonizers: `engine_patch/harmonize.py` runs the engine's model at 1088x1920 inside the NuRec container;
   the temporal model runs in its own container at 1360x768 with four previous outputs as references.
5. `make_videos.sh` and ffmpeg for the comparisons.

With `--scenic SCENARIO.scenic` step 2 is `scenic/hybrid_scenic.py` instead: a [Scenic](https://scenic-lang.org)
scenario drives the ego, which carries the recorded camera (the three `rt_lens` sensors attached to it), and every
other agent; pass A and the neural render are replayed over the ego's captured poses. Two scenarios ship in
`scenic/` (a highway cut-in, overtake and brake wave, an urban stop-and-go and pull-out); `scenic/README.md` has the driver, the globals a scenario
receives and how to write one.

## `sensor.camera.rt_lens_distance`

The geometric twin of `sensor.camera.rt_lens`: identical attribute surface (`camera_model`,
`distortion_coeffs`, `fx`/`fy`/`cx`/`cy`, `theta_max_deg`, `fov`, `show_only_*`), but each pixel carries the
Euclidean distance from the camera origin to the primary hit **along that pixel's lens ray**, in metres,
as one `float32` per pixel:

```python
img = q.get()                                   # carla.DistanceImage
d = np.frombuffer(img.raw_data, np.float32).reshape(img.height, img.width)   # metres
d[d > 900] = np.inf                             # rays that hit nothing (sky)
```

It is a genuine path-tracer AOV, not a post-process of the colour image: the ray generation shader records
the primary hit distance into a dedicated R32F render-graph texture beside the existing albedo/normal/depth
AOVs (`FPostProcessSettings::PathTracingLensAOV`, `PathTracer.LensDistance`), and the scene capture resolves
that texture through `ESceneCaptureSource::SCS_PathTracingLensDistance`. Nothing on that route applies
exposure, tone mapping, denoising or 8-bit quantization, and the primary ray is fired through the exact
pixel centre with no anti-aliasing jitter, so the output is deterministic (bitwise identical across ticks on
a static scene) and metrically exact.

Why it exists: a pinhole depth image warped into a fisheye can only cover the part of the frame the pinhole
frustum reached (~95 % for the NuRec f-theta rigs), and view-space Z -- the only thing a raster depth camera
can report -- is not a meaningful depth for a lens whose rays leave the frustum at all. This sensor covers
the whole frame in the recorded lens.

## `sensor.camera.rt_lens_instance`

The labelling twin of `sensor.camera.rt_lens`, same attribute surface again, with the byte layout
`sensor.camera.instance_segmentation` already uses, so the existing decode works unchanged:

```python
b = np.frombuffer(img.raw_data, np.uint8).reshape(img.height, img.width, 4)
label = b[:, :, 2]                                    # semantic tag (crp::CityObjectLabel)
actor = b[:, :, 1].astype(np.int32) | (b[:, :, 0].astype(np.int32) << 8)   # 16-bit instance id
```

The semantic image is the R channel of this one, so one sensor covers both. Note the id is
`AActor::GetUniqueID()` -- the same id the raster sensor reports, not the client-side `actor.id`.
Pixels whose ray hit nothing (sky) read label 0, id 0.

The tag comes from exactly where the raster pass gets it: `ATagger` writes it into custom
primitive data slot 4 (`Carla/Game/Tagger.cpp`), which `Shaders/Private/SegmentationSensor.usf`
reads as `CustomPrimitiveData[1].xy`. The path tracer's closest-hit shader reads that same
GPUScene field, carries it in the material payload, and the ray generation shader records it into
`PathTracer.LensTag` (R32_UINT) beside the albedo/normal/depth AOVs
(`FPostProcessSettings::PathTracingLensAOV == 2`); the capture resolves it through
`ESceneCaptureSource::SCS_PathTracingLensTag`. The raster segmentation pass cannot simply be
reused here: `ShowFlags.PathTracing` makes the renderer treat the frame as a ray-traced overlay
and skip the depth prepass, base pass and Nanite raster entirely, so it would yield an empty or
stale buffer. Like the distance AOV it is point sampled at the pixel centre and never blended --
the average of two object ids is a third object -- so ids are exact and reproducible.

`tests/` holds one live-server check per CARLA feature: `showonly_test.py`, `lut1d_fisheye_test.py`,
`fisheye_memory_test.py`, `skymap_test.py`, `rt_lens_ftheta_test.py`, `rt_lens_distance_test.py`,
`rt_lens_instance_test.py`, `rt_lens_sync_determinism_test.py` (same-tick delivery and zero drops at 16 spp),
`rt_lens_first_frames_test.py` (every actor complete in tag, distance and colour from the first delivered frame:
in synchronous mode the sensors build the ray tracing material pipeline synchronously,
`carla.RTLens.SyncModeBlockingPipelineCreation`; before that, freshly spawned actors rendered through the
engine's black, untagged fallback hit shader for the ticks the pipeline took to compile; each takes `--port`;
see their docstrings), and two that need no server: `exposure_calib_unit_test.py` and `scenic_xodr_test.py` (the
OpenDRIVE sanitizer on every sample artifact: the cleaned map must parse in Scenic with the recorded start pose on a lane).

`carla_sun_calib.py` re-derives the sun convention on a live server if it is ever in doubt; `harm_merge.py`
applies only the low-frequency part of a harmonizer's correction, smoothed over time, when the composite's
detail must be kept.
