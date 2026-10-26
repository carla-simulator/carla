## LATEST Changes

### New features

#### Simulator

* **CARLA now runs on Unreal Engine 5.8**, on CARLA's own fork of the engine. It brings World Partition streaming and Large World Coordinates for the large maps, Lumen on hardware ray tracing with ray-traced shadows, and the path tracer the ray-traced lens cameras render through.
* **Balanced the vehicle lights across models**: every car uses the same emissive intensity, and each light group — low and high beam, position, fog, brake, reverse, blinkers — is tuned against a reference vehicle. The Dodge Charger and Kia Niro stop borrowing the police Charger's materials, the CarlaCola and firetruck lamps light the right group, the Audi TT's front left blinker no longer lights with the right one, and the Tesla Model 3's blinkers are amber. See `Docs/tuto_content_authoring_vehicles.md`.
* **Rebuilt weather and night lighting on the UE5 sky**: the weather API drives the sky rig again (maps were near-black at noon), street lamps emit real light in lumens, vehicle headlight beams, moonlight and stars work at night, and rain falls per camera with droplets on the lens.
* **Improved the OpenDRIVE map generator**: a ground plane and terrain that follow road elevation, crosswalks, lane markings through junctions, `lateralProfile <shape>`, PCG street furniture, and a `spawn_custom_mesh` RPC for client-supplied meshes.
* Restored the **classic towns**: Town01 to Town07 are back alongside Town10HD, migrated to the new lighting stack. They ship only as layered maps, so load them by their `_Opt` name (`Town03_Opt`) and toggle their layers with [carla.MapLayer](https://carla.readthedocs.io/en/latest/core_map/).
* Moved the **large maps to World Partition**: Town11, Town12, Town13 and Town15 stream their cells in and out around the vehicles instead of CARLA's old tiling, with no rebasing and nothing tiled on the client side — from the Python API they behave like any other map. Every vehicle keeps the ground under itself loaded, and a vehicle whose `role_name` is `hero` gets a wider ring set by `tile_stream_distance` on `carla.WorldSettings`. A `CarlaLargeMapConvert` commandlet converts legacy tiled towns.
* **Vehicle wheels now turn when Chaos is not simulating them**: with physics disabled, or while another movement component drives the vehicle, the wheels roll by the distance they travel instead of staying frozen, and they keep their spin when Chaos takes them back. A wheel that moves faster than 150 m/s is treated as teleported and is not turned. Custom vehicles need their animation blueprint reparented to `CarlaVehicleAnimationInstance` to get this.
* Added **opt-in ground snapping for OpenDRIVE-generated traffic signs and lights**, through `adjust_signs_height_to_ground` on `carla.WorldSettings` or `bAdjustSignsHeightToGround` on the traffic-light manager. Off by default; hand-placed actors are never moved.
* Added **environment-map lighting**: `World.set_sky_light_map` replaces the real-time atmosphere capture with an equirectangular HDR panorama, so actors are lit by the environment of the footage or neural scene they are composited into. `clear_sky_light_map` restores the atmosphere and `has_sky_light_map` reports the state. The override survives weather changes but not `load_world()`/`reload_world()`.
* Added a **geometry importer for map authors** (`UGeometryImporter`, Blueprint-callable): GeoJSON polygons, CSV coordinate lists and pixel contours become world-space splines, and OBJ/FBX meshes can be imported into the level from the editor (#8946, #8949, #8950).

#### Sensors and cameras

* Added the **ray-traced lens camera family**, path-traced cameras with real lens models: `sensor.camera.rt_lens` (RGB), `sensor.camera.rt_lens_distance` (float32 Euclidean distance, exact for every lens model unlike `sensor.camera.depth`) and `sensor.camera.rt_lens_instance` (instance/semantic, same byte layout as `sensor.camera.instance_segmentation`). Lens models include an NVIDIA `ftheta` polynomial; `exposure_mode`/`exposure_compensation` give deterministic exposure, and `show_only_tags`, `shadow_catcher_tags` and `show_only_actor_ids` restrict a capture to a subset of actors for compositing. In synchronous mode the image is captured on the same tick (`carla.RTLens.SyncModeBlockingReadback`, on by default).
* Added **fisheye cameras** (`sensor.camera.rgb_fisheye`, `depth_fisheye`, `semantic_segmentation_fisheye`, `instance_segmentation_fisheye`) using the Kannala-Brandt model with configurable distortion coefficients. See `PythonAPI/examples/manual_control_fisheye.py`.
* Added the **V2X sensor family**: a CAM service sensor (`sensor.other.v2x`), a custom binary-payload sensor (`sensor.other.v2x_custom`), a configurable path-loss propagation model and owner-less infrastructure (V2I) sensors.
* Added a **Hybrid Solid-State LiDAR** (`sensor.lidar.hss_lidar`) modelled on the Hesai AT128, with configurable channels, FOV and horizontal resolution.
* Added a **camera profile library**: a `sensor.camera.rgb` without an explicit `post_process_profile` loads `AutomotiveHDR` (`carla.PostProcess.Profile`), matched to the nuScenes front camera, while the viewport and the spectator load `Cinematic` (`carla.PostProcess.ViewportProfile`), so the sensors' look can change without changing what you see. Nine presets ship, each tuned against real frames of a driving dataset: `AutomotiveHDRClean`, `WideHDR8MP` (Zenseact ZOD), `LegacyCCD` (KITTI), `LogHDR` (Cityscapes), `Smartphone` (comma.ai), `GoPro`, `Dashcam` and `Cinematic`. With them come exposure-linked motion blur (`carla.Camera.ExposureMotionBlur`), sensor noise and optical softness that follow the auto exposure, and an FFT lens flare; `manual_control.py` cycles the profiles with K / Shift+K. Your own are authored in the editor and saved as JSON under `Content/Carla/Config/PostProcess` — see `Docs/camera_postprocess_presets.md`.
* Added **`World.get_imu_sensor_gravity`/`set_imu_sensor_gravity`** to read and set the gravity the IMU accelerometer uses.
* Added **`PythonAPI/examples/rig_perf_demo.py`**, a two-pass multi-camera rig benchmark: `measure` records the delivered frame rate of an N-camera rig, `render` replays it into a labelled grid video.

#### Python API

* Added **road network objects** to the Python API: `carla.Road`, `carla.LaneSection` and `carla.Lane`, so the map can be addressed as objects instead of only sampled as waypoints. Reached through `Map.get_roads`, `get_road`, `get_lane_section`, `get_lane`, `get_junctions`, `get_junction_by_id`; `Waypoint.get_lane`, `get_road`; and `Junction.get_connecting_roads`, `get_adjacent_roads`, `get_entry_lanes`, `get_exit_lanes`, `get_entry_waypoints`, `get_exit_waypoints`.
* Added **geolocation that follows the map's own projection**. CARLA used to convert between world coordinates and latitude/longitude with a fixed Mercator formula, which disagreed with the map's OpenDRIVE `geoReference` on anything but Web Mercator. The `geoReference` PROJ string is now read into a typed projection — Transverse Mercator, UTM, Web Mercator or Lambert Conformal Conic — exposed through `Map.get_geoprojection` and accepted by `Map.transform_to_geolocation` and the new `Map.geolocation_to_transform`.
* Added a **wheel animation override** (`Vehicle.set_wheel_animation_override`): the wheels then show only what `set_wheel_steer_direction`, `set_wheel_pitch_angle` and `set_wheel_suspension_offset` set, whether or not Chaos simulates the vehicle, for replays and co-simulation platforms that bring their own wheel state. `get_wheel_steer_angle`, `get_wheel_pitch_angle` and `get_wheel_suspension_offset` read the poses back. Visual only; the setters raise an error while the override is off, and every one of them raises for a wheel the vehicle does not have.
* Added **actor introspection** to the Python API: `Actor.get_component_names`, `get_bone_names`, `get_socket_names` and the world/relative transform getters for each.
* Added **`base_type` and `subtype` blueprint attributes to the props** and `special_type` to the pedestrians, so the blueprint library says what a blueprint actually is — a bench, a traffic cone, a police officer — and can be filtered on it.

#### Ecosystem and integrations

* Added a **choice of ROS 2 middleware**: the native connector sits behind a middleware abstraction supporting FastDDS, CycloneDDS and Zenoh, selected at launch with `--rmw=`. `--ros-domain-id=<N>` sets the domain id (falling back to `ROS_DOMAIN_ID`, then 0), and `ROS2TopicVisibility` sets whether sensor topics are exposed by default.
* Added **stop-sign support and more reliable traffic-light lane linking** to the CARLA-Autoware map generator, plus a traffic-light state bridge for Autoware.
* Added a **Scenic mode to the NuRec hybrid example** (`hybrid_run.py --scenic SCENARIO.scenic`), with five scenarios: highway cut-in, overtake, brake wave, urban stop-and-go and pull-out.

#### Build and tooling

* Added an **agent skill library**: 62 vetted CARLA procedures under `Skills/`, covering setup, the Python API, UE 4.26 / 5.5 / 5.8 builds, packaging, asset imports, Scenario Runner, Leaderboard, Scenic, native ROS 2 and the documentation itself. `Util/AgenticTools/` serves the same library over MCP. See `Docs/agent_skills.md`.
* Added **Ubuntu 24.04** support alongside 22.04, and **NVIDIA RTX 50 series (Blackwell)** support with driver 570+ and CDI-based Docker instructions.
* Added a **Docker-based development environment** for CARLA UE5.
* Added **`CARLA_MAPS_TO_COOK`**, a CMake option taking a `+`-separated list of map package paths, to cook a smaller package.

### Extensions

#### API changes

* **Renamed `carla.Waypoint.get_right_lane`/`get_left_lane` to `get_right_waypoint`/`get_left_waypoint`**, and `carla.GeoOffsetTransform.ApplyTransformation` to `apply_transform`. The old names still work and raise a `DeprecationWarning`.
* **Corrected the pitch and roll rotation order** in the geometry types, and added `carla.Velocity`, `carla.AngularVelocity`, `carla.Acceleration`, `carla.Quaternion` and right-handed vector conversions. Clients that compensated for the wrong order must remove that workaround.
* **`actor.destroy()` now always asks the server**, so a handle returned by `spawn_actor()` reports `is_alive` True until a newer snapshot arrives. Scripts that polled `is_alive` to detect "not yet spawned" must use `world.wait_for_tick()`/`tick()` instead.
* **Renamed the `static.prop.bike helmet` blueprint to `static.prop.bikehelmet`**, the only blueprint id that contained a space.

#### Performance

* **Reworked the quality levels** (`-quality-level=<Low|Medium|High|Epic>`, case-sensitive, Epic by default) on UE 5.8 scalability groups. Every tier now keeps the full view distance, foliage density and simulation content — Low used to cut them, which changed what a sensor saw — and lowering the tier only costs image quality. Epic looks exactly as before; only Epic uses hardware ray tracing on ordinary cameras, and `rt_lens` works on all of them.
* **Multi-camera rigs are much faster**: all raster, fisheye and ray-traced cameras now share one GPU synchronisation per tick and a pooled readback instead of one drain per camera. A 5-camera 1080p rig gains about 30% (9.4 to 12.0 fps per camera), and the marginal cost of an extra camera drops from 19 to 13.4 ms. A single camera is unchanged, and so is the image.
* **Ray-cast LiDARs are much faster** (`sensor.lidar.ray_cast`, `ray_cast_semantic`, `hss_lidar`): they simulate on background tasks that overlap the other sensors' work, and several LiDARs run in parallel. Three semantic LiDARs on Town10 went from 130 to 86 ms per frame. Output is unchanged for a given seed; `carla.Lidar.Batch 0` restores sequential simulation.
* **Optimized the DVS camera** grayscale conversion, about 8x faster in log mode and 4x in linear, with bit-identical output.
* **Made volumetric clouds cheaper** at the Epic and Cine quality levels, with no visible difference (`r.VolumetricRenderTarget.Mode=2`).

#### Python API

* **Extended the recorder and replayer**: the weather is recorded and restored on replay, each vehicle's wheel steer and spin are recorded and replayed (interpolated between frames, so the wheels turn smoothly at any replay rate), a recording can be replayed while another is being recorded, `start_recorder` takes a `stop_replayer` flag, `replay_file` takes a map override and a follow offset, and the replayer can follow traffic signs.
* **Restored `Vehicle.get_telemetry_data()`**, missing since the move to Unreal Engine 5. It returns the last applied control, forward speed, engine RPM, gear and per-wheel slip and angular velocity from the Chaos physics state.
* **Added `World.debug.clear_debug_shape`/`clear_debug_string`** to flush persistent debug shapes and HUD strings, which previously had to be waited out.
* **`client.get_available_maps()` now lists `EmptyMap`**; `OpenDriveMap` and the World Partition sub-levels stay hidden.
* **Spawned actors now carry their blueprint attributes as Unreal actor tags** (`key:value`), so editor tooling and native code can find actors by how they were spawned.
* **Hardened `Vehicle.get_wheel_steer_angle`**: it returns `0.0` with a warning for a wheel the vehicle does not have, and silently for a vehicle with physics disabled, instead of risking the Chaos solver's `check()`.

#### Traffic Manager and agents

* **Improved the Traffic Manager**: lateral control was rebuilt for Chaos's linear steering, with the gains scaled by each vehicle's steering authority so buses and trucks hold their lane through a junction like cars do, and large vehicles take wider turns on top of that (`traffic_manager.vehicle_large_vehicle_wide_turn` and the global toggle). Stuck vehicles recover with a K-turn, parked vehicles are gated, a vehicle that reaches the end of its route holds position, and synchronous mode runs inline without the internal mutex.
* **Modernized the Python navigation agents** with type hints, typed obstacle and traffic-light detection tuples in `agents/tools/hints.py`, and a `BasicAgent.set_destination` that takes an optional start location and queue-clean flag.

#### Sensors and content

* **Clamped `sensor.camera.rt_lens`'s `samples_per_pixel`** to `r.PathTracing.MaxFramePassCount`, with a warning: asking for more samples than the engine can run in a frame used to deliver a partially accumulated, noisier image every frame.
* **Static props now report their semantic tags** (`Static`) through `actor.semantic_tags`, `get_actor()` and `get_actors()`; the segmentation cameras were already right.
* **Disabled Nanite on content meshes with translucent materials** (Town11/Town13 road signs, Town13/15 building glass, bus stops, streetlights). Nanite only supports opaque and masked blends, so those sections rendered with the default material — whole glass towers were missing from Town13 — and spammed `Invalid material` warnings.

#### ROS 2 and ecosystem

* **Upgraded Fast-DDS** from v2.11.2 to the v2.14.6 LTS line on Fast-CDR 2.x, keeping the XCDRv1 wire format so topics stay compatible with every ROS 2 distribution.
* **Restricted the native ROS 2 FastDDS transport to UDPv4 by default.** The builtin UDPv4+SHM default silently delivered no data across a container, uid or version boundary. Set `FASTDDS_BUILTIN_TRANSPORTS=DEFAULT` to restore shared memory.
* **Reworked the ROS 2 native sensor publishers** behind a shared publisher and subscriber template layer, unified the camera and point-cloud publishers, and added an Ackermann control subscriber.
* **`run_carla_autoware.sh --with-rviz`** now starts RViz on a CARLA-adapted config in classical mode too, showing the front camera and following the ego.
* **Updated the `invertedai_traffic.py` example** with ego-vehicle integration and video generation improvements.

#### Build and internals

* **Added NumPy 2 compatibility** by upgrading Boost to 1.90.0, and migrated LibCarla networking off the deprecated `boost::asio` APIs.
* **Moved the NVIDIA DLSS SDK handling into CMake** (`CARLA_DLSS_SDK_PATH`, defaulting to `$DLSS_SDK`, then `~/SDKs/DLSS`, then a fetch; `disabled` opts out), so builds work from any shell and a bad path fails at configure time. The SDK is now a required dependency.
* **Hardened UObject ownership** in the Carla plugin: `TObjectPtr<>` throughout, mesh caches, async heightmap streaming and soft references for catalogue assets.
* **Enabled the LibCarla GoogleTest suite** (server and client) and gated both in CI.
* **Added a Visual C++ Redistributable prerequisite** and troubleshooting note to the Windows quickstart (#9560), and corrected Doxygen parameter names across the LibCarla headers.
* **Added `--no-xodr`, `--no-nav` and `--no-tm` to `carla-pack add`**, which unregister a sidecar of an in-pack map and delete the staged file; omitting a flag keeps the registered file.

#### Documentation

* **Improved the documentation** of many classes and functions with information that was missing or wrong, and documented the Python API that the bindings exposed but the reference never listed — the actor component, bone and socket queries, the ego spawn points and IMU gravity, the geographic projection, the physics control settings, `Client.load_world_if_different`, and the `VehicleTelemetryData`, `WheelTelemetryData`, `Quaternion`, `BoneTransformDataIn`, `BoneTransformDataOut` and `FakeImage` classes.
* **Updated the walker skeleton tutorial** to `carla.WalkerBoneControlIn`, `Walker.set_bones` and `Walker.show_pose`, and fixed the bone names in its skeleton tree.

### Bug fixes

#### Sensors

* Fixed every native sensor (LiDAR, radar, IMU, cameras) simulating only the last physics substep instead of the true elapsed time when `sensor_tick` spanned more than one substep, which halved LiDAR's swept angle and skewed IMU and radar readings.
* Fixed the `noise_seed` attribute of `sensor.lidar.ray_cast` being ignored, so every ray-cast LiDAR used the default seed.
* Fixed the IMU sensor's compass yaw orientation, and the order in which sensors are disposed when their parent actor is destroyed.
* Fixed several legacy UE4-era bugs across LibCarla and the Carla plugin: LiDAR memory reset, DVS validation, camera profiling, image reads, sensor materials and Python sensor teardown.

#### Traffic Manager and agents

* Fixed autopilot vehicles accelerating hard away from stops, junctions and roundabouts, and cutting tight corners too fast; both now stay within a comfortable acceleration bound. Also fixed the Traffic Manager's asynchronous loop drifting off the server frame rate, lights flickering on vehicles the server does not report, and an unbounded command queue on a slow server.
* Fixed a Traffic Manager segfault when reaching a vehicle that is no longer available.
* Fixed the Traffic Manager leaking its whole in-memory road graph — several hundred MB on a large map — every time it shut down or rebuilt its map for a new episode. Its waypoints link to each other with shared pointers, so the cycles kept them all alive; those links are now broken when the map is destroyed.
* Fixed `BehaviorAgent.run_step` raising `AttributeError` once the local planner's waypoint queue empties, for instance at the end of a route.
* Fixed Python navigation agents shifting vehicle-center transforms while checking bumper positions, which understated obstacle distances and could cause premature braking (#7859).
* Fixed the Traffic Manager's localization stage never forgetting the junction a vehicle is entering (`vehicles_at_junction_entrance`) when the vehicle is removed or the Traffic Manager is reset.

#### Python API and client

* Fixed clients failing `get_world()` against a packaged server: `get_required_files` leaked pak-relative paths that escaped the client cache. The server now sends content-relative names and refuses paths that resolve outside the content folder — previously an arbitrary-file-read vector.
* Fixed `from carla.command import ...` by registering the `command` submodule in `sys.modules`.
* Fixed `actor.destroy()` returning False right after `spawn_actor()`, because the client consulted an uninitialised snapshot state and refused to contact the server.
* Fixed `str(carla.WorldSettings(...))` raising `bad_optional_access` when `fixed_delta_seconds` was unset — the default for any settings object that has not pinned the time step. It now prints `fixed_delta_seconds=None`.
* Fixed client snapshot data for OpenDRIVE traffic lights outside junctions, by giving their groups fallback controllers (#9853).
* Fixed actor aliasing on replay caused by reusing actors with the same id, recorder crashes during actor cleanup, and the replayer spawning the wrong blueprints.
* Fixed a `ServerSession::CloseNow` double invocation that let a dropped client evict a still-alive subscriber.
* Fixed an RPC server deadlock on shutdown, an overly strict `Vector3D` assert, OpenDRIVE lane width for centre lanes, busy-wait `yield()` calls and strict `RELEASE_ASSERT`s in road `Map`.

#### Simulation and content

* Fixed the walker bounding box being centred on the actor pivot rather than the skeletal mesh, so `actor.bounding_box` now lines up with the visible pedestrian.
* Fixed the traffic sign bounding box to use the first valid trigger volume and keep its rotation, with a guard for null volumes.
* Fixed `sensor.camera.rt_lens` rendering almost black at night: its auto exposure was pinned to a daylight range, and now follows its camera profile with only the upper end capped (`carla.RTLens.MaxAutoExposureEV`).
* Fixed the bright flash of a newly spawned camera — pressing TAB in `manual_control.py`, for instance — whose auto exposure started from a default and took seconds to adapt. Its first captures are rendered as camera cuts (`carla.Camera.SpawnCameraCuts`), so the exposure is settled from the second image on.
* Fixed the `color` attribute being ignored by the Lincoln MKZ and Mini Cooper, which always spawned black: their meshes imported the car-paint slot under a name `BaseVehiclePawn` does not paint.
* Fixed a potential segfault in `LaneCrossingCalculator` with missing lane marking records, and hardened pedestrian navigation against a null dereference while collecting traffic lights for walker routing.
* Fixed Digital Twin Tool crashes on dense metropolitan OSM data, vegetation spawning inside driving lanes on rural maps, and one-way streets being silently excluded (#9565, #9678).

#### ROS 2

* Fixed camera fx in the ROS 2 `CameraInfo`, large map actor tagging, and camera frame and timestamp consistency.

#### Build and packaging

* Fixed `carla-pack add` dropping existing OpenDRIVE, traffic-light phase, navigation and Traffic Manager sidecars when re-adding an in-pack map without sidecar flags (#9987).
* Fixed the Windows build: the setup installs the MSVC 14.38 toolset the engine fork needs, MSVC builds use `/W4`, and two compile and link errors in LibCarla.
* Fixed the CMake toolchain picking the oldest clang sysroot when an upgraded engine keeps several side by side.

#### Examples and tests

* Fixed `PythonAPI/examples/manual_control.py` raising `NameError: name 'sys' is not defined` instead of their error message when the server cannot send the OpenDRIVE file or the map has no spawn points.
* Fixed `PythonAPI/examples/no_rendering_mode.py` ignoring `--show-triggers`, `--show-connections` and `--show-spawn-points` when a cached map image existed; the flags are part of the cache name now.
* Fixed the spawn check in `test_collision_determinism.py`, which raised `AttributeError` instead of reporting the failure, and `Sensor.get_current_detection_points` in the LiDAR smoke tests.

#### Documentation

* Clarified that `WorldSnapshot.id` identifies the loaded episode and remains stable across its ticks; use `frame` to identify a snapshot's tick (#7377).
* Fixed the Python API reference listing keyword names the bindings do not accept (`World.get_level_bbs(actor_type=...)` raised `ArgumentError`; it is `bb_type`), and the same on `Vehicle.set_autopilot`, `Walker.blend_pose`, `Client.apply_batch_sync`, `Client.get_trafficmanager`, the `Client` recorder methods, `BoundingBox.get_world_vertices` and `World.apply_settings`.
* Fixed the semantic segmentation class table in `Docs/ref_sensors.md`, which still described the 0.8.x 22-class taxonomy instead of the actual 29 classes, breaking ground-truth mapping.
* Fixed code examples that failed when run, in the Python API reference snippets and in `core_actors.md`, `adv_traffic_manager.md`, `ref_sensors.md` and `tuto_G_bounding_boxes.md`.
* Fixed broken links in the documentation: the SYNKROTRON page on the home page, the build guides in the contribution guidelines and the colliders tutorial, and Python API links in three tutorials.
* Fixed typos in `README.md`.

## CARLA 0.10.0

* Unreal Engine migration from version 4.26 to version 5.5
    - Nanite enabled for entire Town 10
    - Lumen powers new realistic illumination
    - Improved lighting with new post-process configuration
    - Physics migrated from PhysX to Chaos
* Remodeled Town 10:
    - New buildings
    - Road topology includes uneven surface, potholes, speed bumps
* Remodeled 13 vehicles with more detail:
    - Nissan Patrol
    - Dodge Charger
    - Dodge Charger Police
    - Ford Crown Taxi
    - Lincoln MKZ
    - Mini Cooper
    - Firetruck
    - CarlaCola
    - Ambulance
    - Mercedes Sprinter
    - Mitsubishi Fusorosa bus
    - Mining earth mover (provided by Synkrotron)
* Rework of build system to use cmake
* Native server support for ROS2 (without CARLA-ROS bridge)
* Refactor of semantic segmentation and instance segmentation sensors
    - Semantic segmentation can now be visualized in UE editor for debugging
    - Only supports meshes with Nanite enabled
* Included support to data-driven realistic traffic generation powered by InvertedAI
* Updated Python API scripts with extra examples in the PythonAPI folder:
    - examples/invertedai_traffic.py
    - examples/interpolate_camera.py
    - examples/vehicle_doors_demo.py
    - examples/vehicle_lights_demo.py
    - examples/visualize_radar.py
    - examples/ros2_native.py
    - util/tools/generate_video_from_frames.py
    - util/apply_texture.py
    - util/change_map_layer.py
    - util/manage_environment_objects.py
    - util/manage_traffic_light.py
    - util/recorder_comparer.py
    - util/show_crosswalks.py
    - util/show_junctions.py
    - util/show_topology.py
    - util/spectator_transform.py
* Motion blur disabled below 1080p resolution to avoid artefacts
* Python API now supports Python 3.8, 3.9, 3.10, 3.11 and 3.12
* Python API support dropped for Python versions 3.7 and lower
* Added support for Scenic 3.0
* RSS functionality removed from docs
* Removed Light Manager from API and docs
* Added Mine01 off-road mining map from Synkrotron
