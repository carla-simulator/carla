---
name: visualize-ros-rviz
description: Inspects and visualises a ROS-2-enabled CARLA server from Docker containers, with no local ROS 2 install — list/echo/rate-check topics (the "are the topics actually on the wire" check), run the bundled map-and-lidar demo stack (hero vehicle with camera/lidar/GNSS/IMU on autopilot), and open RViz2 with the lane-network preset. Use when the user asks to "check the ROS topics", "echo /carla/map", "is CARLA publishing to ROS", "visualise the lidar in RViz", "open rviz", or "run the ROS 2 demo".
license: MIT
compatibility: Linux with Docker (daemon reachable) and a running CARLA server that was BUILT and STARTED with ROS 2. Needs no local ROS 2 installation — the ros2 CLI and RViz run in containers built on first use (network required, minutes). The rviz mode needs an X display; everything else works headless. Building the demo image needs a matching carla wheel (cp310 for humble, cp312 for jazzy) in PythonAPI/carla/dist.
metadata:
  group: ros2
  requires: run-carla-server
  prerequisites: scripts/check_env.sh
  reference: references/ros-consumers.md
---

# See the ROS 2 side of a CARLA server

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Everything else in this collection acts through the RPC API. This skill is the
**consumer** side: it proves what is really on the DDS wire and draws it. It
never launches a simulator, and it needs no ROS 2 on the host — the `ros2` CLI
and RViz run inside images built from the checkout's own
`PythonAPI/examples/ros2/`.

Use it after [[run-carla-server]] `ROS2=1`: `world-data ros-topics` says what
*should* be published, this says what *is*.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: A server is running WITH ROS 2, on a known --rmw and domain
- [ ] Step 3: topics -> prove /clock exists; echo/hz -> prove data flows
- [ ] Step 4: (opt-in) demo -> spawn the stack; rviz -> draw it
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 3: Inspect (read-only)

```bash
source scripts/env.sh

bash scripts/ros_view.sh topics                      # ros2 topic list + verdict
bash scripts/ros_view.sh info /carla/map             # type, endpoints, QoS
bash scripts/ros_view.sh echo /carla/map             # one sample (latched OpenDRIVE)
bash scripts/ros_view.sh hz /carla/hero/lidar/point_cloud 10
bash scripts/ros_view.sh local-env                   # exports for a LOCAL ros2 install
```

`echo` adds `--qos-durability transient_local --full-length` automatically for
`/carla/map`: it is latched and published only on episode start, so a default
(volatile) subscription waits forever and a default echo truncates the OpenDRIVE
at 128 characters.

**Using a local ROS 2 instead of these containers?** Run
`eval "$(bash scripts/ros_view.sh local-env)"` first. It exports the checkout's
RMW profile, which forces UDP-only transport. Without it CARLA's
shared-memory-built Fast DDS never matches your subscriber: the topics list fine
and **no data ever arrives**, with no error on either side.

`topics` is the one to run first: it waits for DDS discovery, lists the topics
and **fails loudly with the ordered list of causes** when `/clock` is absent
(not built with ROS 2 → not started with `--ros2` → domain mismatch → RMW
mismatch → missing Zenoh router).

Pass **ROS** names (`/clock`), not DDS names (`rt/clock`) — the `rt/` prefix is
the DDS-level encoding of the same topic.

### Step 4: Demo stack and RViz (opt-in — has side effects)

```bash
bash scripts/ros_view.sh demo            # SPAWNS a hero + camera/lidar/GNSS/IMU
bash scripts/ros_view.sh demo --map-only # only the lane markers, no actors
bash scripts/ros_view.sh rviz            # RViz2 with the bundled preset
```

`demo` **mutates the world**: it spawns a hero vehicle with four sensors, drives
it on autopilot, publishes `map->hero` TF and converts the latched
`/carla/map` OpenDRIVE into `/carla/map_markers` for RViz. Run it only when the
request asks for the demo or for a live scene to visualise; it cleans up its
actors and restores the world settings on exit (single Ctrl+C). A second
concurrent run fails fast on the fixed container name instead of spawning a
duplicate hero on the same topics.

`rviz` is read-only and needs `$DISPLAY`. Both modes delegate to the checkout's
own `run_map_and_lidar_demo.sh` / `run_rviz.sh`, so their behaviour is never
forked here.

## On CARLA 0.10.0 (the UE5 line: 5.5 and 5.8)

**`/carla/map` is gone on 0.10.0.** `CarlaMapPublisher` exists in 0.9.x's
`LibCarla/source/carla/ros2/publishers/` and is absent in 0.10.0, so the latched
OpenDRIVE topic is never advertised. `info /carla/map` and `echo /carla/map`
will simply wait. Get the map with `map.to_opendrive()` over RPC, or point RViz
at the lidar/camera topics, which are unchanged.

That also means the RViz lane-network preset has no map source on 0.10.0 — the
road network has to come from the client side.

Topics that gained publishers on 0.10.0, **UE 5.8 only**: the Autoware set
(`AutowareGNSSPublisher`, `AutowareVehicleStatusPublisher`, from
`sensor.other.autoware_gnss` and `sensor.other.vehicle_status`). The DVS camera
publisher was renamed `CarlaDVSPublisher` → `CarlaDVSCameraPublisher`; its topics
are unchanged.

`rt/tf` is additionally gated by `World.set_publish_tf()` on 0.10.0, so a missing
TF tree can now mean "switched off globally" rather than "no sensor publishing".

## Knobs

| Env | Default | Meaning |
|---|---|---|
| `ROS_DISTRO_TAG` | `humble` | `humble` or `jazzy`; picks the image and the wheel tag (cp310 / cp312) |
| `RMW` | `fastdds` | `fastdds` · `cyclonedds` · `zenoh` — **must match the server's `--rmw`** |
| `ROS_DOMAIN_ID` | unset (0) | **must match the server's `--ros-domain-id`** |
| `CARLA_HOST`/`CARLA_PORT` | `localhost`/`2000` | the server the demo stack connects to |

Images are named `carla-rviz-<distro>-<rmw>` and
`carla-map-and-lidar-demo-<distro>-<rmw>`, so each combination is built once and
cached. Only the demo image needs the carla wheel; topic inspection and RViz do
not.

## Examples

**Example 1: is CARLA publishing at all?**

User says: "is CARLA publishing to ROS?"

`bash scripts/ros_view.sh topics`. `/clock` present → yes, and the list shows
which actor topics exist. Absent → the command prints the ordered causes.

**Example 2: the camera topic is missing**

User says: "my camera doesn't show up in ROS"

`topics` (server alive?) → [[world-data]] `ros-topics` (does the sensor exist and
is it `enabled_for_ros`?) → [[create-sensor]] `ros --id N` to enable it → `hz` on
the topic to confirm frames.

**Example 3: see the town and the lidar together**

User says: "show me the lidar in RViz"

`demo` in one shell (spawns the stack), `rviz` in another. Fixed frame `map`;
set `Views → Target Frame: hero` to follow the car.

## Verify

- `topics` exits non-zero when `/clock` is missing — treat that exit code as the
  gate, not the printed list.
- `info <topic>` shows the publisher count: `0` means the topic name exists in
  discovery but nothing writes it.
- `hz` gives the real rate; a camera at ~0 Hz with a live server usually means
  `-nullrhi` (no rendering, so no images) rather than a ROS problem.

## Troubleshooting

**Problem: `topics` lists nothing at all (not even `/clock`)**
Cause: in order of likelihood — the server was not built with ROS 2, was started
without `--ros2`, is on a different `ROS_DOMAIN_ID`, or uses a different `--rmw`.
Solution: work down that list; the command prints it. Note `RMW` does **not**
have to equal the server's `--rmw` for fastdds/cyclonedds — verified
interoperable, both being RTPS. Only `zenoh` must match on both sides.

**Problem: topics appear but `echo`/`hz` gets nothing (local ROS 2)**
Cause: no RMW profile exported — shared-memory vs UDP mismatch. The most common
local failure, and completely silent.
Solution: `eval "$(bash scripts/ros_view.sh local-env)"`. The containers here are
unaffected.

**Problem: topics appear but `echo` hangs (in the containers too)**
Cause: subscribing to a best-effort sensor topic that is not currently producing
(sensor not `enable_for_ros`-ed, or a camera on a `-nullrhi` server); or a latched
topic subscribed volatile.
Solution: [[world-data]] `ros-topics` to see the enable state; use `PACKAGED=1`
or `WINDOW=1` for camera data ([[run-carla-server]]).

**Problem: a topic has a publisher but never any data, after a map change**
Cause: CARLA does not unregister ROS 2 publishers on episode teardown, so the old
actor's endpoint lingers (verified: `Publisher count` grows by one per map switch
when the same `ros_name` is reused).
Solution: ignore the zombie or restart the server; see [[load-map]].

**Problem: RViz opens empty / no map markers**
Cause: RViz started before the latched sample was published, or the demo stack is
not running (the markers come from the demo's `map_to_markers.py`, not the
server).
Solution: toggle the Map display checkbox to resubscribe, and make sure `demo`
is running (`--map-only` is enough for markers alone).

**Problem: `docker: permission denied` / daemon unreachable**
Cause: the user is not in the `docker` group.
Solution: fix the group membership outside this skill; every mode here needs the
daemon.

**Problem: zenoh: server and containers see nothing**
Cause: no Zenoh router. Zenoh is the only RMW here needing a separate process.
Solution: start `rmw_zenohd` before both, e.g.
`docker run --rm --net=host carla-rviz-humble-zenoh ros2 run rmw_zenoh_cpp rmw_zenohd`.

## Outputs

Console output (topic lists, samples, rates) and, for `rviz`, a window. `demo`
leaves the world as it found it. Nothing is written to the checkout.

Consumer-side detail — QoS matching, `use_sim_time`, the demo's helper processes,
the topic-name mapping — in
[`references/ros-consumers.md`](references/ros-consumers.md).
