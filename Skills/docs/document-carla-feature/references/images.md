# Category `images` — graphic resources for the docs

Captures, animations and diagrams for `Docs/`. Unlike the other categories this
one **needs a running CARLA server** (an editor PIE session works) for every
render; diagrams only need Python and Chrome.

The scripts live in [`../scripts/images/`](../scripts/images/). Run them from that
directory; each one writes into the output directory it is given and cleans up
every actor it spawned, and puts the world back in asynchronous mode, even on
error.

## Rules

1. **Folder per page.** An image used by one page lives in
   `Docs/img/<page>/` (folder = the `.md` name without extension). One used by
   several pages lives in `Docs/img/shared/`. Never add files to `Docs/img/`
   itself. Exceptions with fixed paths: `img/catalogue/` (written by
   `PythonAPI/docs/catalogue_doc_gen.py`), `img/snipets_images/` (read by
   `doc_gen_snipets.py`, which matches an image to a snippet by file name
   **without extension**: keep one file per snippet or both get inserted),
   `img/logos/`, `img/btn/` and `img/carla_ue5_readme_img.webp` (README.md).
2. **Link relative to `Docs/`**: `![alt](img/ref_sensors/sensor_rgb.webp)`.
   `../img/...` only works on the built site because of directory URLs and
   breaks on GitHub.
3. **No code in images.** Commands, API calls, flags and snippets go in the
   page as a fenced code block so the reader can copy them. A diagram may name
   things (a port number, a component), never show something to type. SVG text
   embedded with `![]()` is not selectable either.
4. **Look.** Sensor views (what an AV stack sees) use the sensor default
   profile **AutomotiveHDR** (leave `post_process_profile` empty, or set it).
   Art and overview shots (catalogue, aerial views, weather showcase) use
   **Cinematic**, which has no lens artefacts. When a page is about the look
   itself (weather, camera profiles) show both, or one grid per condition.
5. **Formats.** Photos and renders: WebP quality 86, 1280 px wide (grids up to
   ~2000 px). Animations: animated WebP, 10 fps, 60-90 frames, 800-1000 px
   wide, under 3 MB (`anim.py`). Diagrams: SVG (`svgkit.py`), white background,
   palette and fonts from `svgkit.C`. Charts or map plots: PNG from matplotlib.
   Never GIF (the UE4-era GIFs were 9-55 MB).
6. **Every image gets a caption record**: what it shows in one sentence, and
   for renders the town, sensor, camera profile and weather. Keep it next to
   the image list you hand to whoever writes the page (`manifest.py`).
7. **Towns.** Town10HD_Opt is the default. Town15 also gives good shots; it is
   World Partition, so it only streams around a vehicle with
   `role_name=hero` (spawn one near the camera, physics off). Do not jump the
   camera across Town13 in an editor session: it runs out of memory (50+ GB).
8. **Verify before drawing.** Every number, port, packet id or order in a
   diagram is read from the code first (grep), not from the old image.

## Scene recipes

| Need | Script | Notes |
|---|---|---|
| Same frozen street scene through every sensor | `gallery.py OUT --pose x,y,z,yaw` then `render_pc.py OUT OUT/out` and `make_gallery.py OUT FINAL` | Populates traffic (`scene.populate`), lets it run, **freezes it** (`scene.freeze`: autopilot off, physics off, walker controllers stopped) and stages cars and walkers in the camera's front cone (`scene.stage_front`), so every sensor sees the identical frame even when captured in batches. |
| Same frozen scene, many camera profiles x weathers | `looks.py OUT --pose ... --mode matrix [--weathers ...]` then `grids_matrix.py OUT FINAL town10` | 9 weathers x 9 profiles. At night the frozen cars get `Position|LowBeam`, or they look parked with lights off. 200 ticks after each `set_weather` (lighting jump, wetness, exposure). |
| Hood camera drive with 2D/3D boxes, DVS, optical flow | `drive.py OUT --start x,y,z,yaw` then `anim.py` | Boxes are occlusion-tested against a depth camera on the same mount; without it boxes of hidden cars get drawn. DVS sends no message on a frame with no events: get it with a short timeout. |
| Traffic Manager aerial / hybrid physics | `traffic.py OUT --aerial x,y,z,yaw,pitch --top x,y,z` | The TM measures the hybrid radius from the vehicle spawned with `role_name=hero`; the top camera follows it. |
| Vehicle light states | `lights.py OUT [model]` | ClearNight, front and rear 3/4 views; keeps the brightest of 30 frames so the blinker is caught on. |
| Walker skeleton | `skeleton.py OUT` | Bone pairs parsed from `PythonAPI/examples/draw_skeleton.py`. AI walkers report `get_velocity() == 0`; check movement by position. |
| Recorded collision | `crash.py OUT` | Records with `start_recorder`, prints `show_recorder_collisions`. |
| Snippet images | `snippets.py OUT` | `debug.draw_box`/points/lines show in sensor images; `draw_string` text does **not** (HUD only), so put printed output in the page, not burnt into the image. |
| Chase camera | `chase.py OUT` | `AttachmentType.SpringArmGhost`. |
| Map plot with spawn indices | `map_plot.py OUT.png` | From `generate_waypoints` and `get_spawn_points`; top view with +y down, as in the editor. |
| Diagrams | `diag_*.py OUT_DIR` | Writes the SVG and a PNG preview next to it (headless Chrome); look at the PNG before shipping (overlaps, clipped text) and do not commit it. |
| Editor UI screenshot | `xwdgrab.py <window id> OUT.png` (window id from `wmctrl -lG`) | Reads the editor window's own pixels (X11, works even when other windows cover it, no PIE needed). Drive the editor first through `ue.py`: `EditorUtilityLibrary.sync_browser_to_folders([...])` to open a Content Browser folder, `EditorActorSubsystem.set_selected_level_actors([...])` to fill Details, `UnrealEditorSubsystem.set_level_viewport_camera_info(...)` to frame the viewport. Crop the panels you need; outline what matters in red. Undo any temporary folder or selection afterwards. `Shot showui` also renders the window, but only while PIE runs, and in PIE the editor selection API sees no actors. |
| Set-path routes | `tm_paths.py OUT.json R1S R1 R2S R2 [s]` then `routes_plot.py OUT.png OUT.json` | Runs the routes with the Traffic Manager and plots the recorded tracks (cut at the last route point, where the TM takes over). `CAM="x,y,z,yaw,pitch;dir;first_step" OVERLAY=1` adds a top-down animation with each car ringed in its route colour. |
| Server quality levels / OpenDRIVE worlds | `run_server.sh game` (`QUALITY=Low`), then `still.py` / `odr.py` | Close the editor first (24 GB GPU: editor ~15 GB, server ~8 GB). Shoot Epic and Low from the same pose. `generate_opendrive_world` crashed the server on Town03.xodr (raster overflow in `OpenDriveGenerator.cpp`); Town01.xodr works. Above ~40 m the generated ground plane edge shows as a black void. |
| Catalogue actors | `PythonAPI/docs/catalogue_doc_gen.py --recapture [--catalogue ...]` | Writes `img/catalogue/` and the pages. In an editor PIE session pass `--towns Town10HD_Opt --images-only` (PIE cannot switch maps; a partial run must not rewrite the pages). |

## Poses that worked

| Town | Pose `x,y,z,yaw[,pitch]` | Use |
|---|---|---|
| Town10HD_Opt | `102.93,-9.38,2.2,90.39` | street-level sensor views (spawn point 8 junction) |
| Town10HD_Opt | `102.4,-14.0,6.5,90.39,-9` | elevated view for weather / profile grids |
| Town10HD_Opt | `47.56,-57.23,0.6,0` | `drive.py` start (hood camera drive) |
| Town10HD_Opt | aerial `75,-15,30,60,-38`, top `100,20,130` | `traffic.py` |
| Town15 | `440.4,205.96,124.1,39.96,-9` with `HERO=1` | elevated campus street for weather / profile grids |
| Town10HD_Opt | `75,-15,30,60,-38` | aerial junction (Epic vs Low quality, TM traffic) |
| Town10HD_Opt | top-down `-35,20,60,-90,-90` | set_path convergence (routes 43 -> 45,149,151,58,153 and 83 -> 149,151,58,153) |

Switching the editor to Town15: stop PIE, `LevelEditorSubsystem.load_level('/Game/Carla/Maps/Town15/Town15')`
through `ue.py`, then begin play; World Partition PIE blocks remote execution for a minute or two, so
poll the CARLA port instead of the editor.

## Handing the images over

The person writing the pages needs, per image: its path, the page and section it belongs to, a
one-sentence caption, and for renders the provenance (town, sensor, profile, weather). Keep those
records in a manifest outside the repository and hand it over with a table of path changes (old ->
new) whenever files move. Text an image cannot carry (commands, snippet output, a query result) goes
in the manifest notes so it lands in the page as a code block.

## Gotchas

- **VRAM.** Each Lumen scene capture costs about 1.4 GB. More than ~6 cameras
  at once crashed the editor (Vulkan out of memory): capture in batches of 3
  on a frozen scene.
- **Synchronous mode.** Every script sets sync + TM sync and restores async in
  `finally`. A script that dies in between leaves the server frozen for every
  other client: reset it with `scene.set_sync(world, tm, False)`.
- **Remote execution deadlocks** while CARLA sync mode is on: do not call
  `ue.py` (HighResShot, `Shot showui`) from inside a sync capture loop.
- **Frame matching.** After `world.tick()` drain each sensor queue until
  `image.frame >= tick_frame`; the first frames of a new camera are dark/bright
  while auto exposure settles (warm up 40-100 ticks).
- **Never `pkill -f` / `pgrep -f <pattern>`** from a tool shell: the pattern
  matches the shell itself.
- **Projection.** UE camera space to image: `(x, y, z) -> (y, -z, x)`, then
  `u = W/2 + f*y/x`, `v = H/2 - f*z/x`, `f = W / (2 tan(fov/2))`. Level boxes
  from `world.get_level_bbs()` are already in world space:
  `bb.get_world_vertices(carla.Transform())`.
- **Features that showed nothing in UE 5.8** (check before documenting them):
  `vehicle.show_debug_telemetry(True)` draws no HUD; `world.apply_color_texture_to_object()`
  had no visible effect on `ProceduralBuilding*` or `BP_BillBoard*` objects.
