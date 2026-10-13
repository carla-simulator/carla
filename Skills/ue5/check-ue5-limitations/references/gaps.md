# UE 5.5 vs UE 5.8 — the evidence

Measured on 2026-08-28 by diffing two real checkouts:

- **5.5** — `carla-simulator/carla` branch `ue5-dev` @ `0a5ce0d` (2026-07-14),
  shallow single-branch clone, source only.
- **5.8** — branch `ue58-dev` @ `718efd7cc` (2026-08-25), the tree every ue58
  skill was verified against.

Every claim below is a **source or layout** fact. No 5.5 engine, content clone or
build exists on this machine, so nothing here is a measured runtime behaviour —
which is the one thing to keep in mind when quoting it.

## Same product, different revision

| | 5.5 | 5.8 |
|---|---|---|
| `CHANGELOG.md` top entry | `## CARLA 0.10.0` | `## CARLA 0.10.0` |
| engine branch the tree names | `ue5-dev-carla` | `ue58-dev-carla` |
| `PythonAPI/carla/src/Sensor.cpp` | \- | **0** changed lines |
| `Client.cpp`, `Map.cpp`, `Blueprint.cpp` | \- | **0** changed lines each |
| `Actor.cpp` | \- | 2 changed lines |
| `World.cpp` | \- | 22 changed lines |

That near-identical Python API is the reason this skill exists instead of six
mirrored ue5 skills: there is almost nothing to mirror, and duplicating the
python-api group by engine would double the maintenance for a 24-line delta.

## Gap 1 — ROS 2 middleware

- 5.8 has `LibCarla/source/carla/ros2/middleware/` with the RMW abstraction,
  `QosProfile.h` and `ActiveMiddleware.{h,cpp}`. 5.5 has **no `middleware`
  directory** at all.
- CMake options present only on 5.8: `CARLA_CYCLONEDDS_VERSION`,
  `CARLA_CYCLONEDDS_TAG`, `CARLA_ZENOH_C_VERSION`, `CARLA_ZENOH_C_TAG`.
- 5.5 **does** have the publisher refactor: `BasePublisher`, `BasicPublisher`,
  `PublisherImpl`, the `listeners/` directory, `FastDDSAliases.h`, and the shared
  helpers `CameraIntrinsics`, `ImuMath`, `DvsEventEncoding`,
  `OpticalFlowEncoding`, `PointCloudFieldsLayout`, `RadarPolarToCartesian`,
  `TransformQuaternion`.
- Neither branch has `CarlaMapPublisher`, so `rt/carla/map` is absent on both —
  that gap is not 5.5-specific.

**Consequence:** the transport is FastDDS, full stop. `[[add-ros-publisher]]`'s
QoS step differs (no `QosProfile.h`), everything else in that skill applies.

## Gap 2 — Autoware

Absent on 5.5: `publishers/AutowareGNSSPublisher.*`,
`publishers/AutowareVehicleStatusPublisher.*`,
`subscribers/AutowareControlSubscriber.*`, `ros2/AutowareSteeringCompensation.h`,
and the two blueprints `sensor.other.autoware_gnss` /
`sensor.other.vehicle_status` (0 references in
`ActorBlueprintFunctionLibrary.cpp` against 1 each on 5.8).

Not checked, because the integration cannot work without the above:
`PythonAPI/examples/av_stacks/autoware/` and `autoware_demo.py`.

**Consequence:** [[run-autoware-ue58]] does not apply. There is no partial path.

## Gap 3 — DLSS and the ray-traced lens

- `enable_dlss` / `dlss_screen_percentage`: 0 references on 5.5, 3 on 5.8.
- `Sensor/SceneCaptureCamera_RayTracedLens.cpp`: only on 5.8, so
  `sensor.camera.rt_lens` has no implementation on 5.5.
- `CARLA_DLSS_SDK_PATH` option and `CMake/DLSS.cmake` handling: 5.8 only.
- `post_process_profile` exists on both (8 references on 5.5, 14 on 5.8), so the
  profile mechanism itself is shared.

## Gap 4 — OFPA large-map mount

`MountExternalPackageRoots` in
`Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Carla.cpp`: **2** references on
5.8, **0** on 5.5.

This is the patch that mounts `Content/Carla/__ExternalActors__` and
`__ExternalObjects__` as package roots. Without it, one-file-per-actor large maps
load with an empty World Partition — a black screen with a valid map name. 5.8
absorbed the patch upstream (which retired a manual step from the build notes);
on 5.5 it is still needed.

**Consequence:** on 5.5, Town12/Town13/Town15 are unusable until patched. Note
that Town15 is *also* uncookable on both branches, for an unrelated content
reason, so it fails twice over.

## Gap 5 — Python API additions

From the 22-line `World.cpp` diff and the 2-line `Actor.cpp` diff, 5.8 adds:

```
World.get_ego_spawn_points()
World.set_publish_tf() / World.get_publish_tf()
World.spawn_custom_mesh()          (with grass / material / vertices / triangles args)
Actor.enable_constant_acceleration() / disable_constant_acceleration()
```

Everything else the python-api skills use is present on both. In particular these
are **not** 5.5 gaps — they are 0.10.0-wide changes relative to UE4 0.9.x, and
apply to 5.5 exactly as documented in those skills:

- `enable_for_ros` / `disable_for_ros` / `is_enabled_for_ros` on
  `carla.ServerSideSensor` (identical `Sensor.cpp` on both);
- the gbuffer bindings, which crash the server on 0.10.0;
- no `carla.GearPhysicsControl`; `forward_gear_ratios` present on both;
- no `draw_hud_*` on either;
- `CityObjectLabel.Rock` on both (`ObjectLabel.h`).

## Shared defects — the ue58 notes apply verbatim

| Defect | 5.5 | 5.8 |
|---|---|---|
| `RecastBuilder` in `Util/DockerUtils/dist` | absent | absent |
| `Environment.sh` sets `CARLA_BUILD_TOOLS_FOLDER` | no | no (upstream); patched locally here |
| `Import.sh` accepts `--help` | no | no (upstream); patched locally here |
| `#if 0 // @CARLAUE5` wheel block in `USDImporterWidget.cpp` | present | present |
| `MapsToCook` default includes uncookable Town15 | yes | yes |
| `Makefile` / `Util/BuildTools` | absent | absent |
| the six `add_carla_ue_package_target` targets | identical | identical |

The `Import.sh` / `Environment.sh` comparison is worth spelling out: the only
differences between the two trees' copies are the two fixes applied locally to
the 5.8 checkout. Upstream, both branches carry the identical defect.

## How to extend this

1. A gap belongs here only with its evidence — a path that exists on one side and
   not the other, or a reference count.
2. `gaps.sh check` tests one marker per gap. Adding a gap means adding its marker
   there, so the claim stays falsifiable against the tree in front of you.
3. If a 5.5 build ever exists, the runtime claims can be promoted from "source
   fact" to "measured" — start with the four that matter most: does `-ros2` work
   with FastDDS only, does a large map really come up black, does `Import.sh`
   fail identically, and does a packaged 5.5 server load the same maps.
