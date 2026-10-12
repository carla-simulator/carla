---
name: run-autoware-ue58
description: Runs the Autoware autonomous-driving stack against CARLA on UE 5.8 over CARLA's native ROS 2 interface — no bridge process. Reports the DDS contract (rt/vehicle/status/* out, rt/control/command/* in), the Autoware-only sensor blueprints, generates the lanelet2 + point-cloud map artifacts Autoware needs instead of an .xodr, and drives the two shipped modes (classical NDT stack, or the camera-only VAD end-to-end model). Use when the user asks to "run Autoware on CARLA", "connect an AV stack", "drive the ego from Autoware", "generate a lanelet2 map", or asks which topics CARLA exchanges with Autoware.
license: MIT
compatibility: Linux, a ue58-dev tree BUILT with -DENABLE_ROS2=ON, and a server STARTED with the single-dash -ros2 flag. Autoware itself is external (a source workspace at AUTOWARE_WS, or the autowarefoundation docker image) and needs ROS 2 Humble at /opt/ros/humble. PARTLY VERIFIED on ue58-dev HEAD 718efd7cc, engine 5.8.0, CARLA 0.10.0 - probe, topics, sensors and the launcher dry run were executed here; the Autoware stack itself was NOT run (no source workspace on this machine), so the driving loop is documented from the shipped scripts and source, not measured.
metadata:
  group: ue58
  prerequisites: scripts/check_env.sh
  reference: references/autoware.md
---

# Run Autoware against CARLA on UE 5.8

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

**There is no bridge.** The CARLA server publishes vehicle status and sensor data
straight onto DDS and subscribes to Autoware's control commands, because the
publishers and the subscriber are compiled into the simulator:

```
CARLA server  --rt/vehicle/status/*, sensor topics-->  Autoware
              <--rt/control/command/*, rt/vehicle/engage--
```

That is the whole integration, and it only exists if two separate things are
true — the build has `-DENABLE_ROS2=ON` (compile-time, cannot be enabled later)
and the server was started with the **single-dash** `-ros2` flag
([[run-carla-ue58-server]]).

The pieces are ue58-only. `AutowareGNSSPublisher`,
`AutowareVehicleStatusPublisher`, `AutowareControlSubscriber` and
`AutowareSteeringCompensation.h` do not exist in the UE4 tree, and neither do the
`sensor.other.autoware_gnss` / `sensor.other.vehicle_status` blueprints.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: probe — is this build and this machine equipped at all
- [ ] Step 3: maps — generate lanelet2 + pointcloud (once per town)
- [ ] Step 4: start the server WITH -ros2, then autoware_demo.py
- [ ] Step 5: launch the stack (dry run first)
```

### Step 2: What you have

```bash
source scripts/env.sh

bash scripts/autoware.sh probe      # build flag, native pieces, stack, maps
bash scripts/autoware.sh topics     # the DDS contract, read from the source
bash scripts/autoware.sh sensors    # Autoware blueprints on a running server
```

`topics` greps the publisher and subscriber constructors rather than restating
them, so it cannot drift from your build. Measured contract:

| CARLA → Autoware | Autoware → CARLA |
|---|---|
| `rt/vehicle/status` (base) | `rt/control/command` (base) |
| `rt/vehicle/status/velocity_status` | `rt/control/command/control_cmd` |
| `rt/vehicle/status/steering_status` | `rt/control/command/gear_cmd` |
| `rt/vehicle/status/control_mode` | `rt/control/command/turn_indicators_cmd` |
| `rt/vehicle/status/gear_status` | `rt/control/command/hazard_lights_cmd` |
| `rt/vehicle/status/turn_indicators_status` | `rt/control/command/emergency_cmd` |
| `rt/vehicle/status/hazard_lights_status` | `rt/vehicle/engage` |

Status is `RELIABLE / VOLATILE / KEEP_LAST 1`; commands are `RELIABLE /
TRANSIENT_LOCAL / KEEP_LAST 1`, so a late-joining simulator still receives the
last command. A ROS 2 node sees these without the `rt/` prefix.

`sensors` confirms the rig's blueprints exist and prints each one's ROS
attributes. Both `ros_name` (a topic *segment*) and `ros_topic_name` (the exact
topic, overriding generation) are on every sensor — the second is how the rig
lands on Autoware's expected names rather than CARLA's.

### Step 3: Maps — Autoware does not read your `.xodr`

```bash
bash scripts/autoware.sh maps --town Town10HD_Opt
```

Autoware needs `lanelet2_map.osm`, `pointcloud_map.pcd` and
`map_projector_info.yaml`, generated from the CARLA map by
`av_stacks/autoware/map_tools/`. They land in `map_tools/maps/<Town>/`, and
`probe` reports which towns are complete. A prebuilt set can also be fetched
with `map_tools/fetch_prebuilt_maps.sh`.

### Step 4: Server, then the CARLA-side driver

```bash
cd ../run-carla-ue58-server && ROS2=1 DETACH=1 bash scripts/run_server.sh game
cd ../run-autoware-ue58     && bash scripts/autoware.sh demo --spawn_index 52
```

**`autoware_demo.py` owns the simulation.** It applies world settings, spawns the
ego and the sensor rig, and ticks the world — the stack does not. Run it before
launching Autoware and leave it running. Useful flags: `--list_maps`,
`--load_map`, `--spawn_index N`, `--follow`, `--hz_rate`, `--run_async`,
`--substepping`, `--resync`, `--mgrs_off`, `--time_scale`.

It also writes its own post-process profile
(`deploy_postprocess_profile("autoware_demo", …)` → `Content/Carla/Config/
PostProcess/autoware_demo.json`), so that file being absent is normal — it is
regenerated, not shipped.

With `ROS2=1` the server needs noticeably longer to accept connections than a
plain start; wait for the port rather than assuming failure.

### Step 5: The stack

```bash
bash scripts/autoware.sh stack --mode e2e            # DRY RUN — prints the plan
bash scripts/autoware.sh stack --mode e2e --go       # actually launch
```

**Dry run is the default** because the real thing starts long-lived processes and
possibly a container. The dry run prints every command, the log paths, the pid
and container files, and the readiness gates it will wait on — read it before
committing.

| Mode | What runs |
|---|---|
| `--mode classical` | full Autoware: NDT localisation against the point-cloud map, lidar/camera perception, behaviour/motion planning, trajectory follower |
| `--mode e2e` | camera-only **VAD** end-to-end model (`autoware_tensorrt_vad`), six surround cameras in, trajectory out; localisation from simulator ground truth |

Other launcher options worth knowing: `--town`, `--stack auto|source|docker`,
`--image`, `--container-name`, `--rmw fastdds|cyclonedds|zenoh`, `--map-path`,
`--goal`, `--spawn-index`, `--with-rviz`, `--with-display`, `--domain-id`
(default 42).

**Keep the RMWs straight.** The launcher defaults the *simulator* side to
`fastdds` and explicitly warns against `cyclonedds` there (a known
fragmented-receive bug); the *Autoware* side always runs cyclonedds, with a
generated config. Both sides must share `ROS_DOMAIN_ID`.

## Examples

**Example 1: "does this build even support Autoware?"**

`probe`. It answers in one screen: `ENABLE_ROS2`, the four native source files,
the five shipped scripts, whether an Autoware workspace or docker image exists,
and which map sets are generated.

**Example 2: "what topics does CARLA exchange with Autoware?"**

`topics`. No server needed — it reads your tree.

**Example 3: "run the end-to-end model on Town10"**

`maps --town Town10HD_Opt`, start the server with `ROS2=1`, `demo`, then
`stack --mode e2e` (read the dry run) and `--go`. First VAD run builds TensorRT
engines and takes tens of minutes; cached engines start in about a minute.

**Example 4: "Autoware is up but the car does not move"**

Check the direction that is failing. `rt/vehicle/status/*` present but
`rt/control/command/control_cmd` absent means the stack is not commanding;
commands present and the ego still still means the subscriber is not reaching the
vehicle — confirm the server actually got `-ros2`, and that both sides share
`ROS_DOMAIN_ID`. `stack` also waits on
`/api/operation_mode/change_to_autonomous`, which must return `success=True`.

## Troubleshooting

**Problem: no topics at all**
Cause: ROS 2 not compiled in, or the server started without `-ros2`.
Solution: `probe` reports the cache value; rebuild with `-DENABLE_ROS2=ON`
([[build-carla-ue58]]) and start with `ROS2=1` ([[run-carla-ue58-server]]). The
flag is single-dash on ue58 — `--ros2` is silently ignored.

**Problem: `ros2` command not found in the dry-run commands**
Cause: no ROS 2 on the host; the launcher sources `/opt/ros/humble/setup.bash`.
Solution: install ROS 2 Humble, or use `--stack docker`.

**Problem: the stack cannot localise / rejects the map**
Cause: missing or partial map artifacts — `.xodr` is not enough.
Solution: `maps --town <Town>`; `probe` flags incomplete sets.

**Problem: conda or a virtualenv breaks the launcher**
Cause: mixed Python environments; the launcher strips `CONDA_*`, `VIRTUAL_ENV`,
`PYTHONPATH` and conda entries from `PATH` for exactly this reason.
Solution: run it from a clean shell rather than fighting it.

**Problem: sensors publish on CARLA-shaped topic names**
Cause: `ros_name` only contributes a segment.
Solution: set `ros_topic_name` for the exact Autoware topic; the reference rig
(`run/spawn_vad_rig.py`) does this per camera.

**Problem: the server dies or the port never opens with ROS2=1**
Cause: a ROS 2 start is slower, and this skill's own measurement hit a 120 s
wait that expired before the port opened.
Solution: poll the port for longer; the blueprint checks (`sensors`) work against
a plain server too, since the Autoware blueprints exist with or without `-ros2`.

## Outputs

`probe`, `topics` and `sensors` are read-only. `maps` writes map artifacts under
`av_stacks/autoware/map_tools/maps/<Town>/`. `demo` runs a client that spawns an
ego and sensors on the running server. `stack` prints a plan by default and only
starts processes with `--go`, recording pids and container names under
`av_stacks/autoware/run/logs/` — tear down with that directory's `stop_all.sh`.

The topic contract, the QoS reasoning, what the reference rig spawns, and which
parts of this are measured versus read from source are in
[references/autoware.md](references/autoware.md).
