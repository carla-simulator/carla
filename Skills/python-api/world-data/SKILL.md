---
name: world-data
description: Queries the live CARLA world and resolves an entity description into a concrete actor id — find actors by type, role, or colour, or the single one nearest a point/actor; read the world snapshot; get static level bounding boxes by label; cast rays for semantic points; project a point to the ground; or (ros-topics) list the native ROS 2 topics each actor should be publishing and why any is silent. Identifies by stable attributes, not by any rank/order. Use when the user asks "what actors are here", "find the red prius", "the vehicle nearest the ego", "list the pedestrians", or needs a specific actor id for another skill.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Does NOT need UE4_ROOT. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/world-data.md
---

# Query the world (and resolve entities)

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

The **resolver** skill. Other skills act on one entity by id; this one finds that
id when the request is a description — "a prius" (but there are three), "the
nearest walker", "the 3rd vehicle". It also reads world-level data: snapshot,
level bounding boxes, raycasts, ground projection.

## Identifying a specific actor (the core use)

There is **no ordering / rank** — peer actors have no meaningful order, so "the
3rd one" is not a thing. Identify by a **stable attribute** or a **spatial
predicate**, narrowing until one remains, then use its **id**:

- by type/blueprint: `--filter '*prius*'`, `--filter walker.pedestrian.*`
- by role: `--role hero`
- by colour (tells identical models apart): `--color 255,0,0`
- the single **closest** to a point/actor: `--near X,Y,Z` / `--near-id N` **with
  `--nearest`** (a predicate — "the closest" — not an index)
- `--full` dumps every attribute (colour, role, size, yaw), so you can spot the
  distinguishing field and filter by it (or just read the id)

So "the red prius" → `actors --filter '*prius*' --color 255,0,0`; "the prius
nearest the ego" → `actors --filter '*prius*' --near-id <ego> --nearest`. Either
yields an **id** to hand to telemetry / bounding-boxes / control-vehicle /
control-spectator. For junctions ("3-way intersections") use the map-waypoints
skill's `junctions --arms 3`.

**Moving objects:** a distance is a single-frame snapshot and changes as actors
move, so `--nearest` is only "closest right now". The **id** is the stable handle
— resolve once, then act on the id; re-query for a fresh distance. (An inherent
order only exists in special cases like a queue of vehicles along a lane — that is
a lane query, not this generic actor list.)

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: actors --filter/--role/--color [--nearest] until one remains → its id
- [ ] Step 3: hand the id to the action skill; or use snapshot/level-bbox/raycast/ground
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh`. **Negative
coordinates:** use the `=` form (`--near=-24,-57,0.6`).

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 2-3: Query

```bash
source scripts/env.sh

python3 scripts/world_data.py actors                              # everything (non-traffic)
python3 scripts/world_data.py actors --filter '*prius*'           # all priuses (ids + locations)
python3 scripts/world_data.py actors --filter '*prius*' --full    # + color/role/size to tell them apart
python3 scripts/world_data.py actors --filter '*prius*' --color 255,0,0          # the red one
python3 scripts/world_data.py actors --filter '*prius*' --near-id 100 --nearest  # closest to actor 100
python3 scripts/world_data.py snapshot                            # frame / time / count
python3 scripts/world_data.py level-bbox --label TrafficSigns     # static boxes by label
python3 scripts/world_data.py raycast --from=0,0,1 --to=50,0,1    # semantic hits along a ray
python3 scripts/world_data.py ground --at=20,20,50               # drop to the ground
python3 scripts/world_data.py ros-topics                          # ROS 2 topic tree per actor
```

### Resolving ROS 2 topics (`ros-topics`)

The same resolver job, one level up: it maps live actors to the topics the server
**should** be publishing, and names the reason for each silent one. It needs no
ROS 2 installation — it derives the names the way the server does, so it answers
the half `ros2 topic list` cannot ("the topic is missing — why?").

```
world topics (exist whenever the server runs with --ros2):
  rt/clock       [rosgraph_msgs/Clock]  every tick
  rt/carla/map   [std_msgs/String]      OpenDRIVE, LATCHED, re-sent on map load
  rt/tf          [tf2_msgs/TFMessage]   per registered actor, unless ros_publish_tf=false

hero vehicle(s): 1
  id=112 vehicle.lincoln.mkz_2017 ros_name=hero
    <- rt/carla/hero/vehicle_control_cmd     [carla_msgs/CarlaEgoVehicleControl]
    <- rt/carla/hero/ackermann_control_cmd   [ackermann_msgs/AckermannDriveStamped]

sensors: 2
  id=113 sensor.camera.rgb ros_name=front  [NOT enabled_for_ros -> SILENT]
    -> rt/carla/hero/front/image        [sensor_msgs/Image]
    -> rt/carla/hero/front/camera_info  [sensor_msgs/CameraInfo]
  id=114 sensor.other.obstacle ros_name=actor114
    (no native publisher for sensor.other.obstacle)
```

Three reasons an expected topic is absent, all visible above: the sensor is not
`enable_for_ros()`-ed ([[create-sensor]] `ros --id N`), the sensor type has no
publisher at all, or the vehicle is not `role_name = hero` (only the hero is
registered). If even `rt/clock` is missing, the problem is upstream — the build
or the launch flag ([[run-carla-server]] `ROS2=1`).

## On CARLA 0.10.0 (the UE5 line: 5.5 and 5.8)

`ros-topics` is the only mode that changes. **`rt/carla/map` does not exist on
0.10.0**: `CarlaMapPublisher` is present in 0.9.x and absent from
`LibCarla/source/carla/ros2/publishers/` in 0.10.0, so nothing ever advertises
the OpenDRIVE topic. The mode detects the server version and says so rather than
listing a topic you would then hunt for; read the map with `map.to_opendrive()`
over RPC instead.

`rt/clock` and `rt/tf` behave the same on both. On 0.10.0, `rt/tf` publishing is
additionally gated globally by `World.set_publish_tf()`, which is new.

The actor-query modes are unchanged, and 0.10.0 adds a few worth knowing:
`world.get_traffic_sign()`, `world.get_names_of_all_objects()` and
`world.set_imu_sensor_gravity()` — plus `world.get_ego_spawn_points()` and
`world.spawn_custom_mesh()`, which are **UE 5.8 only**
([[check-ue5-limitations]]). `CityObjectLabel` gained `Rock`, which matters
for `level-bbox` if you hard-code label integers.

## Examples

**Example 1: 1 vehicle among many**

User says: "get the prius closest to me" (ego = id 100)

`actors --filter '*prius*' --near-id 100 --nearest` → e.g. `id=137`. Then
`telemetry show --id 137`. (Or "the red prius" → `--color 255,0,0`.)

**Example 2: survey the scene**

User says: "what's around?"

`actors` for the movable actors, `snapshot` for the frame/count.

**Example 3: where's the ground here**

User says: "what's the ground height at (20, 20)?"

`ground --at=20,20,50` → projects down to the road surface + its semantic label.

## Troubleshooting

**Problem: too many actors match**
Cause: broad filter.
Solution: narrow by `--role`/`--color`, or take `--nearest --near-id N`; use
`--full` to find a distinguishing attribute, then the id.

**Problem: expected actor not listed**
Cause: it's a traffic light/sign/spectator (hidden by default).
Solution: add `--all`.

**Problem: raycast/ground returns nothing**
Cause: the ray misses geometry, or no ground within `--search`.
Solution: widen the ray / raise `--search`; check the start point is above ground.

## Outputs

Text listings (actors, boxes, points) — no world change. The actor ids it prints
are inputs for the action skills.

Detail (attribute/nearest selection, level bbox labels, cast_ray/project_point,
snapshot fields) in [references/world-data.md](references/world-data.md).
