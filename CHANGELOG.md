## LATEST Changes

### New features

#### Unreal Engine 5.8

* **CARLA now runs on Unreal Engine 5.8**, on CARLA's own fork of the engine. It brings World Partition streaming and Large World Coordinates for the large maps, Lumen global illumination and virtual shadow maps, hardware ray tracing, and the path tracer the ray-traced lens cameras render through.
* Added **Ubuntu 24.04** support alongside 22.04, and **NVIDIA RTX 50 series (Blackwell)** support with driver 570+ and CDI-based Docker instructions.
* The CARLA version is now **1.0**. The NVIDIA install scripts no longer hard-code it: they take it from `CARLA_VERSION_MAJOR/MINOR` in the top-level `CMakeLists.txt`.
* `carla::version()` (`Client.get_client_version()` / `get_server_version()`) now tells builds apart: a release reports its version (`1.0`, from `CARLA_RELEASE_VERSION` or the git tag at HEAD), and a nightly or source build reports the commit it was built from (`1.0-82c52a7c2`, `-dirty` with local changes). The `carla-version` target refreshes it on every build, so the client's version-mismatch warning now catches a client and server built from different commits.

#### Agentic CARLA

* Added an **agent skill library**: 64 vetted CARLA procedures under `Skills/`, covering setup, the Python API, UE 4.26 / 5.5 / 5.8 builds, packaging, asset imports, Scenario Runner, Leaderboard, Scenic, native ROS 2, the documentation itself, and contributing back: issues, discussions and pull requests against `ue58-dev`. `Util/AgenticTools/` serves the same library over MCP. See `Docs/agent_skills.md`.
* Enabled Epic's **Unreal MCP** plugin (`ModelContextProtocol` + `EditorToolset`) in the CarlaUnreal editor: opening the project starts an MCP server on `http://localhost:6256/mcp`, and the repo's `.mcp.json` registers it as `unreal-mcp` so Claude Code connects to the running editor with no setup. Editor only: `-game` and packaged builds do not load it. See `Docs/agent_skills.md`.

#### Sensor simulation

* Added support for **NVIDIA NuRec 26.04**: a recorded real-world drive becomes a scene CARLA can simulate. CARLA owns the physics and the clock, and the NuRec renderer produces photoreal camera frames and lidar sweeps of the reconstructed scene. All the cameras due in a tick are rendered in one batched round trip, the ego and the other vehicles can be re-driven with CARLA physics or the Traffic Manager, and asset editing swaps the rendered asset of a vehicle or inserts a clone with a shifted trajectory. See `Docs/nvidia_nurec.md`.
* Added support for **NVIDIA Cosmos** (Cosmos 2.5, 2.5-AV and Cosmos 3), to turn a recorded run into photoreal variations of itself with a different weather, time of day or road surface: `PythonAPI/examples/nvidia/cosmos/` captures frame-exact multi-camera clips with their depth, segmentation and edge control videos, exports the ground truth as an NVIDIA ClipGT scene package, and submits the generation jobs through the `carla-cosmos` CLI. See `Docs/nvidia_cosmos.md`.
* Integrated **NVIDIA DLSS 4.5** in the simulator: DLSS Super Resolution upscales any camera sensor rendered at a fraction of its output resolution, and DLSS Ray Reconstruction denoises the path-traced frames of the ray-traced lens cameras so they run in real time at low sample counts. See `Docs/core_dlss.md`.
* Added the **ray-traced lens camera family**, path-traced cameras with real lens models: `sensor.camera.rt_lens` (RGB), `sensor.camera.rt_lens_distance` (float32 Euclidean distance, exact for every lens model unlike `sensor.camera.depth`) and `sensor.camera.rt_lens_instance` (instance/semantic, same byte layout as `sensor.camera.instance_segmentation`). Lens models include an NVIDIA `ftheta` polynomial; `exposure_mode`/`exposure_compensation` give deterministic exposure, and `show_only_tags`, `shadow_catcher_tags` and `show_only_actor_ids` restrict a capture to a subset of actors for compositing. In synchronous mode the image is captured on the same tick (`carla.RTLens.SyncModeBlockingReadback`, on by default).
* Added a **camera profile library**: a `sensor.camera.rgb` without an explicit `post_process_profile` loads `AutomotiveHDR` (`carla.PostProcess.Profile`), matched to the nuScenes front camera, while the viewport and the spectator load `Cinematic` (`carla.PostProcess.ViewportProfile`), so the sensors' look can change without changing what you see. Nine presets ship, each tuned against real frames of a driving dataset: `AutomotiveHDRClean`, `WideHDR8MP` (Zenseact ZOD), `LegacyCCD` (KITTI), `LogHDR` (Cityscapes), `Smartphone` (comma.ai), `GoPro`, `Dashcam` and `Cinematic`. With them come exposure-linked motion blur (`carla.Camera.ExposureMotionBlur`), sensor noise and optical softness that follow the auto exposure, and an FFT lens flare; `manual_control.py` cycles the profiles with K / Shift+K. Your own are authored in the editor and saved as JSON under `Content/Carla/Config/PostProcess`; see `Docs/core_camera_presets.md`.
* Added **fisheye cameras** (`sensor.camera.rgb_fisheye`, `depth_fisheye`, `semantic_segmentation_fisheye`, `instance_segmentation_fisheye`) using the Kannala-Brandt model with configurable distortion coefficients. See `PythonAPI/examples/manual_control_fisheye.py`.
* Added a **Hybrid Solid-State LiDAR** (`sensor.lidar.hss_lidar`) modelled on the Hesai AT128, with configurable channels, FOV and horizontal resolution.
* Added the **V2X sensor family**: a CAM service sensor (`sensor.other.v2x`), a custom binary-payload sensor (`sensor.other.v2x_custom`), a configurable path-loss propagation model and owner-less infrastructure (V2I) sensors.
* Added **`World.get_imu_sensor_gravity`/`set_imu_sensor_gravity`** to read and set the gravity the IMU accelerometer uses.
* Added **`PythonAPI/examples/rig_perf_demo.py`**, a two-pass multi-camera rig benchmark: `measure` records the delivered frame rate of an N-camera rig, `render` replays it into a labelled grid video.

#### Maps and content

* Moved the **large maps to World Partition**: Town11, Town12, Town13 and Town15 stream their cells in and out around the vehicles instead of CARLA's old tiling, with no rebasing and nothing tiled on the client side: from the Python API they behave like any other map. Every vehicle keeps the ground under itself loaded, and a vehicle whose `role_name` is `hero` gets a wider ring set by `tile_stream_distance` on `carla.WorldSettings`. A `CarlaLargeMapConvert` commandlet converts legacy tiled towns.
* **Rebuilt weather and night lighting on the UE5 sky**: the weather API drives the sky rig again (maps were near-black at noon), street lamps emit real light in lumens, vehicle headlight beams, moonlight and stars work at night, and rain falls per camera with droplets on the lens.
* **Balanced the vehicle lights across models**: every car uses the same emissive intensity, and each light group (low and high beam, position, fog, brake, reverse, blinkers) is tuned against a reference vehicle. The Dodge Charger and Kia Niro stop borrowing the police Charger's materials, the CarlaCola and firetruck lamps light the right group, the Audi TT's front left blinker no longer lights with the right one, and the Tesla Model 3's blinkers are amber. See `Docs/content_authoring_vehicles.md`.
* Added the **Light Defaults panel** (Window > Light Defaults in the editor): the light and emissive intensity of street lamps, building windows and vehicles, and the colour of street and building lights, are set per class in `Content/Carla/Config/Lights/Defaults.json` and applied to every map. See `Docs/content_authoring_lights.md`.
* Added **environment-map lighting**: `World.set_sky_light_map` replaces the real-time atmosphere capture with an equirectangular HDR panorama, so actors are lit by the environment of the footage or neural scene they are composited into. `clear_sky_light_map` restores the atmosphere and `has_sky_light_map` reports the state. The override survives weather changes but not `load_world()`/`reload_world()`.
* Restored the **classic towns**: Town01 to Town07 are back alongside Town10HD, migrated to the new lighting stack. They ship only as layered maps, so load them by their `_Opt` name (`Town03_Opt`) and toggle their layers with [carla.MapLayer](https://carla-ue5.readthedocs.io/en/latest/core_map/). Town10HD_Opt has no map layers: its sublevels were merged into the persistent level, as sublevels are being phased out on UE 5, and `load_map_layer()` / `unload_map_layer()` have no effect on it. Use `enable_environment_objects()` to hide groups of objects there. A grouping that does not use sublevels will replace map layers in a later release.
* Added **Town_C**, a China-inspired town authored by CARLA consortium member Synkrotron, modelled on the Shanghai International Automobile City in Jiading with recreations of its landmark buildings. It is in the AdditionalMaps pack. See `Docs/map_town_c.md`.
* **OpenStreetMap support is back, and is always built**: a new OSM to OpenDRIVE converter brings back support for OpenStreetMap, free of the SUMO dependency. `carla.Osm2Odr`, `carla.Osm2OdrSettings`, `config.py --osm-path`  all now work out of the box. The `ENABLE_OSM2ODR` CMake option is gone; as it is now built by default.
* **Improved the OpenDRIVE map generator**: a ground plane and terrain that follow road elevation, crosswalks, lane markings through junctions, `lateralProfile <shape>`, PCG street furniture, and a `spawn_custom_mesh` RPC for client-supplied meshes.
* Added a **geometry importer for map authors** (`UGeometryImporter`, Blueprint-callable): GeoJSON polygons, CSV coordinate lists and pixel contours become world-space splines, and OBJ/FBX meshes can be imported into the level from the editor (#8946, #8949, #8950).
* **Custom maps can be used in CARLA again**: a map authored in RoadRunner, or any `.fbx` and `.xodr` pair, can be imported and driven on, in the editor and in a package.
* Added **content packs**: maps, props, vehicles and pedestrians authored in a source build are cooked into a pack with `Util/ContentPacks/carla-pack`, and installing the pack adds them to a CARLA package of the same release without rebuilding or repackaging it. The towns the default package leaves out are distributed as packs too. See `Docs/content_authoring_packs.md`.
* Added the **AdditionalMaps content pack**: the towns the default package does not cook (Town01–07_Opt, Town11, Town13, Town_C, Mine_01, Eixample) are built into one pack by the new `package-additional-maps` target, from the list in `Util/ContentPacks/AdditionalMaps.json` (`CARLA_ADDITIONAL_MAPS_FILE` selects another). The new **`UE5-Weekly` workflow**, run every Sunday at 05:00 UTC and on demand, builds it with the package and the Python API wheels for 3.10 to 3.15, and publishes them to `Linux/Dev/` and `Windows/Dev/` as `CARLA_UE5_Weekly*` (`Deploy.sh`/`Deploy.bat --additional-maps --latest-prefix`), names the nightly `CARLA_UE5_Latest*` never overwrites; every upload now also publishes the package's release metadata. `carla-pack create` takes the map list from a JSON file with `--maps-file`. See `Docs/map_content_packs.md`.
* Added **`CARLA_MAPS_TO_COOK`**, a CMake option taking a `+`-separated list of map package paths, to cook a smaller package.
* Added **opt-in ground snapping for OpenDRIVE-generated traffic signs and lights**, through `adjust_signs_height_to_ground` on `carla.WorldSettings` or `bAdjustSignsHeightToGround` on the traffic-light manager. Off by default; hand-placed actors are never moved.
* **Vehicle wheels now turn when Chaos is not simulating them**: with physics disabled, or while another movement component drives the vehicle, the wheels roll by the distance they travel instead of staying frozen, and they keep their spin when Chaos takes them back. A wheel that moves faster than 150 m/s is treated as teleported and is not turned. Custom vehicles need their animation blueprint reparented to `CarlaVehicleAnimationInstance` to get this.

#### AV stacks and ROS 2

* Added **stop-sign support and more reliable traffic-light lane linking** to the CARLA-Autoware map generator, plus a traffic-light state bridge for Autoware.
* Added a closed-loop example for **NVIDIA Alpamayo 2 Super**: it captures the model's calibrated cameras and ego-motion history, runs the trajectory inference, shows the predicted path and the model's Chain of Causation reasoning, and turns the trajectory into vehicle control, with CARLA-rendered cameras on any map or NuRec cameras on a reconstructed real-world scene. See `Docs/nvidia_alpamayo.md`.
* Added a **choice of ROS 2 middleware**: the native connector sits behind a middleware abstraction supporting FastDDS, CycloneDDS and Zenoh, selected at launch with `--rmw=`. `--ros-domain-id=<N>` sets the domain id (falling back to `ROS_DOMAIN_ID`, then 0), and `ROS2TopicVisibility` sets whether sensor topics are exposed by default.

#### Scenarios

* Added a **Scenic mode to the NuRec hybrid example** (`hybrid_run.py --scenic SCENARIO.scenic`), with five scenarios: highway cut-in, overtake, brake wave, urban stop-and-go and pull-out.

#### Python API

* The Python API builds and runs on **Python 3.10 to 3.15**: list the versions in `CARLA_PYTHON_VERSIONS` to build a wheel for each. In `PythonAPI/examples/requirements.txt`, Python 3.14 and later get `pygame-ce`, a drop-in fork of `pygame`, which has no wheels for them. It is pinned to 2.5.8, a reviewed release. `open3d` is installed up to 3.14 and `invertedai` up to 3.12, the newest versions they support.
* Added **`--tm-port`** to `automatic_control.py`, `manual_control.py`, `visualize_multiple_sensors.py` and `open3d_lidar.py`, as `generate_traffic.py` already had, for when port 8000 is taken.
* Added **road network objects** to the Python API: `carla.Road`, `carla.LaneSection` and `carla.Lane`, so the map can be addressed as objects instead of only sampled as waypoints. Reached through `Map.get_roads`, `get_road`, `get_lane_section`, `get_lane`, `get_junctions`, `get_junction_by_id`; `Waypoint.get_lane`, `get_road`; and `Junction.get_connecting_roads`, `get_adjacent_roads`, `get_entry_lanes`, `get_exit_lanes`, `get_entry_waypoints`, `get_exit_waypoints`.
* Added **geolocation that follows the map's own projection**. CARLA used to convert between world coordinates and latitude/longitude with a fixed Mercator formula, which disagreed with the map's OpenDRIVE `geoReference` on anything but Web Mercator. The `geoReference` PROJ string is now read into a typed projection (Transverse Mercator, UTM, Web Mercator or Lambert Conformal Conic), exposed through `Map.get_geoprojection` and accepted by `Map.transform_to_geolocation` and the new `Map.geolocation_to_transform`. Latitude increases towards the map's north, along CARLA's −y axis.
* Added a **wheel animation override** (`Vehicle.set_wheel_animation_override`): the wheels then show only what `set_wheel_steer_direction`, `set_wheel_pitch_angle` and `set_wheel_suspension_offset` set, whether or not Chaos simulates the vehicle, for replays and co-simulation platforms that bring their own wheel state. `get_wheel_steer_angle`, `get_wheel_pitch_angle` and `get_wheel_suspension_offset` read the poses back. Visual only; the setters raise an error while the override is off, and every one of them raises for a wheel the vehicle does not have.
* Added **deterministic collisions** (`WorldSettings.deterministic_collisions`): by default the physics engine could detect a contact one frame earlier or later depending on how fast the server ran, so the same crash in synchronous mode ended differently between runs. Enabled, collisions repeat exactly, at around 15-25% more physics time with heavy traffic. It applies at the next map load that keeps the settings (`reset_settings=False`). See `PythonAPI/examples/deterministic_collisions.py`; the determinism smoke tests turn it on.
* Added **actor introspection** to the Python API: `Actor.get_component_names`, `get_bone_names`, `get_socket_names` and the world/relative transform getters for each.
* Added **`Vehicle.is_door_open(door)`** to read whether a vehicle's door is open, whoever opened it: another client, a scenario or the replayer. It reads the last tick, like `get_control`, so it costs no call to the simulator. `carla.VehicleDoor` now also exposes `Hood` and `Trunk` (#7701).
* Added **`base_type` and `subtype` blueprint attributes to the props** and `special_type` to the pedestrians, so the blueprint library says what a blueprint actually is (a bench, a traffic cone, a police officer) and can be filtered on it.

#### Build and tooling

* Added a **Docker-based development environment** for CARLA UE5.
* The Linux package now ships the release image's `Dockerfile` at its root, as the UE4 packages did: `docker build -t carlasim/carla:<version> .` from the extracted package builds the image. See `Docs/build_docker.md`.
* Added the manual **`UE5-Release` workflow**: run on a release branch `ue5/<version>` whose head carries the tag `<version>` (e.g. `ue5/1.0` and `1.0`), it builds the package with the AdditionalMaps pack and uploads them to `Linux/`, named as the UE4 releases are, as `CARLA_<version>.tar.gz`, `AdditionalMaps_<version>.tar.gz` and `CARLA_<version>_release-metadata.tar.gz` (`Deploy.sh --release-version`). It also builds the Windows package, with wheels for Python 3.10 to 3.15, and its AdditionalMaps pack, and uploads them to `Windows/` as `CARLA_<version>.zip`, `AdditionalMaps_<version>.tar.gz` and `CARLA_<version>_release-metadata.tar.gz` (`Deploy.bat --release-version`). `Deploy.bat` now publishes the package's release metadata on every upload, as `Deploy.sh` does, and takes `--additional-maps` and `--latest-prefix` as `Deploy.sh` does.
* Added **`CARLA_PYTHON_VERSIONS`**, a CMake option taking a `;`-separated list of Python versions (`3.10;3.11;3.12`) to build the Python API wheel for in a single build. Each interpreter is `python<X.Y>` in `PATH` or `CARLA_PYTHON_<XY>_EXECUTABLE`, and the package ships every wheel. `carla-python-api-install` now installs into the active Python, the `python3` in `PATH`, rather than the one CMake was configured with. It installs the wheel for that Python's version, and fails with the versions available when the build has none for it. The wheel build no longer writes the build's paths into `PythonAPI/carla`, so builds for different Python versions can share a checkout. See `Docs/build_linux_ue5.md`.
* CI now builds the wheels for the Python versions in its `python-versions` input instead of always using Python 3.10. It runs the smoke tests against the package with each version in `smoke-test-python-versions`, through the new `Util/Tools/RunSmokeTests.sh`. The Windows CI takes the same `python-versions` input, each version's interpreter being `py -<X.Y>`. `UE5-Release` builds the wheels for 3.10 to 3.15 on Linux and Windows, and runs the smoke tests with each of them on Linux. See `Docs/dev_testing.md`.
* Added **`Util/Tools/RunPythonVersionTests.sh`**, which tests the Python API once per Python version, each in a fresh virtual environment with its wheel. Its stages are offline checks (imports and example compilation with warnings as errors, plus the unit tests), the smoke tests, and a set of examples run headless.
* `carla-pack install` and `carla-pack verify` on a tarball now report each step and the file being checksummed, instead of staying silent for minutes on a multi-GB pack.

### Extensions

#### API changes

* **Renamed `carla.Waypoint.get_right_lane`/`get_left_lane` to `get_right_waypoint`/`get_left_waypoint`**, and `carla.GeoOffsetTransform.ApplyTransformation` to `apply_transform`. The old names still work and raise a `DeprecationWarning`.
* **Corrected the pitch and roll rotation order** in the geometry types, and added `carla.Velocity`, `carla.AngularVelocity`, `carla.Acceleration`, `carla.Quaternion` and right-handed vector conversions. Clients that compensated for the wrong order must remove that workaround.
* **`actor.destroy()` now always asks the server**, so a handle returned by `spawn_actor()` reports `is_alive` True until a newer snapshot arrives. Scripts that polled `is_alive` to detect "not yet spawned" must use `world.wait_for_tick()`/`tick()` instead.
* **Renamed the `static.prop.bike helmet` blueprint to `static.prop.bikehelmet`**, the only blueprint id that contained a space.

#### Performance

* **Multi-camera rigs are much faster**: all raster, fisheye and ray-traced cameras now share one GPU synchronisation per tick and a pooled readback instead of one drain per camera. A 5-camera 1080p rig gains about 30% (9.4 to 12.0 fps per camera), and the marginal cost of an extra camera drops from 19 to 13.4 ms. A single camera is unchanged, and so is the image.
* **Ray-cast LiDARs are much faster** (`sensor.lidar.ray_cast`, `ray_cast_semantic`, `hss_lidar`): they simulate on background tasks that overlap the other sensors' work, and several LiDARs run in parallel. Three semantic LiDARs on Town10 went from 130 to 86 ms per frame. Output is unchanged for a given seed; `carla.Lidar.Batch 0` restores sequential simulation.
* **Reworked the quality levels** (`-quality-level=<Low|Medium|High|Epic>`, case-sensitive, Epic by default) on UE 5.8 scalability groups. Every tier now keeps the full view distance, foliage density and simulation content (Low used to cut them, which changed what a sensor saw), and lowering the tier only costs image quality. Epic looks exactly as before; only Epic uses hardware ray tracing on ordinary cameras, and `rt_lens` works on all of them.
* **Optimized the DVS camera** grayscale conversion, about 8x faster in log mode and 4x in linear, with bit-identical output.
* **Pedestrians are cheaper to move**: a walker no longer refreshes overlaps on every body of its physics asset each time it moves or animates, and a walker teleported by the client-side crowd or the replayer drops its capsule and trigger boxes from scene queries. Keeping AI walkers and replayed actors visible to sensors (#10068) therefore costs about 4% of the tick with 150 walkers and 40 vehicles on Town10HD instead of 25%, and about 6% while replaying them.
* **Made volumetric clouds cheaper** at the Epic and Cine quality levels, with no visible difference (`r.VolumetricRenderTarget.Mode=2`).

#### Sensors and content

* **Clamped `sensor.camera.rt_lens`'s `samples_per_pixel`** to `r.PathTracing.MaxFramePassCount`, with a warning: asking for more samples than the engine can run in a frame used to deliver a partially accumulated, noisier image every frame.
* **Static props now report their semantic tags** (`Static`) through `actor.semantic_tags`, `get_actor()` and `get_actors()`; the segmentation cameras were already right.
* **Disabled Nanite on content meshes with translucent materials** (Town11/Town13 road signs, Town13/15 building glass, bus stops, streetlights). Nanite only supports opaque and masked blends, so those sections rendered with the default material (whole glass towers were missing from Town13) and spammed `Invalid material` warnings.
* Multi-GPU secondaries now follow the primary's clock and produce the sensor data the primary would, in the same frame. Every frame the primary sends carries its delta time, elapsed and world times and episode settings, and a secondary ticks with exactly that delta: sensor timestamps and `sensor_tick` phases match the primary's, also for sensors created later, where secondaries used to run their own clock and delta time. The frame data now also carries what secondaries rendered or measured differently: actor velocities (IMU gyroscope, radar Doppler), attachments, poses as quaternions, each sensor's noise seed, LiDAR sweep start and IMU and radar history, wheel steering, spin and suspension, open doors, weather changes, map layers, environment objects, scene lights, the day/night cycle, the IMU gravity, ROS 2 TF publishing and each actor's collision, visibility and scale. Mirrored walkers and props no longer move on their own, and traffic lights no longer advance on secondaries. A world generated with `generate_opendrive_world()` is now forwarded to secondaries. Documented in `Docs/core_multigpu.md` under "Frame authority" and "What secondaries mirror".

#### ROS 2 and ecosystem

* **`run_carla_autoware.sh --with-rviz`** now starts RViz on a CARLA-adapted config in classical mode too, showing the front camera and following the ego.
* **Upgraded Fast-DDS** from v2.11.2 to the v2.14.6 LTS line on Fast-CDR 2.x, keeping the XCDRv1 wire format so topics stay compatible with every ROS 2 distribution.
* **Restricted the native ROS 2 FastDDS transport to UDPv4 by default.** The builtin UDPv4+SHM default silently delivered no data across a container, uid or version boundary. Set `FASTDDS_BUILTIN_TRANSPORTS=DEFAULT` to restore shared memory.
* **Reworked the ROS 2 native sensor publishers** behind a shared publisher and subscriber template layer, unified the camera and point-cloud publishers, and added an Ackermann control subscriber.
* **Updated the `invertedai_traffic.py` example** with ego-vehicle integration and video generation improvements.

#### Traffic Manager and agents

* **Improved the Traffic Manager**: lateral control was rebuilt for Chaos's linear steering, with the gains scaled by each vehicle's steering authority so buses and trucks hold their lane through a junction like cars do, and large vehicles take wider turns on top of that (`traffic_manager.vehicle_large_vehicle_wide_turn` and the global toggle). Stuck vehicles recover with a K-turn, parked vehicles are gated, a vehicle that reaches the end of its route holds position, and synchronous mode runs inline without the internal mutex.
* **Modernized the Python navigation agents** with type hints, typed obstacle and traffic-light detection tuples in `agents/tools/hints.py`, and a `BasicAgent.set_destination` that takes an optional start location and queue-clean flag.

#### Python API

* **Extended the recorder and replayer**: the weather is recorded and restored on replay, each vehicle's wheel steer and spin are recorded and replayed (interpolated between frames, so the wheels turn smoothly at any replay rate), a recording can be replayed while another is being recorded, `start_recorder` takes a `stop_replayer` flag, `replay_file` takes a map override and a follow offset, and the replayer can follow traffic signs.
* **Restored `Vehicle.get_telemetry_data()`**, missing since the move to Unreal Engine 5. It returns the last applied control, forward speed, engine RPM, gear and per-wheel slip and angular velocity from the Chaos physics state.
* **Added `World.debug.clear_debug_shape`/`clear_debug_string`** to flush persistent debug shapes and HUD strings, which previously had to be waited out.
* **`client.get_available_maps()` now lists `EmptyMap`**; `OpenDriveMap` and the World Partition sub-levels stay hidden.
* **Spawned actors now carry their blueprint attributes as Unreal actor tags** (`key:value`), so editor tooling and native code can find actors by how they were spawned.
* **Hardened `Vehicle.get_wheel_steer_angle`**: it returns `0.0` with a warning for a wheel the vehicle does not have, and silently for a vehicle with physics disabled, instead of risking the Chaos solver's `check()`.

#### Build and internals

* **Added NumPy 2 compatibility** by upgrading Boost to 1.90.0, and migrated LibCarla networking off the deprecated `boost::asio` APIs.
* **Moved the NVIDIA DLSS SDK handling into CMake** (`CARLA_DLSS_SDK_PATH`, defaulting to `$DLSS_SDK`, then `~/SDKs/DLSS`, then a fetch; `disabled` opts out), so builds work from any shell and a bad path fails at configure time. The SDK is now a required dependency.
* **Hardened UObject ownership** in the Carla plugin: `TObjectPtr<>` throughout, mesh caches, async heightmap streaming and soft references for catalogue assets.
* **Enabled the LibCarla GoogleTest suite** (server and client) and gated both in CI.
* **Added a Visual C++ Redistributable prerequisite** and troubleshooting note to the Windows quickstart (#9560), and corrected Doxygen parameter names across the LibCarla headers.
* **Added `--no-xodr`, `--no-nav` and `--no-tm` to `carla-pack add`**, which unregister a sidecar of an in-pack map and delete the staged file; omitting a flag keeps the registered file.

#### Documentation

* **Improved the documentation** of many classes and functions with information that was missing or wrong, and documented the Python API that the bindings exposed but the reference never listed: the actor component, bone and socket queries, the ego spawn points and IMU gravity, the geographic projection, the physics control settings, `Client.load_world_if_different`, and the `VehicleTelemetryData`, `WheelTelemetryData`, `Quaternion`, `BoneTransformDataIn`, `BoneTransformDataOut` and `FakeImage` classes.
* **Updated the walker skeleton tutorial** to `carla.WalkerBoneControlIn`, `Walker.set_bones` and `Walker.show_pose`, and fixed the bone names in its skeleton tree.
* **Expanded the traffic light and traffic sign authoring pages**: the Traffic Light Tool with an animated build from scratch, its presets, pieces and light atlas; placing a regional `BP_Sign` with its pole, plates and secondary signs; and the regional sign catalog in its three styles.
* **Added a network ports reference** at `Docs/ref_ports.md`, under _Resources_: every port the simulator, the Traffic Manager, ROS 2 and the tooling open, what sets each one, and the two bind errors they produce.
* **The `open-carla-pr` skill now verifies closed issues before opening a new one**: for each matching closed issue, discussion or merged PR it reads how it was closed, checks that the fix reached the user's branch (a fix merged into `ue4-dev` or `ue5-dev` is not in `ue58-dev` unless ported), and tries the fix or workaround on the repro. A solution that works means no new issue; one that fails is cited in the new issue's body.
* **Reorganised the rendering documentation** under _Sensor simulation_: UE native rendering, with a new _Camera features_ section on the post-process presets, the ray-traced lens cameras and DLSS, next to NuRec, Cosmos and the sensor reference.
* **Gave the C++ reference a main page and moved it to Doxygen 1.18.0**: `index.html` now opens on a compact index of the LibCarla namespaces and the Carla plugin's entry classes (`Util/CppReference/mainpage.md`) instead of a blank page, and the `UE5-Doxygen` workflow installs the pinned 1.18.0 release binary instead of Ubuntu's 1.9.8, which drops jQuery: the main page loads 181 KB instead of 302 KB. See `Docs/ref_cpp.md`.

### Bug fixes

#### Sensors

* Fixed **`sensor.camera.rt_lens_instance` labelling the sky `Unlabeled` (0)** instead of `Sky` (11) like the raster segmentation cameras (#10295): camera rays that hit nothing are now tagged `Sky` with id `0xFFFF`. The fix is in the engine's path tracer, CarlaUnreal/UnrealEngine#69.
* Fixed every native sensor (LiDAR, radar, IMU, cameras) simulating only the last physics substep instead of the true elapsed time when `sensor_tick` spanned more than one substep, which halved LiDAR's swept angle and skewed IMU and radar readings.
* Fixed the `noise_seed` attribute of `sensor.lidar.ray_cast` being ignored, so every ray-cast LiDAR used the default seed.
* Fixed the IMU sensor's compass yaw orientation, and the order in which sensors are disposed when their parent actor is destroyed.
* Fixed several legacy UE4-era bugs across LibCarla and the Carla plugin: LiDAR memory reset, DVS validation, camera profiling, image reads, sensor materials and Python sensor teardown.
* Fixed `sensor.other.radar` returning detections beyond its `range` (up to `range / cos(fov/2)`): off-axis rays were longer than the range. Every ray is now exactly `range` long.
* Fixed the V2X CAM message sending vehicle length and width in millimetres. They are now in the ETSI unit of 0.1 m and clamped to the standard's range.
* Fixed **walkers driven by `controller.ai.walker` being invisible to the LiDAR, the radar and `world.cast_ray()`** on the maps where the client runs the navigation (Town01–05, Town10HD) (#10068). `start()` no longer turns the walker's collision off, and `stop()` now gives the walker its movement back, so it obeys `apply_control()` again.
* Fixed **replayed vehicles and pedestrians being invisible to sensors**, including those re-spawned with `replay_sensors=True`: the replayer turned their collision off. They now have query-only collision during the replay, so they still push nothing, and replayed walkers get their movement back when the replay ends.
* Fixed the wheelchair of a wheelchair-user pedestrian being invisible to ray-cast sensors (LiDAR, radar).
* Fixed the editor's `SensorSpawnerActor` crashing with radar point drawing on when it had spawned any sensor other than a radar: `DrawRadarSensorPoints` tested the sensor instead of the radar cast, and dereferenced null for every other sensor.

#### Simulation and content

* Fixed **Ackermann control not starting a vehicle at rest when the physics step is 4 ms or less** (#10291), with `apply_ackermann_control()` and the ROS 2 `ackermann_control_cmd` topic alike: on a fast server or a small `fixed_delta_seconds` the vehicle never moved while the pedal flipped between throttle and brake. The controller now runs at most every 10 ms, on the mean speed over that period; steps of 10 ms and longer behave exactly as before.
* Fixed **walkers on maps with client-side navigation walking slower than their set speed in asynchronous mode** (#10255): the crowd advanced by the delta of the last frame only, losing the time of the frames the client skipped. It now advances by the simulation time elapsed since its previous update, in steps of at most 0.1 s and up to 1 s.
* Fixed **the server stopping answering every client, with no dump, when one client stopped reading a sensor stream in synchronous mode** (#10233). The server waited without a limit for each message to be sent, on the render or game thread, so a `listen()` callback that never returned froze it for good. The wait is now bounded by the 10 s streaming timeout, after which that client's stream is dropped (its client reconnects on its own), and a slow client no longer holds the stream's lock that the other clients' connections need. Unreal's hang detector is also enabled (`[Core.System] HangDuration=120`, not fatal): a stuck game thread now logs its call stack.
* Fixed `WheelPhysicsControl.location`, `old_location` and `velocity` (from `Vehicle.get_physics_control()`) always being zero (#8746): they were copied from Chaos wheel fields that Chaos never updates. They now report the world location of each wheel's centre, in meters like `Actor.get_location()`, suspension travel included, its velocity in m/s (the vehicle body's velocity at that point), and where it was one step earlier. They are read-only, and the documentation no longer calls them relative to the vehicle.
* Fixed `Vehicle.enable_constant_velocity()` making vehicles unable to turn: it overwrote the whole velocity every frame, removing the sideways slip the tyres need to corner and the vertical velocity. It now forces only the velocity component along the requested vector and leaves the perpendicular components to physics (a zero vector still holds the vehicle still), so `ConstantVelocityAgent` and `automatic_control.py --agent=Constant` follow ordinary corners instead of leaving the road.
* Fixed Town_C's vehicle spawn points sitting 4 m above the road, so every spawned vehicle dropped 4 m. They are now 0.5 m above it, as on the other maps.
* Fixed **Town11 still loading as a legacy tiled map** through `ALargeMapManager`: it is now a World Partition map like Town12, Town13 and Town15, and has a pedestrian navmesh, so walkers can spawn and navigate on it. `CarlaLargeMapConvert` no longer fails to save a town whose tiles carry level-script blueprints or redirectors (`Illegal reference to private object`).
* Fixed **walkers falling through the terrain on Town12** when spawned at a point from `get_random_location_from_navigation()` (#10212). The navmesh covers the terrain under some building footprints, and the building's cell could stream in after the walker. `get_random_location_from_navigation()` now only returns points whose surrounding cells are loaded and where a walker fits without overlapping static geometry, and a walker spawned inside a building or wall is refused with `Failed because collision at spawn position`. It also no longer returns the map origin, where a walker falls out of the map, when the navmesh sample finds no polygon.
* Fixed the walker bounding box being centred on the actor pivot rather than the skeletal mesh, so `actor.bounding_box` now lines up with the visible pedestrian.
* Removed the duplicate pedestrian `walker.pedestrian.0044`: it was the same blueprint as `walker.pedestrian.0043` (`BP_EuroF02_B_G2`), so that pedestrian was picked twice as often when walkers are chosen at random. The other pedestrian ids are unchanged; use `walker.pedestrian.0043` in place of `0044`.
* Fixed the traffic sign bounding box to use the first valid trigger volume and keep its rotation, with a guard for null volumes.
* Fixed `sensor.camera.rt_lens` rendering almost black at night: its auto exposure was pinned to a daylight range, and now follows its camera profile with only the upper end capped (`carla.RTLens.MaxAutoExposureEV`).
* Fixed the bright flash of a newly spawned camera (pressing TAB in `manual_control.py`, for instance), whose auto exposure started from a default and took seconds to adapt. Its first captures are rendered as camera cuts (`carla.Camera.SpawnCameraCuts`), so the exposure is settled from the second image on.
* Fixed the `color` attribute being ignored by the Lincoln MKZ and Mini Cooper, which always spawned black: their meshes imported the car-paint slot under a name `BaseVehiclePawn` does not paint.
* Fixed a potential segfault in `LaneCrossingCalculator` with missing lane marking records, and hardened pedestrian navigation against a null dereference while collecting traffic lights for walker routing.
* Fixed Digital Twin Tool crashes on dense metropolitan OSM data, vegetation spawning inside driving lanes on rural maps, and one-way streets being silently excluded (#9565, #9678).
* Multi-GPU secondaries now refuse every call that would change the simulation (`tick`, `apply_settings`, `load_world`, spawning actors and so on) with an error that points to the primary, and report synchronous mode, so `get_world()` on a secondary no longer hangs waiting for a tick. The V2X sensors now stay on the primary like the collision sensor. Fixed a secondary looping forever on a frame with a packet it does not know. A secondary now drops a frame it cannot read and asks the primary for a full resynchronization, and both ends close a connection whose message size is corrupt instead of allocating it. Fixed the primary and the secondaries starting a write on a connection while another one was still in flight, and added TCP keepalive so a dead connection is detected.
* Foliage environment objects keep their ids when the map's environment objects are registered again (after a map layer is loaded or unloaded); they used to get new ids every time.
* Fixed the Multi-GPU secondary adding two worker threads on every reconnect attempt, which leaked threads while the primary was unreachable (#9954).
* Fixed Multi-GPU secondary routing and map reloads: camera streams handed to clients now match the primary's stream ids, secondaries stay in sync across `load_world()`, a lost secondary fails over cleanly and the next secondary to connect takes over all of its sensors' streams (clients resume without a new `listen()` only if that secondary uses the same streaming address and port), a restarted secondary no longer replays an actor onto an unrelated local actor with the same id, a `-nullrhi` primary no longer crashes on camera requests, and a secondary that connects after `load_world()` now loads the primary's current map instead of staying on its default one.
* Fixed the **Sign Generation Controller** of the Digital Twin Tool placing plates edge-on to the traffic (they looked blank) and leaving signs in the roadway. The plate now always stands at _Plate Height Cm_ instead of the `Sign1` socket of `SM_Pole05`, and `DisplaceLocationOutsideNeighboringRoads` snaps the sign to the lane height before measuring, counts parking, biking, shoulder and bidirectional lanes as carriageway, jumps to the outer edge of multi-lane roads on the side of the traffic (right- or left-hand), and removes, with a warning, a sign it cannot move out of the carriageway.
* Fixed the **Traffic Light Tool** preview: it now reframes whenever the traffic light changes size and looks at the heads from a three-quarter front view, instead of starting inside the pole and staying there, and the _Import JSON..._ and _Export JSON..._ buttons no longer show garbage characters.
* Fixed **synchronous mode running frames on its own on a server that had been used before**, which made `deterministic_collisions` runs differ after other scripts. Every `world.tick()` (or `apply_batch_sync(..., True)`) sent while the server was asynchronous was banked and never consumed, and once synchronous mode was turned on the server ran one frame per banked tick without any client ticking. Turning synchronous mode on now starts from no pending ticks, as on a fresh server.
* Fixed `stop_replayer(keep_actors=False)` leaving the replayed actors in the scene, so every play/stop cycle added another copy of them. It destroys the actors the replayer spawned, also when the replay already reached its end; actors it reused from the scene, such as traffic lights, stay.
* The server now names the missing file when a map has no pedestrian navigation mesh (`Failed to find the navigation mesh file 'Map.bin'`) instead of reporting a missing OpenDrive file.

#### ROS 2

* Fixed **the Autoware `/vehicle/status/steering_status` reporting about 1.2 times the real tire angle** (#10294): the vehicle status sensor sent the wheel angle in radians to a steering compensation table keyed on the normalized steer. It now sends the normalized steer, and the reported angle matches the commanded one.
* Fixed **the server crashing with a segmentation fault when its DDS middleware could not start** (#10259), e.g. when `CYCLONEDDS_URI` pointed at the example profile `PythonAPI/examples/ros2/config/cyclonedds.xml` as `Docs/ros2_native.md` suggests. When the middleware fails to start, the server now says so loudly (the DDS reason, the profile in use, and a banner stating that ROS 2 is disabled and nothing will be published), disables ROS 2 for the session and keeps running. The CycloneDDS and Fast-DDS publishers no longer write after a failed `Init()`, and report the dropped messages once. The example profile also drops its `<SharedMemory>` element, which the server's CycloneDDS (built without shared memory) rejected; shared memory is off by default, so subscribers are unaffected.
* Fixed **Autoware's control commands not reaching a `ros2_autoware_control` vehicle when Autoware runs `control_command_gate`**, which replaces `vehicle_cmd_gate` in autoware_launch. The command subscribers were TRANSIENT_LOCAL, and DDS does not match a TRANSIENT_LOCAL reader with `control_command_gate`'s VOLATILE writers, so the vehicle never moved. They are now VOLATILE, which matches both gates.
* Fixed **`/vehicle/status/gear_status` reporting NEUTRAL** while an Autoware-driven vehicle moves forward. Acceleration control never engages the transmission. While it is active, `GetVehicleCurrentGear()` now reports the first forward gear, so Autoware sees DRIVE. `get_telemetry_data()` reports the same gear.
* Fixed camera fx in the ROS 2 `CameraInfo`, large map actor tagging, and camera frame and timestamp consistency.
* Fixed `sensor.camera.instance_segmentation` publishing nothing on ROS 2, `sensor.camera.semantic_segmentation` publishing on an unresolved `semantic_segmentation__` topic, and `sensor.camera.rt_lens` publishing on `rt_lens__` although it has no ROS 2 publisher. The ROS 2 sensor type list had fallen out of step with the sensor registry when the ray-traced lens camera was added; it now has one entry per registry item, checked at compile time.
* Fixed **CycloneDDS subscribers failing to start with the example profile** (`PythonAPI/examples/ros2/config/cyclonedds.xml`, used by `run_rviz.sh --rmw=cyclonedds`) on hosts with the default `net.core.rmem_max` (#10253). Its 16 MB socket receive buffer was a hard minimum, so `rmw_create_node` failed with `failed to create domain` and RViz showed nothing. The 16 MB is now the size requested, capped by the kernel, as in the Fast-DDS profile.

#### Traffic Manager and agents

* Fixed Python local planner crashes after automatic waypoint generation reaches the end of a lane.
* Fixed autopilot vehicles accelerating hard away from stops, junctions and roundabouts, and cutting tight corners too fast; both now stay within a comfortable acceleration bound. Also fixed the Traffic Manager's asynchronous loop drifting off the server frame rate, lights flickering on vehicles the server does not report, and an unbounded command queue on a slow server.
* Fixed a Traffic Manager segfault when reaching a vehicle that is no longer available.
* Fixed the Python client crashing or reading stale snapshots around a map load while a Traffic Manager was alive. Destroying a client during the Traffic Manager's reset segfaulted, the reset ran on the streaming thread (holding back every snapshot for seconds, so `wait_for_tick()` returned frames from before a `set_state()`), and every client alive in the process reset it again, sometimes onto the old episode. The reset now runs once per episode on its own thread, `load_world()` returns after it, and switching the Traffic Manager's synchronous mode no longer races it.
* Fixed the Python client aborting when a vehicle driven by the Traffic Manager in asynchronous mode died under it: destroying it through its own handle (`actor.destroy()` without turning the autopilot off first) or loading a map left the Traffic Manager's worker thread calling the dead actor, and the exception terminated the process. The Traffic Manager now drops such a vehicle as it does any destroyed one, and a failed light query on the streaming thread is logged instead of terminating.
* Fixed the Traffic Manager leaking its whole in-memory road graph (several hundred MB on a large map) every time it shut down or rebuilt its map for a new episode. Its waypoints link to each other with shared pointers, so the cycles kept them all alive; those links are now broken when the map is destroyed.
* Fixed `BehaviorAgent.run_step` raising `AttributeError` once the local planner's waypoint queue empties, for instance at the end of a route.
* Fixed the `BehaviorAgent` tailgating evasion. It re-planned only from the adjacent lane to the waypoint a few meters ahead: the agent never changed lanes, then stopped because its route was gone. It now plans a lane change into the adjacent lane, followed by the route to its original destination. It also no longer raises `AttributeError` when there is no adjacent lane, and it moves left when the lane marking allows changes in both directions.
* Fixed the Traffic Manager's localization stage never forgetting the junction a vehicle is entering (`vehicles_at_junction_entrance`) when the vehicle is removed or the Traffic Manager is reset.
* Fixed the Traffic Manager rebuilding its road graph on Town11 and Town13 from the AdditionalMaps pack although the pack ships their cache: `client.get_required_files()` now also serves a map's OpenDRIVE, navigation and Traffic Manager files from the mounted content pack that declares the map (`<pack>/Content/Maps/{OpenDrive,Nav,TM}/`). A copy in the base content still takes precedence.

#### Python API and client

* Fixed **scripts crashing at exit while callbacks are registered** (segmentation fault or `Fatal Python error: gilstate_tss_set`, about one exit in three on Python 3.14, e.g. `automatic_control.py`): CARLA's client threads kept calling `on_tick` and sensor callbacks into an interpreter that was being finalized. The `carla` module now stops them through an `atexit` hook and waits for the callbacks already running.
* `Client.apply_batch_sync()`, `apply_batch()` and `get_required_files()` now release the GIL while they wait for the server, so sensor callbacks keep running meanwhile.
* Fixed client signal identifier reads exceeding the 32 byte snapshot buffer.
* Fixed clients failing `get_world()` against a packaged server: `get_required_files` leaked pak-relative paths that escaped the client cache. The server now sends content-relative names and refuses paths that resolve outside the content folder, previously an arbitrary-file-read vector.
* Fixed `from carla.command import ...` by registering the `command` submodule in `sys.modules`.
* Fixed `actor.destroy()` returning False right after `spawn_actor()`, because the client consulted an uninitialised snapshot state and refused to contact the server.
* Fixed `is_alive` reporting False right after `spawn_actor()` in asynchronous mode when the client lagged behind the server: a snapshot built before the spawn arrived after it and marked the actor gone. `spawn_actor` now returns the server frame of the spawn, and only snapshots after that frame decide whether the actor exists (#10241).
* Fixed `str(carla.WorldSettings(...))` raising `bad_optional_access` when `fixed_delta_seconds` was unset, the default for any settings object that has not pinned the time step. It now prints `fixed_delta_seconds=None`.
* Fixed client snapshot data for OpenDRIVE traffic lights outside junctions, by giving their groups fallback controllers (#9853).
* Fixed `TrafficLight.set_state()` sometimes not showing in the frame of the `world.tick()` right after it (`test_outside_junction_snapshot` read Green for Yellow in the weekly build). The setters were fire-and-forget, and the server's RPC threads can hand a connection's calls to the game thread out of order, so the tick cue could overtake them. `set_state()`, `set_green_time()`, `set_yellow_time()`, `set_red_time()`, `freeze()`, `reset_group()` and `World.freeze_all_traffic_lights()` now wait for the server, without holding the GIL, and raise an error when the server rejects the call instead of dropping it.
* Fixed actor aliasing on replay caused by reusing actors with the same id, recorder crashes during actor cleanup, and the replayer spawning the wrong blueprints.
* Fixed vehicle doors in the recorder and replayer (#7701). A door that was open when recording started was not recorded, and replaying from a start time lost every door event before it. `apply_physics_control()`, `set_simulate_physics()` and the replayer itself silently closed all open doors and recorded spurious open and close events. `show_recorder_file_info` reported every door event as "opened". Open doors now also survive a vehicle going dormant and waking up again.
* Fixed a `ServerSession::CloseNow` double invocation that let a dropped client evict a still-alive subscriber.
* Fixed the client aborting with `std::terminate` when a light manager with unsent light changes outlived its client: its destructor now logs the error. It also unregisters its tick callback by the right id. An exception thrown by a stream callback is now logged instead of terminating the client.
* Fixed an RPC server deadlock on shutdown, an overly strict `Vector3D` assert, OpenDRIVE lane width for centre lanes, busy-wait `yield()` calls and strict `RELEASE_ASSERT`s in road `Map`.
* Fixed `generate_traffic.py` timing out on Town12 and Town13: the vehicle batch took longer than the hard-coded 10 s client timeout and left the spawned vehicles without a Traffic Manager. The timeout now defaults to 60 s and is set with `--timeout`.
* Fixed combining `carla.MapLayer` values: `carla.MapLayer.Buildings | carla.MapLayer.ParkedVehicles` gave a plain `int`, which `load_world()` and `load_map_layer()` rejected, so the documented multi-layer example failed. `|`, `&`, `^` and `~` between layers now give a `carla.MapLayer`.
* `Sensor.stop()` releases the GIL, so stopping a camera whose frame callback is still running no longer deadlocks the client at exit.

#### Build and packaging

* Fixed Windows deployment to select one `Carla-*-Win64-Shipping.zip` from `Build/Release/Package` and reject missing or ambiguous packages.
* Fixed `carla-pack add` dropping existing OpenDRIVE, traffic-light phase, navigation and Traffic Manager sidecars when re-adding an in-pack map without sidecar flags (#9987).
* Fixed the Windows build: the setup installs the MSVC 14.38 toolset the engine fork needs, MSVC builds use `/W4`, and two compile and link errors in LibCarla.
* Fixed the CMake toolchain picking the oldest clang sysroot when an upgraded engine keeps several side by side.
* Fixed reconfiguring an already-built tree (e.g. `-DENABLE_ROS2=ON`) silently keeping the old options: UnrealBuildTool reused its cached makefile because the `Carla` and `CarlaTools` modules read CMake's generated `Options.def`, `Definitions.def`, `Includes.def` and `Libraries.def` without declaring them, so the rebuild reported success with a plugin that had no ROS 2 in it. Both modules now list those files as `ExternalDependencies`, and a change to any of them re-creates the makefile.
* Fixed `Util/Docker/build.sh` failing to build the development image: the PythonAPI requirements are copied into the build context with their relative layout, so `-r ../carla/requirements.txt` resolves (#10177). The image also installs `libusb-1.0-0`, without which `open3d` cannot be imported.

#### Examples and tests

* Fixed `draw_skeleton.py` hanging on Ctrl+C: an unused `multiprocessing` pool caught the interrupt in its workers. The example now stops on Ctrl+C, stops the camera and walker controller, and destroys its actors.

* Fixed **the agent skills on Windows (Git Bash with a native Python)** and a batch of script bugs found by the weekly package test. `PYTHONPATH` is built with the interpreter's separator and `D:\` paths, `check-carla-ports` falls back to `netstat`/`tasklist` and never reports a busy port as free, `run-carla-ue58-server` runs and stops a standalone package (`CarlaUnreal.exe`) with no checkout, and `install-python-api` searches every configured CARLA root and no longer reports `1.0` against `1.0-<commit>` as a mismatch. Also fixed: `set-world-settings async` hanging on a synchronous world nobody ticks, `read-sensor` and `control-spectator follow` hanging or failing at exit, `replay-recording` reading `/proc` on Windows (and a `stop --clear-scene` for servers that leave replayed actors behind), `create-sensor` and `spawn-vehicles ego` failing on `--ros-frame-id`/`--no-ros-tf` where the attribute does not exist (and the new `--ros-topic-name`), `load-map` not resolving `Town03` to `Town03_Opt`, and evenly spaced `telemetry watch` samples. Plus `spawn-walkers --seconds`, grouped spawn errors in `spawn-vehicles`, and stale paths and examples in the docs.
* Fixed **the Autoware `stop_all.sh` and the launcher teardown leaving the CARLA server running**. They checked only the leader of each process group. The `CarlaUnreal.sh` wrapper exits on SIGTERM, but the engine binary kept running and held the RPC port, so the next launch failed. They now check the whole group and SIGKILL it after the grace period.
* Added `typing_extensions` to `PythonAPI/carla/requirements.txt`. The navigation agents import it, but it was not listed, so `import agents.navigation.basic_agent` failed in a fresh environment.
* Restored wheelchair pedestrians in `PythonAPI/examples/generate_traffic.py`: 11% of the walkers whose blueprint reports `can_use_wheelchair` are now spawned with `use_wheelchair` set, as on `ue4-dev`.
* Fixed `apply_texture.py` to accept RGB diffuse images and update normal or ORM maps without a diffuse texture.
* Fixed `PythonAPI/examples/manual_control.py` raising `NameError: name 'sys' is not defined` instead of their error message when the server cannot send the OpenDRIVE file or the map has no spawn points.
* Fixed `PythonAPI/examples/no_rendering_mode.py` ignoring `--show-triggers`, `--show-connections` and `--show-spawn-points` when a cached map image existed; the flags are part of the cache name now.
* Fixed the `create-scenario` and `write-leaderboard-agent` skills writing their generated files in the system code page on Windows (cp1252), so the templates' em dash made them unreadable as UTF-8. They, and `read-leaderboard-results`' merged results, are now written as UTF-8.
* Fixed the spawn check in `test_collision_determinism.py`, which raised `AttributeError` instead of reporting the failure, and `Sensor.get_current_detection_points` in the LiDAR smoke tests.
* Fixed `PythonAPI/util/config.py --delta-seconds 0` and `--fps 0` (variable frame rate) always failing with a `TypeError`.
* Fixed `PythonAPI/examples/invertedai_traffic.py` failing before driving any traffic (#10221): it loads `Town10HD_Opt` when Inverted AI's `carla:Town10HD` has no plain `Town10HD` map, as on UE 5, and builds the `WaypointManager` with the `lanelet_map` the current `invertedai` SDK requires. `PythonAPI/examples/requirements.txt` pins `invertedai` to that tested commit instead of tracking `develop`.
* Fixed `PythonAPI/examples/lightweather_test.py` crashing with `IndexError` on maps that register no street lights (Town11): it now tests the weather only and skips the light checks. It also moves the spectator to the snapshot camera first, so World Partition maps stream in the scene instead of rendering empty ground.
* The `install-python-api` skill no longer pins `numpy<2` for CARLA 1.0 and later, which support numpy 2: the pin has no wheel for Python 3.13 and later, so the install tried to build numpy 1.26 from source. Releases before 1.0 keep the pin, and `install-scenario-runner` warns about numpy 2 only for them.
* `PythonAPI/util/requirements.txt` installs `pygame-ce` on Python 3.14 and later, where `pygame` has no wheels and failed the whole install, and lists `imageio`, which `util/apply_texture.py` needs.
* `PythonAPI/examples/deterministic_collisions.py` sets physics substepping instead of inheriting it, warns when the server runs frames this client did not tick, and exits with 1 when deterministic repetitions differ. `recorder_replay.py` stops cleanly when stdin closes, and `open3d_lidar.py` and `invertedai_traffic.py` exit with a message when their optional package is not installed.
* Added `imageio` to `PythonAPI/examples/requirements.txt`: `examples/invertedai_traffic.py` imports it, but it was not listed, so the example failed with `ModuleNotFoundError` in a fresh environment.
* Removed `psutil`, `py-cpuinfo` and `python-tr` from `PythonAPI/util/requirements.txt`. They were left over from the removed `performance_benchmark.py`, and no script in `util/` uses them.
* Fixed `Util/Tools/RunPythonVersionTests.sh` running `sensor_synchronization.py` without `--port`, so its `examples` stage failed whenever the server was not on port 2000.

#### Documentation

* Clarified the depth camera's 24-bit distance encoding and the precision of its grayscale visualization.
* **Fixed the CARLA 1.0 download links** in the quick start and the download page, and corrected the ROS 2 native and Autoware pages against a runtime check of the 1.0 package.
* Corrected the coordinate-conventions guide to use a homogeneous change of basis for 4×4 poses.
* Clarified that `WorldSnapshot.id` identifies the loaded episode and remains stable across its ticks; use `frame` to identify a snapshot's tick (#7377).
* Fixed the Python API reference listing keyword names the bindings do not accept (`World.get_level_bbs(actor_type=...)` raised `ArgumentError`; it is `bb_type`), and the same on `Vehicle.set_autopilot`, `Walker.blend_pose`, `Client.apply_batch_sync`, `Client.get_trafficmanager`, the `Client` recorder methods, `BoundingBox.get_world_vertices` and `World.apply_settings`.
* Fixed the semantic segmentation class table in `Docs/ref_sensors.md`, which still described the 0.8.x 22-class taxonomy instead of the actual 29 classes, breaking ground-truth mapping.
* Updated the bounding-box tutorial to skip empty projections before drawing or exporting a box.
* Fixed code examples that failed when run, in the Python API reference snippets and in `core_actors.md`, `adv_traffic_manager.md`, `ref_sensors.md` and `tuto_G_bounding_boxes.md`.
* Fixed broken links in the documentation: the SYNKROTRON page on the home page, the build guides in the contribution guidelines and the colliders tutorial, and Python API links in three tutorials.
* Fixed typos in `README.md`.

## CARLA 0.10.0

### New features

#### Unreal Engine 5.5

* **CARLA migrated from Unreal Engine 4.26 to Unreal Engine 5.5**:
    - Lumen powers new realistic illumination
    - Nanite enabled for the entire Town 10
    - Improved lighting with a new post-process configuration
    - Physics migrated from PhysX to Chaos, with more realistic suspension behaviour for every vehicle
* The documentation of the Unreal Engine 5 version moved to its own address, [carla-ue5.readthedocs.io](https://carla-ue5.readthedocs.io/en/latest/); the Unreal Engine 4.26 documentation stays at [carla.readthedocs.io](https://carla.readthedocs.io/en/latest/).

#### Sensor simulation

* Refactored the semantic segmentation and instance segmentation sensors for stability and performance:
    - Semantic segmentation can now be visualized in the editor for debugging
    - Only meshes with Nanite enabled are supported

#### Maps and content

* Remodeled Town 10:
    - New buildings, pavements, roads and vegetation on the same road network, and new construction assets
    - Road topology with camber and an uneven surface, potholes and speed bumps, which affect vehicle dynamics and sensors
* Added the **Mine_01** off-road open-cast mining map, provided by Synkrotron
* Remodeled 12 vehicles with more detail:
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

#### AV stacks and ROS 2

* Native server support for ROS 2, without the CARLA-ROS bridge, enabled with `--ros2`

#### Scenarios

* Included support for data-driven realistic traffic generation powered by Inverted AI
* Added support for Scenic 3.0, with examples adapted to the new Town 10 in the [CARLA fork of Scenic](https://github.com/carla-simulator/Scenic)

#### Python API

* The Python API supports Python 3.8, 3.9, 3.10, 3.11 and 3.12
* New example scripts in the PythonAPI folder:
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

#### Build and tooling

* Reworked the build system to use CMake

### Removed and not yet migrated

* Python 3.7 and lower are no longer supported
* Removed the Light Manager from the API and the documentation
* Removed the RSS functionality from the documentation
* Motion blur is disabled below 1080p to avoid artefacts
* Not available in 0.10.0, as listed in the [release post](https://carla.org/2024/12/19/release-0.10.0/):
    - Weather control: the weather is fixed to daylight
    - Towns other than Town 10 and Mine_01, large maps and the map layers API
    - OpenDRIVE, OpenStreetMap, digital twin and asset import
    - Vehicles other than the 12 above
    - SUMO, Chrono and PTV Vissim co-simulation
    - V2X and GBuffers
