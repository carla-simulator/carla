---
name: run-carla-server
description: Launches a CARLA RPC server a carla.Client can connect to, working out for itself what you pointed it at — a downloaded/extracted release, a package cooked inside a checkout, or a source checkout run through the UE4 editor — headless or windowed, and shuts it down cleanly. ROS2=1 starts it with the native ROS 2 interface active (--ros2, --rmw=fastdds/cyclonedds/zenoh, --ros-domain-id). Use when the user asks to "run/start the CARLA server", "boot CARLA headless", "launch CARLA with a window", "run the CARLA I downloaded", "start CARLA with ROS2 enabled", or "serve a packaged CARLA build".
license: MIT
compatibility: Linux. Needs one of: an extracted CARLA release (nothing else required), a Dist/ package from package-carla-ue4, or a source checkout plus UE4_ROOT and fetched content for editor mode. WINDOW=1 needs an X display + NVIDIA GPU. The verify client needs an active CARLA client env with a matching `carla` version (install-python-api sets that up).
metadata:
  group: ue4
  requires: build-carla-ue4
  prerequisites: scripts/check_env.sh
  reference: references/lessons.md
  ros2: references/ros2.md
---

# Run a CARLA server

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Starts a CARLA server a `carla.Client` can connect to. Use it to verify authored
content ([[add-carla-vehicle]]), run traffic scenarios, or feed the MCP's
live-simulator tools. It is NOT for asset editing — that is [[ue4-editor-python]]
(editor commandlet, no RPC).

## Point it at a CARLA; it works out the rest

You name **one path**, the skill detects which of the three shapes it is and
launches accordingly:

| Detected | What it found | Rendering | Needs UE4_ROOT |
|---|---|---|---|
| `package` | `<path>/CarlaUE4.sh` — a downloaded/extracted release | real | no |
| `dist` | `<path>/Dist/CARLA_*/LinuxNoEditor/CarlaUE4.sh` — cooked in a checkout | real | no |
| `editor` | `<path>/Unreal/CarlaUE4/CarlaUE4.uproject` — a source checkout | none (`-nullrhi`) or windowed | **yes** |

```bash
CARLA_TARGET=~/CARLA_0.9.16 bash scripts/run_server.sh      # a download
CARLA_UE4_ROOT=~/carla      bash scripts/run_server.sh      # a checkout
DETECT=1 CARLA_TARGET=~/CARLA_0.9.16 bash scripts/run_server.sh   # report, launch nothing
```

Resolution order is `CARLA_TARGET` → `CARLA_PACKAGE_ROOT` → `CARLA_UE4_ROOT` →
`$PWD`, and a path you name **explicitly** that holds no CARLA is an error, never
a silent fallback to a different install.

A checkout that has cooked a package can be run either way, so detection prefers
the cooked build (it renders properly and boots faster). Force the choice with
`RUN_MODE`:

| Knob | Effect |
|---|---|
| `RUN_MODE=auto` | default — cooked build if present, else editor |
| `RUN_MODE=package` | only a cooked build; error if there is none (`PACKAGED=1` is the old spelling) |
| `RUN_MODE=editor` | run the source checkout through UE4Editor even if a package exists |
| `WINDOW=1` | windowed instead of headless — works for both cooked builds and the editor |

The editor caveat (build-carla-ue4 L17): **uncooked** content has no mesh distance
fields, so the real renderer crashes headless. That is why `editor` mode defaults
to `-nullrhi` (no images at all) and `WINDOW=1` disables DF generation. For camera
and lidar images, run a cooked build.

**Boot time is tens of seconds, not a fixed number** — measured here on Town02
headless: ~38 s cold (32 s of it `LoadMap`), less when warm; heavy maps and a
cold shader cache are slower. Always poll the port (S4), never sleep a guess.

RPC + physics + Traffic Manager work in **all** modes; pick the cheapest one that
covers what you're testing. (PACKAGED mode is encoded from build docs + L17; the
uncooked modes are live-verified.)

> Gotchas live in [`references/lessons.md`](references/lessons.md) — read before
> debugging. `S#` citations below point at it.

## Instructions

```
Run Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Pick a mode and launch (backgrounded), poll the RPC port
- [ ] Step 3: Verify a client round-trips
- [ ] Step 4: Stop the server cleanly (see S3 — never pkill -f the uproject)
```

## Prerequisites

- Roots resolve via `scripts/env.sh` (both overridable): `UE4_ROOT` (uncooked
  modes launch the editor) and `CARLA_UE4_ROOT` (the checkout to serve). Export
  them, or run from inside the checkout.
- `editor` mode only: UE4 built + content fetched ([[build-carla-ue4]] steps 03, 05).
- A cooked build: either an extracted release (`CARLA_TARGET`) or `Dist/CARLA_*`
  from [[package-carla-ue4]]. Needs no UE4_ROOT and no content checkout.
- Verify client: any active CARLA client env (the wheel installed by build step
  04); no manager is assumed.

## Quick start

```bash
cd skills/run-carla-server
bash scripts/check_env.sh

# headless smoke-test server, backgrounded DETACHED (see the note below)
setsid nohup bash scripts/run_server.sh </dev/null >/tmp/carla_server.log 2>&1 &
until nc -z 127.0.0.1 2000; do sleep 1; done     # poll, don't sleep blindly

# ... use it (spawn_test.py, MCP tools, any carla.Client) ...

pkill -x UE4Editor                                # clean stop (see S3!)
```

## ROS 2 native interface (`ROS2=1`, opt-in)

Orthogonal to the three modes above — it composes with all of them:

```bash
ROS2=1 bash scripts/run_server.sh                          # fastdds, domain 0
ROS2=1 RMW=zenoh ROS_DOMAIN_ID=5 bash scripts/run_server.sh
CARLA_TARGET=~/CARLA_0.9.16 ROS2=1 bash scripts/run_server.sh   # a download, sensors work
```

Adds `--ros2 [--rmw=<v>] [--ros-domain-id=<n>]` to the launched binary, so the
server publishes DDS topics itself (no `carla-ros-bridge`). Four things decide
whether you see anything:

1. **The binary must be BUILT with ROS 2** — `--ros2` on a plain build is a
   silent no-op. `check_env.sh` reads `Config/OptionalModules.ini` and FAILs when
   `ROS2=1` meets a `Ros2 OFF` checkout. Build it with [[build-carla-ue4]]
   `ROS2=1`, cook it with [[package-carla-ue4]] `ROS2=1`.
2. **`RMW=`** — `fastdds` (default) · `cyclonedds` · `zenoh`. A bad or
   not-compiled-in value **disables ROS 2 for the session** with a log line, it
   does not fail the boot. `zenoh` also needs a router (`rmw_zenohd`) started
   first. `fastdds` and `cyclonedds` **interoperate** — verified: a
   `rmw_fastrtps_cpp` subscriber reads a `--rmw=cyclonedds` server at the same
   rate as a native one, since both speak RTPS. Only `zenoh` is a separate
   protocol and must match on both sides.
3. **The domain must match** the subscriber side (`ROS_DOMAIN_ID`, 0..232, CLI →
   env → 0). A mismatch is indistinguishable from a broken build: no topics, no
   error. Verified both ways: on the server's domain the topics appear, on any
   other domain nothing does.

**Restarting: wait for the port, and mind the process name.** The packaged
server's `comm` is truncated to 15 characters, so `pkill -x
CarlaUE4-Linux-Shipping` matches **nothing** and the old server keeps port 2000.
The next launch then dies with `bind: Address already in use` → `Signal 11`, which
reads like a rendering crash and is not one. Verified — use:

```bash
pkill -x CarlaUE4-Linux-                              # the truncated name
until ! nc -z 127.0.0.1 2000; do sleep 1; done        # THEN wait for release
```
4. **`-nullrhi` still has no cameras.** Non-image sensors (lidar, IMU, GNSS,
   collision), `rt/clock`, `rt/tf` and `rt/carla/map` publish in every mode;
   camera topics need `WINDOW=1` or `PACKAGED=1`, same rule as the RPC path.

Sensors also need `enable_for_ros()` to publish at all ([[create-sensor]] `--ros`).
Readiness is two-stage — RPC port, then topics:

```bash
until nc -z 127.0.0.1 2000; do sleep 1; done      # RPC
ros2 topic list | grep -E '/clock|/carla/map'      # ROS (from a ROS 2 env)
```

No ROS 2 installed here: verify from a container ([[visualize-ros-rviz]]) or
RPC-side with [[world-data]] `ros-topics`. Full flag semantics, zenoh router and
failure table: [`references/ros2.md`](references/ros2.md).

## Reference

- **Map choice:** light maps (Town01/Town02) minimise first-load time. Uncooked
  modes take the map as arg 1; the packaged build boots its cooked default —
  switch with `client.load_world("Town02")` instead.
- **Ports:** `scripts/run_server.sh [MAP] [RPC_PORT]`; streaming port is always
  RPC+1. Run parallel servers on 2000/2002/2004...
- **Readiness:** the RPC port opening is the signal; log line
  `LogCarlaServer: Initialized CarlaServer` appears at the same time.
- **Shutdown:** `pkill -x UE4Editor` (uncooked) /
  `pkill -x CarlaUE4-Linux-` (packaged — **not** `...-Shipping`, see below). **Never**
  `pkill -f CarlaUE4.uproject` — it kills your own shell (S3).

## Verify

A server is up when a client round-trips (run with your CARLA client env active —
venv/conda/system, whichever holds the wheel from build step 04):

```bash
python -c "import carla; print(carla.Client('127.0.0.1', 2000).get_server_version())"
```

Artifacts to check on failure: the server log's first `Signal 11` (not the last —
the trailing CrashReportClient crash is a red herring, S1) and whether the port
ever opened.

## Troubleshooting

**Error: server dies right after the port opens, log ends `close: Bad file descriptor` + Signal 11**
Cause: it was backgrounded with a plain `&`, so it inherited the launching shell's
stdin and died when that shell exited. Not a rendering fault (verified).
Solution: `setsid nohup bash scripts/run_server.sh </dev/null >log 2>&1 &`.

**Error: server SIGSEGVs seconds after opening the RPC port (uncooked)**
Cause: the real renderer dereferences a null mesh distance field — uncooked
content has none (S1).
Solution: use the default `-nullrhi` mode (no render thread), or `WINDOW=1`
(disables DF generation via `-ini:` override). For sensor images, cook and run
`PACKAGED=1`.

**Error: stopping the server killed the calling shell (exit 144)**
Cause: `pkill -f CarlaUE4.uproject` matches the launching shell's own args (S3).
Solution: `pkill -x UE4Editor` (uncooked) or `pkill -x CarlaUE4-Linux-`
(packaged) — exact process names.

**Error: client connects after a fixed sleep but times out**
Cause: it raced first-load shader compilation; boot time varies (S4).
Solution: poll the port — `until nc -z 127.0.0.1 <port>; do sleep 1; done`. Use
`nc -z` / `ss -ltn`, not the bash `/dev/tcp` idiom (fails under zsh, S4).

**Error: `nothing runnable found` / `named explicitly but holds no CARLA build`**
Cause: no path resolved to a release, a cooked package, or a checkout — or the path
you named is not a CARLA install.
Solution: the error lists the three shapes it looked for. Set `CARLA_TARGET` to an
extracted release, or cook one ([[package-carla-ue4]]), or point at a checkout with
`UE4_ROOT` set for editor mode. `DETECT=1` shows what a given path resolves to.

**Error: `RUN_MODE=package found no matching build`**
Cause: only a source checkout is present.
Solution: cook a package, or drop `RUN_MODE`/`PACKAGED=1` to run the editor.

## Outputs

None persisted — a running process serving RPC on the chosen port.
