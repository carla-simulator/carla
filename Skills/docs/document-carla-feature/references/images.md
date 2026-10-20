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
| One view per map, montages | `mapshots.py OUT Town01_Opt Town02_Opt ... [MAP:x,y,z,yaw,pitch]`, then `montage.py OUT.webp 1280 720 2000 IMG:LABEL[:croptop] ...` | Without a pose, `mapshots.py` frames the spawn points from 30° above. The classic towns show a dark band at the top of the frame (sky edge); crop it with `:0.1`. Town11-13 are too heavy to load in a row. |
| World Partition map (Town15) from a `-game` server | `wp_shots.py OUT "name=x,y,z,yaw,pitch;hx,hy,hz"` | Parks a `role_name=hero` vehicle at `hx,hy,hz` (physics off) under the camera and ticks 400 times; without it the shot only shows the cells around the spectator start. |
| Town street views with traffic | `townshots.py OUT W H name=x,y,z,yaw,pitch[,fov] ...` | Populates 70 cars and 60 walkers, ticks 200, shoots each pose. Scout first at 960x540, then render the picks at 1920x1080. |
| Same parameter at several values | `strip.py OUT.webp CROP|none W IMG:LABEL ...` | Labelled side-by-side strip (black tab with the value), for material or setting comparisons. |
| Material examples in a town | `pie_shot.py SHOTS.json OUT` against an editor PIE session (see *Material examples in a town* in the editor recipes) | Same pose for every variant; weather from the presets (`carla.WeatherParameters.WetNoon`), not extreme values. |
| Material instance parameters, one crop per group | `xwdgrab.py`, then `mi_groups.py GRAB.png OUTPREFIX` | See *Fit the whole Details panel*. |
| Texture channels | export with `AssetExportTask` (see editor recipes), then `channels.py DIFFUSE.png ORME.png OUT1 OUT2` | Splits a building's Diffuse (RGB / Alpha) and ORME (R AO, G roughness, B metallic, A emissive) into labelled panels. |
| Physics substepping convergence | `physconv.py OUT.json` then `plot_physconv.py OUT.json OUTDIR` | Same scripted drive on Town10HD_Opt spawn point 79 (a 164 m straight) for several `fixed_delta_seconds` / `max_substep_delta_time`. Plot speed, not `get_acceleration()`: it is one sample per tick, so a coarse step hides the peaks and the comparison is not fair. |
| Vehicle telemetry | `telemetry_run.py OUT.json` then `plot_telemetry.py OUT.json OUT.png` | Launch, lane change and stop, logging `get_telemetry_data()` every tick. `long_slip` spikes to ~1700 at gear shifts on UE 5.8, so plot `omega` instead. |
| Real recorder outputs for a page | `blocked.py OUT.log` | Records one minute of TM traffic and prints `show_recorder_actors_blocked`; `show_recorder_file_info` and `show_recorder_collisions` take the same absolute path. Paste the output in the page as a code block, trimmed with `...`. |

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
| Town10HD_Opt | `-172.2,183.86,27.64,-45.59,0` | the spectator's start view (what the quick start shows) |
| Town10HD_Opt | museum `100,45,2.5,10,6`, palm boulevard `40,30,2,0,3`, skyscrapers `-60,30,2,-60,20`, town houses `-40,100,1.7,-5,3` | map page street views |
| Town10HD_Opt | aerial `0,60,40,0,-10`, top `0,60,320,0,-90` with fov 70 | map overview, road network |
| Town10HD_Opt | spawn point 79 | 164 m straight, for driving tests (physics, telemetry) |
| Town15 | camera `380,155,175,40,-28`, hero at `440.4,205.96,122.6` | campus aerial |

Switching the editor to Town15: stop PIE, `LevelEditorSubsystem.load_level('/Game/Carla/Maps/Town15/Town15')`
through `ue.py`, then begin play; World Partition PIE blocks remote execution for a minute or two, so
poll the CARLA port instead of the editor.

## Editor recipes

Run each through `ue.py` with the editor open (not PIE), then grab with `xwdgrab.py`.

- **Fit the whole Details panel.** Before launching the editor, set `ApplicationScale=0.600000` under
  `[/Script/UnrealEd.EditorStyleSettings]` in `Unreal/CarlaUnreal/Saved/Config/LinuxEditor/EditorPerProjectUserSettings.ini`
  (it is not reachable from Python and needs a restart); keep a copy of the file and restore it when done. At 0.6 a
  2100 px tall window shows every group of `M_GeneralMaster` and most of `M_RoadMaster`. Then split the grab per group
  with `mi_groups.py GRAB.png OUTPREFIX`. Parameters behind a static switch only show when it is ticked: tick them in an
  unsaved copy (`AssetTools.duplicate_asset` into a `/Game/ZZ_*` folder) and grab the copy. Ticking a texture switch
  can hide the flat values it replaces (`M_GlassMaster` hides *1.Plain Glass*): grab both versions.
- **Asset editor in a tall window.** `AssetEditorSubsystem.open_editor_for_assets([asset])`, find the new window with
  `wmctrl -lG`, enlarge it with `wmctrl -i -r <id> -e 0,X,Y,W,H` so the whole Details panel fits (it cannot be
  scrolled from Python), grab, then `close_all_editors_for_asset`. The grab can come back scaled (`xwdgrab` prints
  the real size): scale your crop coordinates.
- **Class defaults miss the buttons.** `CallInEditor` functions (the `RoadPainter` buttons, for instance) only show
  on an instance: spawn a temporary actor, label it `DOCS_TMP_*`, select it, grab the main Details panel, destroy
  it. Spawning dirties the level; never save it.
- **Renders of material variants.** Spawn the vehicle blueprint (`EditorActorSubsystem.spawn_actor_from_class`),
  give every mesh slot that uses the paint instance one shared `create_dynamic_material_instance`, set parameters
  on it, place the viewport camera, and run `HighResShot 1600x900`; the file lands in
  `Unreal/CarlaUnreal/Saved/Screenshots/LinuxEditor/HighresScreenshot*.png`. Static switches cannot change on a MID.
- **Material facts before writing.** `MaterialEditingLibrary.get_scalar/vector/static_switch/texture_parameter_names(master)`
  lists the real parameters; `get_inputs_for_material_expression` walks the graph from
  `get_material_property_input_node`; `AssetRegistry.get_dependencies` shows what a master reads (that is how
  `M_RoadMaster` was found to read `RenderTexture_Town10HD` through `WorldMaskFunction`).
- **Textures to PNG.** `AssetExportTask` with `filename=....png`, `automated=True`, run by
  `Exporter.run_asset_export_task`. `TextureExporterPNG` fails with a fatal assertion (`SupportsTexture(Texture)`,
  the editor dies) on formats it does not support: render targets, HDR and some others. Export only
  `Texture2D` assets whose `compression_settings` is `TC_DEFAULT`, `TC_NORMALMAP`, `TC_MASKS`, `TC_GRAYSCALE`,
  `TC_BC7` or `TC_ALPHA`, and do it in an `UnrealEditor-Cmd -run=pythonscript -NullRHI` commandlet with a progress
  file, never in the editor you are using: a crash then costs one rerun that skips the bad texture. 4K and 8K
  exports are 50-100 MB each; export only the ones you need. Measure the alpha channel (`std`) of a candidate before
  using it in a panel: most UE5-era building textures have a flat alpha.
- **Scene captures in the editor.** `SceneCapture2D` renders nothing while the editor is in the background:
  run `t.IdleWhenNotForeground 0` first. `TestMaps/EmptyMap` has no lighting at all and renders black at any
  exposure; `TestMaps/L_DrivingTest` has sun, sky and clouds. For anything that has to look like CARLA, prefer the
  PIE recipe below.
- **Material examples in a town (PIE).** Open the town in the editor, place demo actors in the editor world first
  (`EditorActorSubsystem.spawn_actor_from_class`; PIE copies the editor world, actors spawned during PIE are not
  needed), create instances that need a different static switch as unsaved `MaterialInstanceConstant`s in a
  `/Game/ZZ_*` folder, then `LevelEditorSubsystem.editor_request_begin_play()` (`editor_play_simulate` does not
  start the CARLA server). Wait for port 2000 with `ss -ltn` (zsh has no `/dev/tcp`). Change materials on the PIE
  actors from `UnrealEditorSubsystem.get_game_world()`: `create_dynamic_material_instance` and set scalars or vectors,
  `set_material` to swap in a different instance, `set_actor_hidden_in_game` to hide one. Shoot with the CARLA camera
  through the client (`pie_shot.py SHOTS.json OUT`), so the look is the simulator's. When done, kill the editor (the
  `ZZ_*` assets are never saved) and check that the Content repository is clean and that no `ZZ_*` folder exists.
  CARLA lights fake interiors and street lamps at night by itself; other emissives need `EmissiveIntensity` and
  `On/Off` set on the instance or the MID. Night shots auto-expose brighter than they look: an emissive needs a
  much higher intensity than the instance default to stand out.
- **Editor Utility Widgets.** `EditorUtilitySubsystem.spawn_and_register_tab(widget)` opens it in its own window; grab
  that window and close it with `wmctrl -i -c <id>`.
- **Blueprint variable names** are not exposed to Python on a BP class default object; `strings -n 5 BP.uasset`
  lists them, spelling mistakes included.
- **Launch the editor** with `cmake --build Build/Release --target launch` (the build directory of the preset).
  If the grab comes out grey or with a tooltip, the user is using the screen: ask before carrying on.

## Checking the pages

```sh
python3 scripts/images/check_images.py          # broken refs (exit 1), images without caption, unreferenced files
python3 scripts/images/add_captions.py caps.json # insert captions: {"page.md": {"img/...": "caption"}}
python3 scripts/images/relayout_images.py --dry --ref=<merged branch>   # after merging a branch with old image paths
```

Build the site with `mkdocs build --strict` from a virtualenv with `Docs/requirements.txt`. Strict does not fail on
dead in-page anchors, which only print as `INFO`; grep the output for `anchor`.

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
- **Streaming before the shot.** On World Partition maps the textures and the foliage of a cell arrive well after
  its geometry: trees showed bare branches and planters empty 20 s after parking the hero. Wait about 90 s after the
  hero is in place, parked within a few metres of the camera, and 60 s with the camera spawned before capturing
  (decals on walls and tree leaves are the last to arrive). A weather change also takes 30-45 s to settle
  (rain, clouds); shoot the same weather in a row instead of switching it per shot.
- **Debug shapes persist.** `draw_arrow` / `draw_box` with a long `life_time` show up in the next shots too; use a
  few seconds and tick past it before the next camera.
- **Labels on spawn points.** `draw_string` does not reach a camera; project the locations (formula above) and draw
  the numbers on the image, and say so in the caption.
- **Admonitions take no fenced code.** The site has no `superfences`: a fenced block inside `!!! Note` renders as one
  inline code line. Put the code after the admonition, or inline.
- **Captions are italic already.** A nested `<i>` inside the caption shows nothing; use `<b>` or `<code>`.
- **Animated WebP size.** A 60-frame 1280 px flight is ~7 MB; 960 px, every second frame and quality 55 brings it
  under 2 MB. Never re-encode the catalogue overviews frame by frame: they hold each actor for `hold_ms`
  (1.5 s) and fade in between, and Pillow reads every WebP frame duration back as `None`, so a re-save
  flattens the timing. Rebuild them with `build_overview` from `catalogue_doc_gen.py`, and check the
  durations in the `ANMF` chunks of the file (24-bit little-endian at offset 12 of each chunk).
- **Features that showed nothing in UE 5.8** (check before documenting them):
  `vehicle.show_debug_telemetry(True)` draws no HUD; `world.apply_color_texture_to_object()`
  had no visible effect on `ProceduralBuilding*` or `BP_BillBoard*` objects. Also: no shipped vehicle blueprint assigns a mesh to
  `CustomCollision`, and `Util/Tools/Import.py` skips the pedestrian navmesh unless `RecastBuilder` is copied into
  `Util/DockerUtils/dist/`.
