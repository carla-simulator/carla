---
name: run-carla-ue58-server
description: Starts a CARLA UE 5.8 server from a ue58-dev tree — the editor binary in -game mode (the working headless server), the packaged shipping build, or the full editor — with offscreen/nullrhi rendering, custom RPC and streaming ports, and native ROS 2 via the single-dash -ros2 flag. Probes a running server, and stops one cleanly despite the 15-character process-name truncation. Use when the user asks to "run/start CARLA UE5", "launch a ue58 server", "run headless", "publish ROS 2 topics from UE5", or a client cannot reach the simulator.
license: MIT
compatibility: Linux with a built ue58-dev tree (engine + carla-unreal) or a package. Rendering modes need Vulkan; -nullrhi does not. ROS 2 needs -DENABLE_ROS2=ON at build time. Verified live against ue58-dev HEAD 718efd7cc, engine 5.8, CARLA 0.10.0 — editor -game and packaged shipping, with and without ROS 2.
metadata:
  group: ue58
  prerequisites: scripts/check_env.sh
  reference: references/running.md
---

# Run a CARLA UE 5.8 server

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Three ways to get a server, and the choice matters more than on UE4:

| Mode | What it is | Use it when |
|---|---|---|
| `game` | the **editor binary** with `-game` | **default.** Full client support, map switching, 29 maps visible |
| `package` | `Build/<preset>/Package/.../CarlaUnreal.sh` | shipping a build; accept the map limitations below |
| `editor` | the full editor GUI | authoring, visual debugging |

There is a fourth thing that looks like the answer and is not:
`Unreal/CarlaUnreal/Binaries/Linux/CarlaUnreal`, the uncooked Development game
target. It **cannot start** — it is built to load cooked content and the tree has
none, so it dies on the missing global shader cache. `nurec/README.md` recommends
it anyway. Use `game` mode instead.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: See what is runnable (detect)
- [ ] Step 3: Start it
- [ ] Step 4: Probe it before running your real client
- [ ] Step 5: Stop it
```

### Step 2-3: Start

```bash
source scripts/env.sh
bash scripts/run_server.sh detect

# the normal case: headless, backgrounded, waits until the RPC port answers
DETACH=1 bash scripts/run_server.sh game Town10HD_Opt

# with native ROS 2
DETACH=1 ROS2=1 bash scripts/run_server.sh game Town10HD_Opt

# the packaged server
DETACH=1 bash scripts/run_server.sh package Town10HD_Opt

# a second server alongside the first
PORT=3000 DETACH=1 bash scripts/run_server.sh game
```

| Knob | Effect |
|---|---|
| `DETACH=1` | background it and wait for the RPC port; log to `/tmp/carla-ue58-<port>.log` |
| `PORT` / `TM_PORT` | RPC port (streaming is `PORT+1`); default 2000 / 8000 |
| `ROS2=1` | append `-ros2`; needs a build with `ENABLE_ROS2=ON` |
| `RMW=` | `fastdds` (default), `cyclonedds`, `zenoh` |
| `ROS_DOMAIN_ID=` | must match the subscriber side |
| `WINDOW=1` | render to a window instead of offscreen; implies `-notrace` |
| `TRACE=1` | keep UnrealTrace enabled under `WINDOW=1` |
| `RES=WxH` | window size, `WINDOW=1` only (e.g. `RES=1920x1080`) |
| `FULLSCREEN=1` | exclusive fullscreen, `WINDOW=1` only; default is `-windowed` |
| `NULLRHI=1` | no RHI at all — **see the warning below** |
| `QUALITY=Low` | `-quality-level=Low` |
| `EXTRA=` | anything else |

**`DETACH=1` is not a convenience.** A plain trailing `&` leaves the server
holding the shell's stdio; when that goes away it dies with
`close: Bad file descriptor` and then Signal 11. The script uses
`setsid nohup … </dev/null &`.

### Filling the screen (and why not `-fullscreen` by default)

`WINDOW=1` alone leaves the window at whatever size the engine picks. To fill a
display, give it the size and leave it a normal window:

```bash
WINDOW=1 RES=1920x1080 DETACH=1 bash scripts/run_server.sh game Town10HD_Opt
```

`FULLSCREEN=1` asks for exclusive fullscreen instead, and it is deliberately not
the default: an overlay window — a terminal being screen-recorded on top of the
simulator, for instance — does not reliably stay above an exclusive-fullscreen
window, and some capture paths flicker. Sized-and-windowed looks the same on
camera and composites properly.

### `WINDOW=1` and the UnrealTrace crash

A windowed launch started straight after a headless one died on startup:

```
SIGSEGV: invalid attempt to read memory at address 0x00007dcfcdff1a10
UE::Trace::Private::Writer_WorkerThread   [Trace/Writer.cpp:1082]
  -> TraceAuxiliaryOnMessageCallback      [TraceAuxiliary.cpp:1455]
  -> FLLMTracker::TrackAllocation         [HAL/LLM/LLM.cpp:6630]
```

Inside UnrealTrace's own writer thread — not the window, not the RHI — with a
stale `UnrealTraceServer` daemon still resident from the previous run. The same
command came up clean with `-notrace`, so `WINDOW=1` now passes it by default
and `TRACE=1` puts it back. If a launch still dies this way, clear the daemon:

```bash
pkill -f "[U]nrealTraceServer"
```

### `NULLRHI=1` will crash the server if you spawn a camera

Not "produces no images" — **crashes**. With no RHI there is no render target, so
the camera readback path dereferences null on the render thread:

```
Unhandled Exception: SIGSEGV: invalid attempt to read memory at address 0x58
libUnrealEditor-Carla.so!ImageUtil::ReadImageDataBegin(...)
    [Plugins/Carla/Source/Carla/Sensor/ImageUtil.cpp:224]
```

Measured on this branch. `-nullrhi` is fine for traffic, physics, waypoints and
non-image sensors; use the default `-RenderOffScreen` for anything with a camera.

### Step 4: Probe

```bash
bash scripts/run_server.sh probe
```

Reports server/client versions, `get_world()` latency, map and spawn-point count,
world settings, blueprint count, actor count and the available-map list. Run it
before your real client so a failure is attributable.

### Step 5: Stop

```bash
bash scripts/run_server.sh stop
```

**Process names are truncated to 15 characters in `/proc`**, so the shipping
binary appears as `CarlaUnreal-Lin`, never `CarlaUnreal-Linux-Shipping`. A
`pkill -x CarlaUnreal-Linux-Shipping` matches nothing, exits as if it worked, and
leaves the port held — the next launch then dies with
`bind: Address already in use` followed by Signal 11. `stop` uses the truncated
name and then waits on the **port**, not the process.

Also: **the server takes SIGSEGV on SIGTERM on this branch**, and the Python
client segfaults at interpreter teardown after completing its work. Exit codes are
useless as a pass/fail signal here; check the port and the results instead.

## Packaged vs game mode — measured, not assumed

Both were exercised on this branch. The packaged server is more usable than the
older report suggests, but its map handling is broken:

| | `game` | `package` |
|---|---|---|
| `get_world()` | works | **works** |
| spawn vehicle | works | **works** |
| `get_random_location_from_navigation()` | works | **works** |
| `get_available_maps()` | **29 maps** | **`[]` — empty** |
| `load_world('Town10HD_Opt')`, `'Town_C'` | works | works |
| `load_world('Town12')`, `'Town13')` | works | **fails** (`std::exception`) |

Root cause of the map failures: two raw-filesystem checks that cannot see inside a
`.pak` — `UCarlaStatics::GetAllMapNames()` uses
`IFileManager::FindFilesRecursive(..., "*.umap")` and `LoadNewEpisode()` gates on
`FPaths::FileExists()`. AssetRegistry-based discovery exists on
`ue58-dev-windows` and is **not merged into `ue58-dev`**.

So: use `package` for a fixed-map deployment, `game` whenever you need discovery
or map switching.

## ROS 2

```bash
DETACH=1 ROS2=1 bash scripts/run_server.sh game Town10HD_Opt
```

The server logs its middleware on startup:

```
LogCarla: ROS2: Creating ROS2 Instance...
LogCarla: ROS2: enabled with middleware 'fastdds'.
LogCarla: ROS2: Fast-DDS transport: UDPv4 only (default; set
          FASTDDS_BUILTIN_TRANSPORTS=DEFAULT to re-enable shared memory)
```

That last line is the important UE5.8 change: the server defaults to **UDPv4
only**, so a local ROS 2 subscriber sees the topics with **no profile file**. On
UE4 you needed `FASTRTPS_DEFAULT_PROFILES_FILE` to defeat shared-memory
transport; here you do not.

`-ros2` is single-dash. UE 5.8's `FParse::Param` strips a leading dash from the
search string, so `TEXT("-ros2")` matches `-ros2` — the opposite of UE4, where
`--ros2` was required. Current `ue58-dev` also accepts `--ros2` through an
explicit `Strifind` fallback in `CarlaSettings.cpp`, but `-ros2` works on every
ue58 commit.

**Sensors must be enabled individually, and the call moved.** In UE 5.8
`enable_for_ros()` is on **`Sensor`**, not on `Actor` as in 0.9.16:

```python
s = world.spawn_actor(cam_bp, transform, attach_to=vehicle)
s.enable_for_ros()          # without this the sensor is never ticked
```

Verified: `hasattr(sensor, 'enable_for_ros')` is `True`, `hasattr(vehicle, …)` is
`False`.

Topics observed from one ROS-enabled camera on a `hero` vehicle:

```
/carla/hero/front/camera_info
/carla/hero/front/image          ~5-6 Hz with sensor_tick=0.1, frame_id "front"
/carla/hero/vehicle_control_cmd  (subscriber, created for role_name=hero)
/clock                           ~640 Hz in async mode
/tf
```

`ros_name` on the blueprint sets both the topic segment and the `frame_id`; the
parent chain builds the path (`hero` → `front`).

**There is no `/carla/map` topic on UE 5.8.** `CarlaMapPublisher` was removed, so
the latched OpenDRIVE topic that exists on UE4 is gone — `"map"` survives only as
the root TF frame id.

## Examples

**Example 1: "start a UE5 server so I can run a client"**

`DETACH=1 bash scripts/run_server.sh game Town10HD_Opt`, then `probe`. Roughly 20 s
to the RPC port on a warm build; much longer on the first launch after a rebuild,
which compiles shaders.

**Example 2: "publish camera images to ROS 2"**

`DETACH=1 ROS2=1 bash scripts/run_server.sh game`, spawn a `hero` vehicle and a
camera with `ros_name`, call `enable_for_ros()`, then `ros2 topic list`. No DDS
profile file needed.

**Example 3: "the client can't find any maps"**

You are on a packaged server. `get_available_maps()` cannot work inside a `.pak`
on this branch. Switch to `game` mode.

**Example 4: "run two servers"**

`PORT=3000` on the second. Ports 3000–3002 must all be free; the preflight checks
all three.

## Troubleshooting

**Problem: `bind: Address already in use`, then Signal 11**
Cause: a stale server still holds the port — very often because `pkill -x` was
given the untruncated process name.
Solution: `bash scripts/run_server.sh stop`. The preflight refuses to launch onto
a busy port and prints both remedies.

**Problem: the server dies the moment a camera is spawned**
Cause: `-nullrhi`. See the warning above.
Solution: drop `NULLRHI=1`.

**Problem: `The global shader cache file ... is missing. Your application is built to load COOKED content`**
Cause: you launched `Unreal/CarlaUnreal/Binaries/Linux/CarlaUnreal`, the uncooked
game target.
Solution: `game` mode (the editor binary with `-game`).

**Problem: `-ros2` produces no topics**
Cause: the build has `ENABLE_ROS2=OFF`, or the flag never reached the server.
Solution: `detect` prints the cached `ENABLE_ROS2`; rebuild with `ROS2=1` if it is
off ([[build-carla-ue58]]).

**Problem: topics exist but a local subscriber sees nothing**
Cause: domain mismatch, or a stale DDS profile file forcing shared memory.
Solution: match `ROS_DOMAIN_ID` on both sides. Unset
`FASTRTPS_DEFAULT_PROFILES_FILE` — UE 5.8 already defaults to UDPv4 only, and an
inherited UE4-era profile can undo that.

**Problem: the run exits non-zero even though everything worked**
Cause: the server takes SIGSEGV on SIGTERM, and the client segfaults at teardown.
Solution: judge by results, not exit status. Known, cosmetic, and the reason no
script here gates on `$?` from the server.

**Problem: `load_world` fails for a large map**
Cause: packaged mode; see the table above.
Solution: `game` mode.

**Problem: the RPC port never comes up**
Cause: usually first-launch shader compilation, which can far exceed the 120 s the
script waits.
Solution: watch the log it prints; re-run `probe` later.

## Outputs

A running server on `PORT` (log at `/tmp/carla-ue58-<port>.log` when detached),
optionally publishing ROS 2 topics. `probe` prints a connection report; `stop`
terminates it and waits for the port to be released.

Launch flags, the mode comparison, ROS 2 topic naming and the shutdown behaviour
are in [references/running.md](references/running.md).
