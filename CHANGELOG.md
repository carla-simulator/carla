## LATEST Changes

* Fixed the fisheye camera family (`sensor.camera.rgb_fisheye`, `depth_fisheye`, `semantic_segmentation_fisheye`, `instance_segmentation_fisheye`) crashing the server on spawn under UE 5.8 (`Assertion failed: IsInRenderingThread()`: the old delivery path locked the GPU readback on a worker thread). They now deliver through the same per-tick batched readback as the pinhole cameras, with their own recycling readback pool.
* Fixed `sensor.camera.rt_lens` leaving `r.RayTracing.NonBlockingPipelineCreation` at 0 for the rest of the process when the sensor was destroyed in synchronous mode (every other ray tracing pipeline in the process then blocked on shader compiles); the sensor restores the value in `EndPlay`.
* Fixed car paint rendering near black under the path tracer (`sensor.camera.rt_lens`): `M_CarPaintMaster` instances carry roughness values up to 1.6, which the raster path clamps but the path tracer integrates literally, so the metal reflected nothing. The master now clamps the roughness to 0.2 on the path-traced side of a `PathTracingQualitySwitch` (`Util/ContentRepair/fix_carpaint_pathtracing.py`; the raster look is unchanged).
* Raised the NuRec hybrid example's sky-light default (`skymap.DEFAULT_INTENSITY`) from 26000 to 160000: the old value was calibrated to the raster ambient term, while the neural sky sits at ~6x the sunlit road's luminance, so path-traced car paint (a mirror of the sky) composited 3-6x too dark.
* Fixed the `color` attribute being ignored by the Lincoln MKZ and Mini Cooper (they always spawned in their default black paint): `BaseVehiclePawn` paints the material slot named `Bodywork_Mat`, and those meshes imported their car-paint slot under another name. The slots are renamed in the content (`Util/ContentRepair/fix_vehicle_paint_slots.py` does it for any vehicle mesh whose paint material derives from `M_CarPaintMaster`).
* Added a Scenic mode to the NuRec hybrid-rendering example (`PythonAPI/examples/nvidia/nurec/hybrid/hybrid_run.py --scenic SCENARIO.scenic`): a [Scenic](https://scenic-lang.org) scenario drives the ego, which carries the recorded camera, and every other agent on the scene's road network (handed to Scenic as sanitized OpenDRIVE); five scenarios ship with it (highway cut-in, overtake and brake wave, urban stop-and-go and pull-out). See `PythonAPI/examples/nvidia/nurec/hybrid/scenic/README.md`.
* Documented the ray-traced lens camera family (`sensor.camera.rt_lens`, `sensor.camera.rt_lens_distance`, `sensor.camera.rt_lens_instance`), `world.set_sky_light_map`/`clear_sky_light_map`/`has_sky_light_map`, `carla.DistanceImage`, and the path-tracer tuning console variables — see `Docs/ref_sensors.md`, `Docs/python_api.md` and `Docs/adv_rendering_options.md#ray-traced-lens-tuning`.
* Added `sensor.camera.rt_lens_distance` and `sensor.camera.rt_lens_instance`, the geometry-AOV twins of `sensor.camera.rt_lens`: a path-traced single-channel float32 Euclidean distance camera (`carla.DistanceImage`, exact for every supported lens model unlike the planar, encoding-lossy `sensor.camera.depth`) and a path-traced instance/semantic camera sharing `sensor.camera.instance_segmentation`'s byte layout (R = semantic label, G/B = 16-bit engine actor id). Both share the full `sensor.camera.rt_lens` lens attribute surface and default to `samples_per_pixel=1`/`enable_denoiser=false` (the AOV is exact and undenoised by construction).
* Added an `ftheta` lens model to `sensor.camera.rt_lens` (`camera_model=ftheta`): the NVIDIA rig f-theta polynomial (θ = Σ cᵢ·rⁱ, up to 8 coefficients via `distortion_coeffs`, r in pixels when `fx = 1/image_size_x`). Requires an explicit `fx`.
* Added `exposure_mode` (`auto`/`manual`) and `exposure_compensation` (EV) attributes to `sensor.camera.rt_lens`, for deterministic exposure when compositing path-traced frames against external renders.
* Added `show_only_tags`, `shadow_catcher_tags` and `show_only_actor_ids` attributes to the ray-traced lens camera family, restricting a capture to a subset of actors (plus, for `shadow_catcher_tags`, the actors that should receive their cast shadows) for compositing.
* Clamped `sensor.camera.rt_lens`'s `samples_per_pixel` to the console variable `r.PathTracing.MaxFramePassCount`, logging a warning when it clamps: a sensor requesting more samples than the engine can run in one frame previously delivered a partially accumulated (and therefore noisier) image every frame instead of a smaller, fully converged one.
* Added the `carla.RTLens.SyncModeBlockingReadback` console variable (default on): in synchronous mode, the ray-traced lens camera family now delivers the image captured on the *same* tick (a batched, blocking GPU readback shared by every such camera that tick) instead of the previous tick's frame; asynchronous mode is unchanged (non-blocking, one tick of latency).
* Corrected Doxygen parameter names in LibCarla headers to match their function signatures.
* Silenced the ~58 `Invalid duplicate copies of ExternalActor … ActorFolder_UID_…` warnings logged on every editor start and cook of the World Partition towns. The content bundle ships its `__ExternalActors__`/`__ExternalObjects__` trees inside its own folder and mounts them under the names the levels reference, so each file is reachable under two package names; the engine dropped the duplicate external *actors* silently but had no equivalent rule for external *objects*. Requires the matching engine commit (`AssetRegistry.cpp`: `ShouldSkipGatheredAsset` validates the external-objects folder like the external-actors folder).
* Fixed `actor.destroy()` returning False (`attempting to destroy an actor that is already dead`) when called right after `spawn_actor()`: the client refused to contact the server whenever the actor was missing from the last world snapshot, and the snapshot state it consulted was uninitialised. `destroy()` now always asks the server. Related behaviour change, documented on `carla.Actor.is_alive`: a handle returned by `spawn_actor()` reports `is_alive`/`is_active` True (not False or garbage) until a snapshot newer than the spawn arrives; any newer snapshot is authoritative, and handles from `get_actor()`/`get_actors()` keep reflecting snapshots only. Scripts that polled `is_alive` to detect "spawn not yet visible" must use `world.wait_for_tick()`/`tick()` instead.
* Spawned static props (`static.prop.*`, `static.prop.mesh`, and props from content packs) now report their semantic tags (`Static`) in `actor.semantic_tags`, `get_actor()` and `get_actors()`; they were tagged after being registered, so the API always returned an empty list while the segmentation cameras were right.
* `client.get_available_maps()` now lists `EmptyMap` (an empty OpenDRIVE file ships with it so `world.get_map()` works); `OpenDriveMap` and the World Partition sub-levels stay hidden.
* Fixed World Partition maps (Town12, Town13, Town15) loading with no actors — empty World Partition window, no streaming cells, vehicles falling through the world — when the CARLA content bundle is cloned into a subfolder of `Content` (the standard layout): the engine resolves one-file-per-actor packages at the mount root (`/Game/__ExternalActors__/...`), while the bundle ships them under `Content/Carla/__ExternalActors__/...`. The Carla module now registers a package mount point per bundle at startup so the names the levels reference resolve to the files the bundle ships, with no content relocation. Contributed by Guillermo Lopez Diest.
* Moved the NVIDIA DLSS SDK handling into CMake: configure resolves the SDK (new `CARLA_DLSS_SDK_PATH` option, defaulting to the `DLSS_SDK` environment variable, then `~/SDKs/DLSS`, fetching from NVIDIA's public repository as a last resort; `disabled` opts out) and injects `DLSS_SDK` into every Unreal build, package, and launch command, so `cmake --build`/`launch` work from any shell without `CarlaSetup.sh`'s exports and a bad SDK path fails at configure time instead of mid-build.
* Restricted the native ROS2 FastDDS middleware to the UDPv4 transport by default: Fast-DDS's builtin UDPv4+SHM default silently delivers no data (while discovery keeps topics visible) whenever the shared-memory path crosses a container, uid, or Fast-DDS-version boundary, and never falls back to UDP. Subscribers in unconfigured containers now work out of the box; set `FASTDDS_BUILTIN_TRANSPORTS` (e.g. `DEFAULT`) to restore shared memory for same-host, same-uid setups. The active transport choice is logged at startup.
* Upgraded the bundled eProsima Fast-DDS used by the native ROS2 integration from v2.11.2 to the v2.14.6 LTS line, moving serialization onto Fast-CDR 2.x while keeping the classic XCDRv1 wire format so published topics stay compatible with every ROS2 distribution.
* Added a `--ros-domain-id=<N>` server option to set the ROS2 domain id at startup. The value (0 to 232) is stored in the middleware abstraction layer and honored by every middleware (FastDDS, CycloneDDS, and Zenoh), so future middlewares pick it up automatically. When the option is omitted, the server falls back to the `ROS_DOMAIN_ID` environment variable, and then to the default domain 0. The resolution order is: `--ros-domain-id`, then `ROS_DOMAIN_ID`, then 0.
* Added Zenoh as a third native ROS2 middleware, selectable at server launch via `--rmw=zenoh` and compatible with `rmw_zenoh_cpp` peers.
* Decoupled the native ROS2 connector from a hard FastDDS dependency into a strategy-pattern middleware abstraction supporting both FastDDS and CycloneDDS, selectable at server launch via the `--rmw=` command-line flag.
* Integrated the follow-up UE 5.8 migration branches (`ue58-dev-carla`, `ue58-mapgen-features`, `ue58-migration-02/-03`) into `ue58-dev`, which is now the integration branch for all UE 5.8 migration work. Highlights of the integrated stack:
  * **Large maps on World Partition**: legacy tiled towns (Town12) migrated to native World Partition streaming with double-precision Large World Coordinates; a `CarlaLargeMapConvert` commandlet converts legacy tiled towns in place; hero vehicles get streaming sources so the ground under them always loads; World Partition streaming crashes fixed in traffic lights, the actor registry (weak actor handles), the LiDAR, and semantic tagging of streamed cells.
  * **Weather and night lighting**: the weather API reconnected to the UE5 sky rig (fixes near-black maps at noon), height-fog parameters remapped, night lighting reworked end to end (street lamps emit real light under 5.8, ambient moon/skylight floor, vehicle headlight beams, night sky stars), and the street light API exposed in `manual_control`.
  * **Traffic Manager and vehicle control**: linear Chaos steering input with a reimplemented `GetWheelSteerAngle`, lateral PID retuned for UE5 (0.9.12 gains restored), stuck-vehicle K-turn recovery, parked-vehicle gating, TM speed seeding, route-end hold, pursuit-distance cap, and a deterministic TM navigation A/B benchmark (`PythonAPI/test/tm_benchmark.py`).
  * **Generated OpenDRIVE maps**: real ground plane and terrain heightfield following road elevation, crosswalk zebras, lane markings through splice junctions, OpenDRIVE `lateralProfile <shape>` support, median/carriageway gap filling, PCG street furniture with working night lighting, and a new `spawn_custom_mesh` RPC for client-supplied runtime meshes.
  * **Rendering**: Lumen switched to hardware ray tracing with ray-traced shadows, path-traced sensors keep their cache (viewport on software Lumen + VSM), DLSS-RR denoising re-enabled for the path tracer, rt_lens auto exposure pinned to the daylight histogram window, Vulkan bindless descriptor heap grown to 2M entries, and the Nanite cluster streaming pool tripled for the large World Partition towns.
  * **NuRec**: the NVIDIA NuRec example suite ported to CARLA UE5 / NRE 26.04 with installer, demo config, and runner script.
  * Plus assorted fixes: readable error when the RPC port is already bound, listen sockets marked close-on-exec, walkers restore speed control and report real velocity, recorder-replay light-cache fixes, vehicles no longer left undrivable after a physics toggle or replay, and Recast OBJ export sanitizes corrupt vertices that hung navmesh builds.
* Fixed the server segfaulting on `client.load_world('Town12')`: `ALargeMapManager` dereferenced the null `UWorldComposition` on World Partition maps in `BeginPlay` (superseded on this branch by the full World Partition migration above, which removes the legacy tile manager on such maps entirely).
* Disabled Nanite on the remaining content meshes that paired Nanite with translucent materials (Town11/Town13 generated road signs, legacy Town13/15 building glass, bus stops, streetlights): Nanite only supports opaque/masked blend modes, so those sections rendered with the engine default material -- entire glass towers were missing from Town13's downtown -- and spammed `Invalid material` warnings. Full-map sweeps of Town12/13/15 now log zero warnings.
* Ported the CarlaUnreal project, plugins, and build system to Unreal Engine 5.8 (engine branch `ue58-dev-carla`): updated the removed/renamed 5.8 APIs across the Carla, CarlaTools, and StreetMap plugins, kept `-Wshadow` at warning level, and adapted the toolchain to the 5.8 clang sysroot layout (libc++ now ships inside the sysroot instead of `ThirdParty/Unix/LibCxx`).
* Fixed the CMake toolchain picking the oldest clang sysroot when an in-place-upgraded engine keeps several SDKs side by side: candidates are now walked newest-first preferring one that ships `libc++.a`, with a newest-first fallback for UE <= 5.5 layouts.
* Fixed packaging under UE 5.8: `BuildCookRun` now passes `-pak -iostore` (the 5.8 cook writes to the Zen store, so staging otherwise shipped a binaries-only package), and the archive-cleanup step -- which had never run due to a variable-name mismatch -- now also purges stale `ue.projectstore` files that would make the packaged binary fail to start.
* Renamed the translucency scalability CVars deprecated in UE 5.7 (`r.TranslucencyLightingVolumeDim`/`r.TranslucencyVolumeBlur` to `r.TranslucencyLightingVolume.Dim`/`.Blur`); the deprecation ensures previously failed the cook.
* Fixed clients failing `get_world()` against a packaged server: `get_required_files` leaked pak-relative paths (`../../../CarlaUnreal/Content/...`) that escaped the client cache. The server now sends content-relative names, the client `FileTransfer` contains every server-supplied path inside its cache root, and the server's `request_file` refuses names that resolve outside the content folder (previously an arbitrary-file-read vector).
* `CarlaSetup.sh`/`.bat` now clone the `ue58-dev-carla` engine and content branches, and the NVIDIA DLSS SDK became a required build dependency (fetched automatically from NVIDIA's public repository; opt out explicitly with `DLSS_SDK=disabled`) since the rt_lens denoiser and DLSS upscaling depend on it.
* Fixed `sensor.camera.rt_lens` ghosting stale emissives (e.g. traffic light lamps keeping the previous color lit) after material parameter changes: the path tracer now clears the DLSS-RR temporal denoiser history when a material parameter invalidation fires, while keeping it across motion-driven restarts.
* Added the `CARLA_MAPS_TO_COOK` CMake configure option to select which maps the `package` targets cook. Set it to a `+`-separated list of package paths (e.g. `/Game/Carla/Maps/Town10HD_Opt`) to produce a smaller package with only those maps; leave it empty (the default) to cook the full map list from `DefaultGame.ini`.
* Fixed `from carla.command import ...` on the Python API by registering the `command` submodule under `carla.command` in `sys.modules` (the ue5 wheel ships `carla` as a bare extension module, so the previous `libcarla.command` namespace was not importable).
* Modernized the Python navigation agents with type hints and code cleanup ported from `ue4-dev`, added typed obstacle/traffic-light detection-result tuples in `agents/tools/hints.py`, reworked `BasicAgent.set_destination` to accept an optional start location and queue-clean flag, and fixed obstacle detection treating an empty `vehicle_list` as "all vehicles".
* Updated the `invertedai_traffic.py` example with InvertedAI quality-of-life improvements for ego-vehicle integration and video generation.
* Fixed `Sensor.get_current_detection_points` in the LiDAR smoke tests so it remains a valid instance method.
* Added fisheye camera sensors (`sensor.camera.rgb_fisheye`, `sensor.camera.depth_fisheye`, `sensor.camera.semantic_segmentation_fisheye`, `sensor.camera.instance_segmentation_fisheye`) using the Kannala-Brandt projection model. Each variant captures up to 6 cubemap face render targets (the back face is skipped unless the FOV or equirectangular mode requires it) and composites them through a custom HLSL shader (`Plugins/Carla/Shaders/WideAngleLens.usf`) using configurable distortion coefficients. See `PythonAPI/examples/manual_control_fisheye.py` for an interactive demo. Ported from ue4-dev.
* Fixed the Windows build: the setup now installs the MSVC 14.38 toolset required by the Unreal Engine 5.5.4 fork, MSVC builds use `/W4` instead of `/Wall`, and two Windows compile/link errors in LibCarla were fixed.
* Added a Hybrid Solid-State LiDAR sensor (`sensor.lidar.hss_lidar`) modelled on the Hesai AT128, with configurable channels, horizontal/vertical FOV, and horizontal resolution. Reuses the rotating LiDAR's intensity, drop-off, and noise model.
* Documented left-hand traffic (LHT) support in `Docs/core_map.md`, covering the OpenDRIVE `rule="LHT"` road attribute, the RHT default, and the impact on traffic signs and signals.
* Fixed three latent bugs in the `FPixelReader` GPU pixel-readback path: an inverted validity guard in `SendPixelsInRenderThread` that discarded every valid sensor's frame, the readback-wait task being scheduled onto a render-pipeline thread (deadlocking game/render/RHI threads), and `FRHIGPUTextureReadback::Lock` truncating the payload to a single row by aliasing its row-pitch out-parameter.
* Fixed ServerSession::CloseNow double-invocation on ue5-dev by guarding with std::atomic_bool _is_closed so the close path runs exactly once, preventing a dropped client from evicting a still-alive subscriber (port of ue4-dev fix #9740)
* Added NumPy 2 compatibility to the PythonAPI by upgrading Boost to 1.90.0, which ships the upstream NumPy 2 C ABI fix. LibCarla networking migrated from `boost::asio::deadline_timer` to `boost::asio::steady_timer`, and the deprecated `io_service`, `address::from_string`, `resolver::query`, `buffer_cast`, and `io_context::work` APIs replaced across LibCarla and the CarlaTools plugin.
* Added `Vehicle.get_telemetry_data()` returning `VehicleTelemetryData` with last-applied control, forward speed, engine RPM, current gear, and per-wheel lateral slip, longitudinal slip, and angular velocity sourced from the Chaos vehicle physics state.
* Added a GeoProjection engine that parses the OpenDRIVE `geoReference` PROJ string into a typed cartographic projection (Transverse Mercator, UTM, Web Mercator, or Lambert Conformal Conic) and exposes it via `Map.get_geoprojection`. `Map.transform_to_geolocation` and `Map.geolocation_to_transform` now accept an optional projection argument together with the new `carla.GeoProjectionTM`/`GeoProjectionUTM`/`GeoProjectionWebMerc`/`GeoProjectionLCC2SP` and `carla.GeoEllipsoid` types, replacing the previous fixed Mercator conversion.
* Added support for parsing offsets from OpenDRIVE using optional offset transforms.
* Added `World.get_imu_sensor_gravity` and `World.set_imu_sensor_gravity` Python APIs to read and configure the gravity constant used by the IMU sensor's accelerometer.
* Fixed recorder crashes during actor cleanup and replayer spawning incorrect actor blueprints
* Fixed walker bounding box being centred on the actor pivot instead of the skeletal mesh: the box reported by `actor.bounding_box` now reflects the mesh component's offset from the character root, so pedestrian boxes line up with the visible mesh.
* Added `Actor.get_component_world_transform`, `Actor.get_component_relative_transform`, `Actor.get_bone_world_transforms`, `Actor.get_bone_relative_transforms`, `Actor.get_component_names`, `Actor.get_bone_names`, `Actor.get_socket_world_transforms`, `Actor.get_socket_relative_transforms`, and `Actor.get_socket_names` Python APIs for introspecting an actor's scene components, skeletal bones, and sockets by name.
* Added `World.debug.clear_debug_shape` and `World.debug.clear_debug_string` Python APIs to flush persistent debug shapes and HUD strings drawn from the client.
* Added `Map.geolocation_to_transform` Python API that maps a `GeoLocation` back to a world-space `Location`, the inverse of `Map.transform_to_geolocation`.
* Reworked the sensor render pipeline and quality tiers by pooling GPU readbacks, gating GBuffer capture on listeners, adding a per-camera ray-tracing toggle, and introducing four server launch tiers (Low, Medium, High, Epic) selectable via `-quality-level=<Tier>` (case-sensitive, Epic by default). Each tier applies a coherent CVar configuration at engine init that persists across runs without manual `GameUserSettings.ini` cleanup.
* Added weather recording and replay, simultaneous record-and-replay, `stop_replayer` flag on `start_recorder`, `map_override` and follow-offset arguments on `replay_file`, and traffic-sign follow targets in the replayer (ported from ue4-dev)
* Reworked the ROS 2 native sensor publishers behind a shared publisher and subscriber template layer, unified the camera and point-cloud publishers, and added an Ackermann control subscriber so vehicles can be driven from ROS 2 Ackermann messages.
* Added `carla.Velocity`, `carla.AngularVelocity`, `carla.Acceleration`, `carla.Quaternion`, and right-handed vector conversions to the geometry types, and corrected the pitch and roll rotation order. Clients that previously compensated for the incorrect rotation will need to remove that workaround.
* Added a `ROS2TopicVisibility` startup flag that sets whether sensor topics are exposed by default when the server launches.
* Added the V2X sensor family: a CAM service sensor (`sensor.other.v2x`), a custom binary-payload sensor (`sensor.other.v2x_custom`) with channel selection and multiple messages per frame, a configurable path-loss propagation model, and owner-less infrastructure (V2I) sensors.
* Fixed the IMU sensor compass yaw orientation and corrected the order in which sensors are disposed when their parent actor is destroyed.
* Hardened pedestrian navigation against a null dereference when collecting the traffic lights used for walker routing.
* Hardened UObject ownership in the Carla plugin by migrating UPROPERTY raw pointers to `TObjectPtr<>`, adding mesh caches, enabling async heightmap streaming, and converting catalog assets to soft references.
* Corrected the Semantic Segmentation camera class table in `Docs/ref_sensors.md` to match the actual 29-class taxonomy defined in `ObjectLabel.h` and `CityScapesPalette.h`. The previous table reflected the legacy 0.8.x CityScapes taxonomy (22 classes), which caused mismatches between documentation and engine output. This update aligns the documentation with the true engine enum values and RGB palette, preventing ground-truth mapping errors when building perception pipelines.
* Fixed several legacy UE4-era bugs across LibCarla and the Carla plugin affecting lidar memory reset, DVS validation, camera profiling, image reads, sensor materials, and Python sensor teardown.
* Improved Traffic Manager handling of large vehicles at junctions, including wide-turn behaviour and a smoothed lateral offset profile
* Made the Traffic Manager large-vehicle wide turn safer by scaling the lateral offset to the vehicle's length, limiting the inboard cut-in that pushed long vehicles into the inside shoulder, suppressing the swing when the side it moves into is occupied, and adding `traffic_manager.vehicle_large_vehicle_wide_turn` and `traffic_manager.global_large_vehicle_wide_turn` toggles
* Improved Traffic Manager PID controller using a direct angle to the target waypoint and smoother waypoint interpolation
* Removed Traffic Manager internal mutex and condition variables; synchronous mode now executes inline
* Avoid actor aliasing on replay caused by reusing actors with the same id
* Fixed segfault in traffic manager when trying to access not available vehicles
* Enabled the LibCarla GoogleTest suite (server + client) on ue5-dev and gated both in CI.
* Fixed Digital Twin Tool crashes on dense metropolitan OSM data, vegetation spawning inside driving lanes on rural maps, and one-way streets being silently excluded from generation (#9565, #9678)
* Added Ubuntu 24.04 support alongside Ubuntu 22.04
* Added NVIDIA RTX 50 series (Blackwell) support with driver 570+ and CDI-based Docker instructions
* Fixed compiler warnings across 20 LibCarla files including signed/unsigned conversions, pessimizing moves, deep copies in range-for loops, and C-style casts (ported from ue4-dev)
* Fixed camera fx calculation in ROS2 CameraInfo, large map actor tagging, pedestrian bounding box using skeletal mesh bounds, and camera frame/timestamp consistency (ported from ue4-dev)
* Fixed RPC server deadlock on shutdown, overly strict Vector3D assert, OpenDrive lane width for center lanes, busy-wait yield() calls, and strict RELEASE_ASSERTs in road Map (ported from ue4-dev)
* Fix typos in README.md
* Added actor description as Actor TAGs
* Create class with functions to import points and polylines from satellite segmentation (#8946, #8949 #8950)
* Added Visual C++ Redistributable prerequisite and troubleshooting note to Windows quickstart guide (#9560)
* Added Docker-based development environment for CARLA UE5
* Fixed potential segfault in LaneCrossingCalculator by adding a nullptr guard for missing lane marking records
* Fixed traffic sign bounding box returned through the Python API to use the first valid trigger volume and preserve its rotation, with a guard against null trigger volumes

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
