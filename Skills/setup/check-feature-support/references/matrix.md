# Evidence behind each verdict

Stamped to `ue58-dev` HEAD `718efd7cc` (engine 5.8.0, reporting CARLA 0.10.0 —
the pre-release label for the line now shipping as **CARLA 1.0**) and the 0.9.16
UE4 tree at `~/carla`. Read `0.10.0` throughout as naming the UE5 line, not one
release; where a fact is specific to a build, the row says which. **Evidence** is either *measured* (executed against
a live server or the build) or *read* (source or file layout). Each row records
how to re-check it after an upgrade, and what promoting it would take.

## `[broken]` — verified broken or removed

### GBuffer capture — crashes the server
- **Measured.** One `listen_to_gbuffer(GBufferTextureID.SceneColor, cb)` call
  raised `RuntimeError: std::exception` client-side and killed the simulator:
  `Assertion failed: Stream.has_value() [Carla/Sensor/DataStream.h] [Line: 55]`,
  `SIGSEGV`, through `FCameraGBufferUint8::GetToken() const`. A plain RGB listener
  on the same sensor delivered frames in the same session.
- The API is fully present — `listen_to_gbuffer` / `stop_gbuffer` /
  `is_listening_gbuffer` on `carla.ServerSideSensor`, all seven
  `GBufferTextureID` members — which is why it must be called out rather than
  left to discovery.
- **Re-check:** spawn a camera, call `listen_to_gbuffer`, watch the server.
- **To promote:** the gbuffer streams have to be assigned server-side; this is an
  upstream fix, not a usage question.

### `Landmark.waypoint` — always `None`
- **Measured.** Town10HD_Opt: 68 landmarks, 68 with `waypoint is None`. All other
  fields populate.
- **Alternative:** `map.get_waypoint_xodr(road_id, lane_id, s)`.

### Gear ratios unreadable
- **Measured.** `forward_gear_ratios` / `reverse_gear_ratios` raise
  `TypeError: No to_python (by-value) converter found for C++ type:
  std::__1::vector<float, ...>`. The other 27 fields read; a `mass` write
  round-tripped 1696 → 1750 kg.

### Standalone asset packages — gone both ways
- **Read.** UE4 has `Util/BuildTools/Package.sh --packages=Name1,Name2` and
  `Util/ImportAssets.sh`. ue58 has no `Package.sh`; the only targets are
  `add_carla_ue_package_target` × 6 (`Unreal/CMakeLists.txt:546-551`), all
  whole-server, scoped by `CARLA_MAPS_TO_COOK`. A built package root contains
  `CHANGELOG LICENSE Linux PythonAPI README Tools VERSION` — no installer.

### Pedestrian navmesh on map import — silent skip
- **Read.** `Import.py:487 build_binary_for_navigation` calls
  `Util/DockerUtils/dist/build.sh`, guarded by `if [ -f "FBX2OBJ" ]` and
  `if [ -f "RecastBuilder" ]`. UE4 ships `RecastBuilder` in `dist/`; ue58 ships
  only the four scripts. CMake builds RecastBuilder to
  `Build/<preset>/_deps/recastnavigation-build/RecastBuilder/` and stages it into
  packages as `Tools/RecastBuilder`, but nothing populates `dist/` and no target
  references `DockerUtils`. `FBX2OBJ` has its own
  `Util/DockerUtils/fbx/CMakeLists.txt` that the main build never adds.
- **To promote:** copy both binaries into `dist/` and re-run an import — the
  workaround is in [[import-carla-ue58-map]].

### Semantic tagging of imported assets
- **Read.** `GenerateTaggedMaterialsRegistry`: 2 UE4 plugin files, **0** in
  ue58. UE4's `Package.sh` ran it per package.

### Co-simulation tooling
- **Read.** ue58-dev has **no `Co-Simulation/` directory**. UE4 ships
  `Co-Simulation/{Sumo,PTV-Vissim,Chrono,Carsim}`.
- **Measured.** `Vehicle.enable_chrono_physics()`, `enable_carsim()` and
  `use_carsim_road()` are still on the 0.10.0 Python API — API without tooling.

### `rt/carla/map`
- **Read.** `CarlaMapPublisher` is in 0.9.x's
  `LibCarla/source/carla/ros2/publishers/` and absent on 0.10.0.
- **Alternative:** `map.to_opendrive()` over RPC.

## `[untested]` — present, never exercised here

| Feature | What exists | What is missing to promote it |
|---|---|---|
| RSS | `sensor.other.rss` in the library (**measured**); `ENABLE_RSS:BOOL=OFF` in this cache (**measured**) | a build with `-DENABLE_RSS=ON`, then a run that produces an RSS response |
| V2X | `sensor.other.v2x`, `sensor.other.v2x_custom` (**measured**); `ServerSideSensor.send()` (**measured**) | two actors exchanging a message, and the message format documented |
| multi-GPU | `LibCarla/source/carla/multigpu/` (**read**); server accepts `-carla-primary-host`, `-carla-primary-port`, `-carla-secondary-port` (**read**) | two GPUs, or two machines; a primary/secondary pair actually rendering |
| digital twins / OSM | `OpenDriveToMap.h`, `DigitalTwinsBaseWidget.h`, `MapGeneratorWidget.h`, `ProceduralBuildingUtilities.h`, `ProceduralWaterManager.h` (**read**) | these are editor **widgets**, not a CLI — a driving path has to be found first. The OSM to OpenDRIVE converter itself is always compiled in. | |

Vehicle import on ue58 used to be listed here. It now has a skill:
[[carla-vehicle-authoring]] covers the Blender and Unreal pipeline, physics
included, and its runtime validation.

Note the asymmetry that makes RSS worth a special mention: the blueprint is
present whether or not the feature is compiled in, so "it's in the blueprint
library" is not evidence of support. Same shape as gbuffers.

## `[works]` — verified, no skill

- **Texture streaming.** `World.apply_color_texture_to_object(s)`,
  `apply_float_color_texture_to_object(s)`, `apply_textures_to_object(s)` all
  present on 0.10.0, with `carla.TextureColor`, `carla.FloatColor`,
  `carla.MaterialParameter` (**measured**). No procedure has been vetted — in
  particular nobody here has checked how it interacts with a packaged server.
- **CarSim / Chrono hooks.** See the co-simulation row: the calls exist, the
  bridges do not.

- **Map layers: Town01_Opt to Town07_Opt only.** Measured on the 1.0 weekly
  `6c726a4a3` with the AdditionalMaps pack: `unload_map_layer` removes the layer
  from a camera image and from `get_environment_objects()` (Town03_Opt
  `Buildings`: 9254 -> 8513 objects, pixel diff 31.5 against 3.5 noise) and
  `load_map_layer` restores it, on all seven towns. **Town10HD_Opt has no layers**:
  its sublevels were merged into the persistent level on purpose (Content
  `68520135b61`, 2024-07-02), so `ConvertMapLayerMaskToMapNames` matches no
  streaming level and every call is a silent no-op (60433 objects before and after
  for every layer). The `T10HD_*` files under `Maps/Sublevels/Town10HD_Opt/` are
  leftovers. Sublevel-based layers are being phased out on UE 5; a grouping without
  sublevels is planned (issue #10228). On Town10HD_Opt use
  `enable_environment_objects` ([[toggle-env-objects]]).
- **Re-check:** `unload_map_layer(MapLayer.Buildings)` on Town03_Opt and on
  Town10HD_Opt, compare `get_environment_objects()` counts.

## Things worth knowing that are neither broken nor new features

Measured on 0.10.0, useful when judging whether a procedure will transfer:

- `enable_for_ros` / `disable_for_ros` / `is_enabled_for_ros` moved from
  `carla.Actor` (0.9.x) to `carla.ServerSideSensor`. Introspecting `carla.Sensor`
  finds neither — that base class has only `listen`/`stop`/`is_listening`.
- `carla.GearPhysicsControl` no longer exists; `VehiclePhysicsControl` and
  `WheelPhysicsControl` were replaced field-for-field by the Chaos rewrite
  ([[control-vehicle]] has the mapping).
- The `sensor.camera.rgb` blueprint exposes 18 attributes, without the
  photographic controls 0.9.x has; post-processing comes from
  `post_process_profile` naming a JSON under `Content/Carla/Config/PostProcess/`.
- New and unmentioned elsewhere: `World.get_ego_spawn_points()`,
  `World.spawn_custom_mesh()`, `World.set_publish_tf()`,
  `World.set_imu_sensor_gravity()`, `CityObjectLabel.Rock`.

## How to keep this honest

1. A row moves out of `[untested]` only when someone **ran** it — not when the
   code was read more carefully.
2. A row moves into `[broken]` with a measurement pasted in, as above.
3. When a row becomes a skill, replace it with the skill name; do not leave both.
4. If CARLA upgrades, re-stamp the header and re-run the cheap measurements: the
   blueprint list, the physics-control field read, one `unload_map_layer` check,
   and `probe`.
