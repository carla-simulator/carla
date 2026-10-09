# Running a server with the native ROS 2 interface

Detail layer for `ROS2=1`. Read from the CARLA sources on the `ue4-dev` HEAD that
ships `LibCarla/source/carla/ros2/` (`CarlaSettings.cpp`, `CarlaEngine.cpp`,
`ActorDispatcher.cpp`, `ROS2.cpp`, `Sensor.cpp`, `MultiStreamState.h`) plus
`PythonAPI/examples/ros2/README.md`, and **verified live** (2026-08) against a
ROS-2-enabled build and cooked package, with ROS 2 Humble as the consumer.

## The two independent switches

| Switch | Where | Effect |
|---|---|---|
| build-time | `Ros2 ON` in `Config/OptionalModules.ini` → `WITH_ROS2` | the publisher code exists in the binary at all |
| **run-time** | `--ros2` on the command line | `ROS2::Enable(...)` for this session |

Both are required. A ROS-2-built server started **without** `--ros2` publishes
nothing; a non-ROS-2 server started **with** `--ros2` logs nothing and publishes
nothing. That silent second case is why `check_env.sh` reads the ini.

Note the ini is *intent*, not proof: UBT does not track it, so flipping the flag
can relink a plugin that still has no ROS 2 code in it. The binary is the truth —
`nm -DC libUE4Editor-Carla.so | grep -c carla::ros2` (verified: 1112 symbols on a
correct build, 0 on a relink-only one). [[build-carla-ue4]] step 06 now checks
this for you.

## Subscribers on this host need the RMW profile (verified)

**The single most confusing failure.** CARLA's Fast DDS is built with shared
memory; a stock local ROS 2 install does not match it, so `ros2 topic list` shows
the topics (discovery is UDP) and **no data ever arrives** — no error on either
side. The checkout ships the profiles that force UDP-only; export the one for your
RMW in **every** shell that subscribes:

```bash
set +u                                   # ROS setup.bash breaks under set -u
source /opt/ros/humble/setup.bash
export FASTRTPS_DEFAULT_PROFILES_FILE=$CARLA_UE4_ROOT/PythonAPI/examples/ros2/config/fastrtps-profile.xml
# cyclonedds instead: export CYCLONEDDS_URI=file://$CARLA_UE4_ROOT/PythonAPI/examples/ros2/config/cyclonedds.xml
ros2 topic hz /clock                     # ~4 kHz on an async -nullrhi server
```

Without the profile that same `hz` prints nothing at all. The containers in
[[visualize-ros-rviz]] mount these files at `/config` already, which is why the
Docker path works out of the box; `ros_view.sh local-env` prints the exports for a
local shell.

## Flags

```bash
ROS2=1 bash scripts/run_server.sh                       # fastdds, default domain
ROS2=1 RMW=cyclonedds bash scripts/run_server.sh
ROS2=1 RMW=zenoh ROS_DOMAIN_ID=5 bash scripts/run_server.sh
CARLA_TARGET=~/CARLA_0.9.16 ROS2=1 bash scripts/run_server.sh   # a downloaded release
PACKAGED=1 ROS2=1 bash scripts/run_server.sh            # a package cooked in the checkout
```

They become `--ros2 [--rmw=<v>] [--ros-domain-id=<n>]` on the server's own
command line. Double dash is required: `CarlaSettings` parses them as
`FParse::Param(TEXT("-ros2"))` / `FParse::Value(TEXT("-rmw="))`, which match
`--ros2` / `--rmw=`.

- **`--rmw=`** — `fastdds` (default), `cyclonedds`, `zenoh`. An unrecognised
  value or one not compiled in: the server logs `Available: …` and **disables
  ROS 2 for the session** instead of failing.
- **`--ros-domain-id=`** — `0..232`. Resolution order is CLI → the server
  process's own `ROS_DOMAIN_ID` env var → `0`. Out of range logs an error and
  falls back to the default domain. **A domain mismatch looks exactly like a
  broken build**: no topics, no error on either side.

### The editor (`make launch`) case

`BuildCarlaUE4.sh --ros2` forwards `--ros2` (and `--rmw=`/`--ros-domain-id=`) to
the launched editor's own command line, so on this HEAD
`make launch ARGS="--ros2"` covers both build and run — the
`--editor-flags='--ros2'` seen in older instructions is a harmless duplicate.
`bash scripts/run_server.sh` launches `UE4Editor` directly and passes the flags
itself.

### Zenoh needs a router

`zenoh` is the only RMW with an extra process: start a Zenoh router **before**
the server, or nothing matches.

```bash
# from a ROS 2 environment / container that has rmw_zenoh_cpp
ros2 run rmw_zenoh_cpp rmw_zenohd
```

The bundled session config is
`LibCarla/source/carla/ros2/middleware/zenoh/config/zenoh_session_config.json5`.
The demo image `carla-rviz-<distro>-zenoh` ([[visualize-ros-rviz]]) contains the
router if you have no local ROS 2.

## What appears when it works

Three topics exist as soon as ROS 2 is enabled — no actors needed:

| Topic | Type | Notes |
|---|---|---|
| `rt/clock` | `rosgraph_msgs/Clock` | every tick; the reason subscribers should run with `use_sim_time`. Async `-nullrhi` ticks free-running, measured **~4000 Hz** |
| `rt/carla/map` | `std_msgs/String` | full OpenDRIVE of the current map, **latched** (transient_local), re-published on every map load ([[load-map]]) |

`rt/tf` is **not** in that list: it appears only once a **sensor** publishes
(verified — a hero vehicle alone produces no `rt/tf`). CARLA emits sensor→parent
transforms; the `map`→vehicle transform has to come from outside, which is exactly
what the demo's `ego_tf_broadcaster.py` does ([[visualize-ros-rviz]]).

Reading the latched map needs the matching QoS request, or the subscriber waits
forever for a sample that only fires on episode start:

```bash
ros2 topic echo --once --qos-durability transient_local --qos-reliability reliable \
                --full-length /carla/map        # --full-length or it truncates at 128 chars
```

The `rt/` prefix is the DDS-level name; a ROS 2 node sees these as `/clock`,
`/carla/map`, `/tf`. Per-actor topics and the `enable_for_ros()` requirement are
in [[world-data]] (`ros-topics`), [[create-sensor]] and [[read-sensor]].

## Readiness

The RPC port opening does **not** mean topics are up. Order of appearance:
RPC port → episode begins → `rt/carla/map` + `rt/clock`. So:

```bash
ROS2=1 bash scripts/run_server.sh >/tmp/carla_ros2.log 2>&1 &
until nc -z 127.0.0.1 2000; do sleep 1; done          # RPC ready
# then, from a ROS 2 environment on the same domain:
ros2 topic list | grep -E '/clock|/carla/map'          # ROS ready
```

No ROS 2 CLI on the box? Use the demo image ([[visualize-ros-rviz]]), or fall
back to the RPC-side proof: `world-data ros-topics` lists what the server
*should* be publishing and whether each actor is registered.

## Failure modes

| Symptom | Cause | Fix |
|---|---|---|
| RPC works, no ROS topics at all | binary not built with ROS 2, or `--ros2` missing | `carla_ros2_ini_state` → rebuild ([[build-carla-ue4]] `ROS2=1`); check the launch line |
| `ROS2: unrecognized --rmw value 'x'` then nothing | typo'd RMW; ROS 2 disabled for the session | use `fastdds`/`cyclonedds`/`zenoh` |
| `--rmw='zenoh' is not compiled into this binary` | that middleware missing from the build | rebuild, or pick an available one from the logged `Available:` list |
| new server dies instantly with `bind: Address already in use` then `Signal 11` | the previous server still holds the port — `pkill -x CarlaUE4-Linux-Shipping` matched nothing (comm truncated to 15 chars) | `pkill -x CarlaUE4-Linux-`, then `until ! nc -z 127.0.0.1 2000; do sleep 1; done` |
| topics **listed** but no data ever arrives (`hz` silent) | the local subscriber is not using the checkout's UDP-only RMW profile (shared-memory mismatch) — **most common local failure** | export `FASTRTPS_DEFAULT_PROFILES_FILE` / `CYCLONEDDS_URI` as above |
| topics exist but the subscriber sees none | domain mismatch, or (zenoh) no router | match `ROS_DOMAIN_ID` on both sides; start `rmw_zenohd` |
| `echo --once /carla/map` never returns | latched sample needs a transient_local request; it is re-sent only on episode start | add `--qos-durability transient_local --qos-reliability reliable` |
| a topic has a publisher but no data, after a map change | publishers leak across episodes (see [[load-map]]) | ignore the zombie, or restart the server |
| `/clock` and `/carla/map` fine, sensor topics missing | the sensor was never enabled for ROS | `sensor.enable_for_ros()` — see [[create-sensor]] `--ros` |
| topics vanish after a map change | actors (and their publishers) are destroyed with the episode | re-spawn; `rt/carla/map` re-latches automatically |

## Multi-server hosts

The ROS 2 domain, not the RPC port, separates two servers on one machine. Two
servers on ports 2000/2002 with the same domain publish onto the **same** topic
names and interleave. Give each its own `ROS_DOMAIN_ID`.
