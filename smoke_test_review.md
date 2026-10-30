# Smoke test review

Every module in `PythonAPI/test/smoke`, run one module per `nose2` invocation against the `ue58-dev` editor binary in `-game` mode on port 3654.
An uncooked tree, so every map is available; each module capped at 900 s, `SIGINT` first so its `tearDown` runs.

The first full pass was **38 modules, 103 tests, 67 minutes**, with 6 modules failing.
`smoke_test_list.txt` named 37 of the 38: `test_encoding_cameras` existed but was never run by CI.

After the work below the suite is **37 modules, 95 tests**, and `test_encoding_cameras` is in the list.

## Results

Times are from the first pass. Rows marked *trimmed* have been cut since and are faster now.

| Module | Tests | What it covers | Time | First pass | Now |
| --- | --- | --- | --- | --- | --- |
| `test_client` | 1 | Client and server version strings match. | 28 s | OK | OK |
| `test_sync` | 4 | Map reload in sync mode, camera and sensor transforms per tick, `apply_batch_sync`. | 170 s | OK | OK |
| `test_world` | 1 | `fixed_delta_seconds` honoured across six values. | 19 s | OK | OK |
| `test_blueprint` | 1 | Every blueprint id matches `type.subtype.name`. | 19 s | **FAILED** | **OK** — fixed |
| `test_map` | 2 → 1 | Loads every map and walks waypoints, topology, geo-reference, `to_opendrive`. | 65 s | **FAILED** | **OK** — fixed |
| `test_spawnpoints` | 1 | Every spawn point accepts every vehicle. | 95 s | **FAILED** | **OK**, 38 s — fixed, *trimmed* |
| `test_snapshot` | 1 | 20 vehicles, snapshot frame and per-actor transforms. | 38 s | OK | OK — was flaky, fixed |
| `test_props_loading` | 1 | Spawns every `static.prop.*`. | 19 s | **FAILED** | **OK** — fixed |
| `test_actor_introspection` | 4 | Vehicle components and sockets, walker bones, error on unknown component. | 153 s | OK | OK |
| `test_collision_sensor` | 1 | Drives every vehicle into a wall, expects a collision event. | 29 s | OK | OK |
| `test_debug_clear` | 3 → 1 | `clear_debug_shape` / `clear_debug_string`, including with nothing drawn. | 114 s | OK | OK, *trimmed* |
| `test_streamming` | 1 | One GNSS sensor, five client threads subscribing to the same stream. | 930 s | **HUNG** | **FAILED** fast — bug open, *capped* |
| `test_geoconversion` | 16 | Geo-reference, projection and round-trips for plain, TM, UTM N/S, Web Mercator, Lambert. | 304 s | OK | OK, *trimmed* |
| `test_road_lane_api` | 14 | Roads, junctions, lanes, waypoint round-trips, neighbours, lane graph. | 270 s | OK | OK, *trimmed* |
| `test_traffic_light` | 1 | Traffic-light state in a snapshot outside a junction. | 25 s | OK | OK |
| `test_walker_bounding_box` | 1 | Walker bounding box carries a relative offset. | 19 s | OK | OK |
| `test_walker_navigation` | 1 | `WalkerAIController` routes walkers without crashing. | 20 s | OK | OK |
| `test_recorder` | 2 | Blueprint id round-trips through a recording; destroy on the same tick. | 67 s | OK | OK |
| `test_replay_no_actor_aliasing` | 1 | Replayed actors do not inherit another actor's state. | 21 s | OK | OK |
| `test_determinism` | 1 | 100 autopilot vehicles, 1000 ticks, run twice, positions compared. | 126 s | OK | OK |
| `test_collision_determinism` | 4 → 2 | Scripted collisions replayed 5 times each. | 523 s | **FAILED** | *trimmed* — see below |
| `test_sensor_determinism` | 1 | All raycast sensors over a scripted run, twice, output compared. | 19 s | OK (skipped) | **FAILED** — accepted, not tracked |
| `test_lidar` | 11 | Point counts sync/async, comparison, batch consistency, noise seed, fixed seed, `sensor_tick`, semantic tags. | 220 s | OK | OK |
| `test_dvs` | 3 | DVS log and linear event generation, threshold reduces events. | 70 s | OK | OK |
| `test_encoding_cameras` | 3 | Depth encoding stable, semantic labels valid, instance ids unique. | 1 s | OK | OK — **added to CI** |
| `test_sensor_tick_time` | 1 | Every sensor with `sensor_tick` fires the expected number of times. | 25 s | OK | OK |
| `test_rt_lens_capture_sync` | 1 | Ray-traced lens frame/transform coherence in async mode. | 31 s | OK | OK |
| `test_v2x` | 4 | Custom V2X send/receive, byte-length handling, CAM generation, infrastructure sensor. | 78 s | OK | OK |
| `test_ros2` | 5 | ROS 2 enable/disable, camera and lidar publishing, radar/DVS/semantic lidar, cycling, 4-sensor stress. | 106 s | OK | OK |
| `test_vehicle_physics` | 10 → 6 | `apply_physics_control`, wheel collision, sticky control. | 261 s | **FAILED** | 3 server fixes in, 4 PhysX-era tests removed |
| `test_vehicle_telemetry` | 1 | Shape and values of `get_telemetry_data()`. | 36 s | OK | OK |
| `test_vehicle_wheel_steer_angle` | 1 | `get_wheel_steer_angle` tracks steering. | <1 s | OK | OK |
| `test_vehicle_wheel_spin` | 3 | Wheels roll with physics off, keep spin when Chaos resumes, roll under Chaos. | 1 s | OK | OK |
| `test_vehicle_wheel_animation_override` | 5 | Wheel animation override with and without Chaos, plus error cases. | 1 s | OK | OK |
| `test_vehicle_wheel_replay` | 1 | Replayed wheels are interpolated. | <1 s | OK | OK |
| `test_traffic_manager_sync_step` | 1 | No deadlock on a synchronous TM step. | 24 s | OK | OK |
| `test_traffic_manager_control` | 4 | Comfortable pull-away and cornering; automatic vehicle lights. | 118 s | OK | OK |
| `test_traffic_manager_large_vehicle` | 1 | Large-vehicle autopilot produces no NaNs. | 22 s | OK | OK |

## What was cut

| Change | Why |
| --- | --- |
| `test_collision_determinism`: dropped `TwoCarsHighSpeedCollision` and `ThreeCarsSlowSpeedCollision` | Car-vs-car overlaps `test_determinism`; `CarBike` and `CarWalker` keep the contact paths it does not cover. ~4 min. |
| `test_geoconversion`, `test_road_lane_api`: `tearDown` no longer reloads the map | Neither spawns anything or changes the map, but each paid the base class' reload plus a 5 s sleep on all 16 and 14 tests. ~9 min. |
| `test_streamming`: `join(timeout=30)` and an explicit assertion | It deadlocks; an unbounded join took 15.5 min of the suite and reported nothing. Now fails in ~30 s naming the stuck clients. |
| `test_spawnpoints`: one town, snapshot assertions removed | The per-actor transform checks are `test_snapshot` verbatim; walking every map only multiplied map loads. 73 lines to 41. |
| `test_debug_clear`: three tests to one | Three near-identical tests, each paying a map load. |
| `test_map`: dropped `test_reload_world` | `SmokeTest.tearDown` reloads the world after every test in the suite. |
| `test_encoding_cameras` added to `smoke_test_list.txt` | Existed but was never run; costs 1 s. |
| `SmokeTest` waits for the map instead of sleeping 5 s | 35 call sites. `test_geoconversion` 304 s → 1 s, `test_debug_clear` 114 s → 41 s, `test_spawnpoints` 95 s → 38 s. |
| `test_vehicle_physics`: dropped the four friction and stiffness tests | `test_vehicle_tire_long_stiff` lost the `long_stiff` parameter that was its only variable (no Chaos wheel equivalent); `test_vehicle_friction_volume`, `test_vehicle_zero_friction` and `test_vehicle_friction_values` all assume a dragless, frictionless vehicle coasts, which Unreal's body `LinearDamping = 0.01` contradicts. To be rewritten against Chaos. |

`test_client` was cut and then restored: a one-assert client/server version check is the test that stops a stale wheel from being diagnosed as a dozen unrelated failures.


## Fixed

### Test bugs

`test_vehicle_physics:31` used `carla.libcarla.Vector3D`; the module no longer exposes `libcarla`, so one line errored 8 of 10 tests. Now `carla.Vector3D`.

`test_sensor_determinism:329` guarded on `m.endswith("/Town03_Opt")`, but `get_available_maps()` returns bare names, so the test skipped itself and reported OK. Now compares the basename.

`test_map` and `test_spawnpoints` had the same stale-path bug in their exclusion lists, so both had been loading Town11 and Town12 on every run. Both now compare basenames, and also skip `EmptyMap`, which has no roads by design, and `Town13`, whose 12478 spawn points `test_spawnpoints` would multiply by 22 vehicles.

`test_props_loading` spawned every `static.prop.*` including `static.prop.mesh`, which `AStaticMeshFactory` correctly refuses without a `mesh_path` (`StaticMeshFactory.cpp:70-82`). 82 of 83 props spawn; that one is excluded.

### Server and binding bugs

`get_available_maps()` listed 10 entries that are not loadable maps: the seven `Town15_*` sublevels, `DigitalTwinMap` and `RiverPreset01`. Loading one gives `RuntimeError: failed to generate map`. `UCarlaStatics_IsHiddenMap` filtered `Sublevels`, `_Tile_`, `BaseMap` and `BaseLargeMap`, but Town15's sublevels sit beside `Town15.umap` with ordinary names. One rule added at `CarlaStatics.cpp:156-161`: a real map is either directly in a `Maps` folder or in a folder of its own name. 31 entries down to 19.

The `blueprint.*` factory never resolved its class. `MakeBlueprintDefinition` ignored `Parameters.Path`, so `Definition.Class` stayed null and every entry failed to spawn, including the blueprints a content pack ships through `carla-pack add --blueprints`. Fixed at `ActorBlueprintFunctionLibrary.cpp:2050-2057`. Its one example entry was dropped from `BlueprintParameters.json`, and `bp_doc_gen.py`'s `DENYLIST` with it.

Wheel friction could not be set on any vehicle. `ApplyVehiclePhysicsControl` wrote it through `SetWheelFrictionMultiplier` (`CarlaWheeledVehicle.cpp:1007`), which reaches only the physics-thread sim wheel, and then called `RecreatePhysicsState()` at `:1046`. That runs `CreateWheels()`, which does `Wheels.Reset()` and re-instantiates every wheel from its `WheelClass` defaults (`ChaosWheeledVehicleMovementComponent.cpp:1420-1432`), discarding both the applied value and CARLA's own BeginPlay normalisation to 3.5. The readback was therefore the wheel blueprint's default -- 2.0 for the Kia Niro, 3.5 for the Lincoln MKZ, which is why only 7 of 25 vehicles appeared to work. Friction is now re-applied after the recreate, to the new wheel object and to the simulation. Verified: 25 of 25 vehicles honour the value, and 7.25, 1.0, 3.5 and 0.0 all round-trip.

`VehiclePhysicsControl.forward_gear_ratios` and `reverse_gear_ratios` could not be read: `std::vector<float>` has no registered converter, unlike the vectors of bound types at `Control.cpp:384-428`. The getters now build the list element by element. That exposed a second bug the first had hidden — the setters used `extract<float &>`, so the properties could not be written either. Both now `extract<float>`.

## Open — to be discussed

| Issue | Evidence |
| --- | --- |
| Multi-client sensor streaming deadlocks | `test_streamming`'s worker threads never return from `create_client`, which subscribes a second client to another client's GNSS sensor; the main thread hangs in `join()`. A `join(timeout=30)` per thread did **not** contain it — the module still ran 430 s and was killed, so something after the joins blocks too, most likely `gnss_sensor.destroy()` or the `tearDown` reload. |
