# Building the native ROS 2 interface (`ROS2=1`)

Detail layer for the `ROS2=1` knob. CARLA's **native** ROS 2 interface publishes
DDS topics from inside the server — no `carla-ros-bridge`, no `rclcpp` in the
server. It is **compile-time gated**: a binary built without it can never enable
it, and no runtime flag can add it.

Everything below is read from the CARLA build system (`Util/BuildTools/*.sh`,
`Util/BuildTools/Linux.mk`, `Carla.Build.cs`) on the `ue4-dev` HEAD that ships
`LibCarla/source/carla/ros2/`. Marked **(verified)** where checked by running
the command here, **(source)** where read from the code but not executed.

## What `--ros2` reaches

`make CarlaUE4Editor ARGS="--ros2"` — one flag, three consumers, because
`Linux.mk` forwards `$(ARGS)` down the whole dependency chain
`CarlaUE4Editor → LibCarla.server.release → setup` (source):

| Script | What `--ros2` does there |
|---|---|
| `Setup.sh` | builds the middleware deps **from source** into `Build/`: `fast-dds-install`, `cyclone-dds-install`, `zenoh-install` (+ foonathan memory vendor). Long, network-heavy, cached — skipped if the install dir already exists. |
| `BuildLibCarla.sh` | builds the `carla_ros2` static lib (`LibCarla/cmake/ros2/`), all three middleware macros defined at once, compiled with `-fexceptions`. |
| `BuildCarlaUE4.sh` | writes `Ros2 ON` into `Unreal/CarlaUE4/Config/OptionalModules.ini`. |

`Carla.Build.cs` then reads that ini and, on `Ros2 ON`, defines `WITH_ROS2` and
links `carla_ros2` — every `#if defined(WITH_ROS2)` block in the plugin
(`ActorDispatcher`, `CarlaEngine`, every sensor) compiles in only then (source).

## The one-line ini (why the flag is sticky state)

`BuildCarlaUE4.sh` writes all module flags on a **single space-separated line**
(verified):

```
Fast_dds ON Unity ON Ros2 OFF Pytorch OFF Chrono OFF CarSim OFF SimReady ON
```

It rewrites that file on **every** run, so a later build **without** `--ros2`
silently flips `Ros2 OFF` and the next relink drops ROS 2 support.

**Worse: flipping the flag does not, by itself, change the binary** (verified
2026-08). `Carla.Build.cs` turns the ini into the `WITH_ROS2` *preprocessor
definition*, but UBT tracks source files and knows nothing about
`OptionalModules.ini`. With no source change it reuses the cached objects and
performs a **link-only** build — 10 actions, all `Link`, zero `Compile` — and
produces `libUE4Editor-Carla.so` with **0** `carla::ros2` symbols while every log
line reads `Success`. The server then accepts `--ros2` and publishes nothing.

Step 06 handles this: it reads the ROS 2 state from the **binary**
(`nm -DC … | grep carla::ros2`), not the ini, deletes the Carla module's object
cache (`Plugins/Carla/Intermediate/Build/*/*/*/Carla`) when the flag differs, and
after building asserts the symbols are present — 1112 of them on a correct build.

Consequences:

- `scripts/06_build_editor.sh` compares requested `ROS2=` against the ini and
  rebuilds on a flip — the plain "`.so` exists → skip" shortcut would otherwise
  hand back a binary that does not match the request.
- Anything that re-runs `BuildCarlaUE4.sh` must repeat the flag. That includes
  `make package`, whose `package:` target depends on `CarlaUE4Editor`
  ([[package-carla-ue4]] `ROS2=1`).
- `Fast_dds ON` is written unconditionally and means nothing here — only the
  `Ros2` token gates the build.

## Harmless stderr you will see (verified)

`make CarlaUE4Editor ARGS="--ros2"` also passes `--ros2` to
`BuildUE4Plugins.sh`, whose `getopt` does not declare it:

```
parse-options: unrecognized option '--ros2'
```

Not fatal: `getopt` still emits the options it does know, the script has no
`$?` guard and a `* ) shift ;;` catch-all, so the flag is dropped and the build
continues. The same is true for `Package.sh` and `BuildPythonAPI.sh`. Only
`Setup.sh`, `BuildLibCarla.sh` and `BuildCarlaUE4.sh` declare `--ros2`.

## Middleware built in

All three RMWs are compiled into one binary; the choice is made at **run** time
with `--rmw=` ([[run-carla-server]]):

| RMW | Notes |
|---|---|
| `fastdds` | default. Built with `-DBUILD_SHARED_LIBS=OFF`, patched to drop its vendored boost and to compile without exceptions; OpenSSL + libc++ come from the UE4 tree. |
| `cyclonedds` | uses a custom sertype (`CycloneDDSSertype.cpp`). |
| `zenoh` | needs a **router process** at run time (`rmw_zenohd`); ships a session config (`middleware/zenoh/config/zenoh_session_config.json5`). |

`MiddlewareFactory::IsMiddlewareAvailable` decides at run time whether the
requested one is present; the server logs `Available: …` and **disables ROS 2**
for the session when it is not (source).

## Verifying the build

In order of strength:

1. `carla_ros2_ini_state` (env.sh) → `on`. Necessary but **weak** — it only proves
   the flag reached the ini, not the compile (see above).
2. The dependency installs exist: `Build/fast-dds-install`,
   `Build/cyclone-dds-install`, `Build/zenoh-install`.
3. `libcarla_ros2.a` exists (verified path on this HEAD:
   `Build/libcarla-fastdds-install.release/LibCarla/cmake/ros2/libcarla_ros2.a`).
4. **The binary check** — this is the one that catches the link-only trap:
   ```bash
   nm -DC Unreal/CarlaUE4/Plugins/Carla/Binaries/Linux/libUE4Editor-Carla.so \
     | grep -c carla::ros2        # 1112 on a correct build, 0 on a relink-only one
   ```
5. **End-to-end proof**: run with `--ros2` and see topics
   ([[run-carla-server]] `ROS2=1` → `/clock`, `/carla/map`). A server built
   without ROS 2 accepts `--ros2` and simply logs nothing about it.

## Cost

The dependency stage adds a source build of three middlewares (git clones +
CMake/Ninja each) on top of the normal build, once per checkout. `carla_ros2`
itself is a small static lib; the editor relink is the usual minutes. Re-running
with `ROS2=1` after the deps exist only re-links.

## Not built by this flag

- **`carla-ros-bridge`** — a separate ROS package outside this repo. The native
  interface replaces it for the topics it covers; it does not build it.
- **A ROS 2 installation** — nothing here needs `/opt/ros` present. The server
  speaks DDS directly. You only need ROS 2 (or the demo's Docker images, see
  [[visualize-ros-rviz]]) on the *consumer* side.
- **Message-type generation** — there is no IDL step: the message structs are
  hand-written PODs ([[add-ros-message-type]]).
