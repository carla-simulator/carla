# Running a UE 5.8 CARLA server

Everything below was exercised on `ue58-dev` HEAD `718efd7cc`, engine 5.8,
CARLA 0.10.0 — editor `-game` and packaged shipping, with and without ROS 2.

## The three modes

### `game` — the editor binary with `-game`

```bash
$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Linux/UnrealEditor \
    $CARLA_UE58_ROOT/Unreal/CarlaUnreal/CarlaUnreal.uproject \
    Town10HD_Opt -game -vulkan -RenderOffScreen \
    -carla-rpc-port=2000 -carla-streaming-port=2001
```

The default choice. Runs against the uncooked tree, so it needs no package, and it
is the only mode with working map discovery.

### `package` — the shipping build

```bash
Build/<preset>/Package/Carla-<ver>-Linux-Shipping/Linux/CarlaUnreal.sh \
    Town10HD_Opt -RenderOffScreen -carla-rpc-port=2000
```

The launcher is a two-line `sh` shim that chmod +x's and execs
`CarlaUnreal/Binaries/Linux/CarlaUnreal-Linux-Shipping`, passing arguments through.

### `editor` — the GUI

`cmake --build Build/<preset> --target launch` runs
`UnrealEditor CarlaUnreal.uproject -<rhi> $CARLA_LAUNCH_ARGS`, where
`CARLA_LAUNCH_ARGS` is a **configure-time** cache variable (default `-log`). To
change editor launch flags you must re-configure, which is why this skill invokes
the binary directly for server use instead.

`launch` depends on `carla-unreal-editor` and `check-unreal-content`;
`launch-only` skips the rebuild.

### The one that does not work

`Unreal/CarlaUnreal/Binaries/Linux/CarlaUnreal` — the uncooked Development game
target — exits with:

```
The global shader cache file '.../Engine/GlobalShaderCache-VULKAN_SM6.bin' is
missing. Your application is built to load COOKED content. No COOKED content was
found -> Exiting abnormally (error code: 1)
```

`nurec/README.md` recommends it as the server. It cannot be used on an uncooked
tree.

## Flags

| Flag | Meaning |
|---|---|
| `-carla-rpc-port=N` | RPC port |
| `-carla-streaming-port=N` | sensor streaming port (conventionally RPC+1) |
| `-carla-primary-host=` / `-carla-primary-port=` | multi-server (primary/secondary) |
| `-RenderOffScreen` | render without a window — the headless default |
| `-nullrhi` | no RHI at all. **Crashes on any camera sensor** |
| `-vulkan` | RHI selection; matches `CARLA_UNREAL_RHI` |
| `-quality-level=Low\|Epic` | scalability |
| `-ros2` | native ROS 2 (single dash on 5.8) |
| `-rmw=fastdds\|cyclonedds\|zenoh` | middleware |
| `-ros-domain-id=N` | DDS domain, 0..232 |
| `-game` | run the project as a game rather than opening the editor |
| `<MapName>` | positional: the map to boot |

Parsed in `Plugins/Carla/Source/Carla/Settings/CarlaSettings.cpp`.

## `-nullrhi` and cameras

```
[24]LogCarla: Spawning actor 'sensor.camera.rgb'
CommonUnixCrashHandler: Signal=11
Unhandled Exception: SIGSEGV: invalid attempt to read memory at address 0x58
libUnrealEditor-Carla.so!ImageUtil::ReadImageDataBegin(
    ImageUtil::ReadImageDataContext&, UTextureRenderTarget2D&,
    TSharedPtr<FRHIGPUReadbackPool, (ESPMode)1>, ...)
    [Plugins/Carla/Source/Carla/Sensor/ImageUtil.cpp:224]
  <- render thread, via ImageUtil::ReadImageDataAsync (ImageUtil.cpp:428)
```

`ImageUtil.cpp` and `RHIGPUReadbackPool.h` are UE5.8-era additions to the sensor
path, so this is not inherited UE4 behaviour. Non-image sensors, traffic, physics
and waypoint work are all fine under `-nullrhi`.

## Packaged vs game: measured

| Call | `game` | `package` |
|---|---|---|
| `get_server_version()` | 0.10.0 | 0.10.0 |
| `get_world()` | works, ~2 s | works, ~0 s |
| `get_map().get_spawn_points()` | 155 (Town10HD_Opt) | 155 |
| blueprint library | 168 | 168 |
| `try_spawn_actor(vehicle)` | works | works |
| `get_random_location_from_navigation()` | works | works |
| `get_available_maps()` | **29** | **0** |
| `load_world('Town10HD_Opt')` | works | works |
| `load_world('Town_C')` | works | works |
| `load_world('Town12')` | works | **`std::exception`** |
| `load_world('Town13')` | works | **`std::exception`** |
| `load_world('NoSuchTown')` | fails (correctly) | fails |

The 29 maps `game` mode reports include the Town15 sublevels
(`Town15_Buildings`, `Town15_Road`, …) and the map-generator templates, so the
list is discovery output rather than a list of playable towns.

Cause of the packaged failures: `UCarlaStatics::GetAllMapNames()` uses
`IFileManager::FindFilesRecursive(..., "*.umap")` and
`UCarlaEpisode::LoadNewEpisode()` gates on `FPaths::FileExists()` — neither can
see inside a `.pak`. AssetRegistry-based discovery
(`GetMapPackageNamesFromAssetRegistry()`, package-path `FindMapPath()`) exists on
`ue58-dev-windows` and has **not** been merged to `ue58-dev`.

An earlier Linux report claimed a client cannot reach a packaged server at all —
a `FileTransfer::WriteFile` path escape (`../../../` out of
`~/carlaCache/<version>/`) making every `get_world()` fail. That did **not**
reproduce here; `~/carlaCache/0.10.0/Carla/Maps` already existed, which is
consistent with the failure being `create_directories` on a missing tree. Treat
the escape as a latent defect (a server can still write outside the client cache)
rather than a blocker.

## ROS 2

Build with `-DENABLE_ROS2=ON`, run with `-ros2`. Both are required; either alone
gives silence.

Startup log on success:

```
LogCarla: ROS2: Creating ROS2 Instance...
LogCarla: ROS2: enabled with middleware 'fastdds'.
LogCarla: ROS2: Fast-DDS transport: UDPv4 only (default; set
          FASTDDS_BUILTIN_TRANSPORTS=DEFAULT to re-enable shared memory).
```

**UDPv4-only by default is a UE5.8 improvement.** On UE4 a local subscriber saw
nothing until you pointed `FASTRTPS_DEFAULT_PROFILES_FILE` at a profile disabling
shared-memory transport. Here the server does it for you — and an inherited
UE4-era profile file will *undo* it, so unset it.

### Flag spelling

UE 5.8's `FParse::Param` strips a leading dash from the search string, so
`FParse::Param(TEXT("-ros2"))` matches `-ros2`. On UE4 the same call matched
`--ros2`. `ue58-dev` additionally accepts `--ros2` via an explicit
`FCString::Strifind` fallback (`CarlaSettings.cpp:167-168`), added precisely
because of this trap. `-ros2` is safe on every ue58 commit.

### Enabling a sensor

`enable_for_ros` / `disable_for_ros` / `is_enabled_for_ros` live on **`Sensor`** in
UE 5.8, not on `Actor` as in 0.9.16. Confirmed at runtime:
`hasattr(sensor, 'enable_for_ros') == True`,
`hasattr(vehicle, 'enable_for_ros') == False`.

Without the call the sensor is never ticked — `ASensor::Tick` returns early unless
something is listening, and a ROS subscriber is not a stream client.

### Topic naming

`rt/carla/<parent chain>/<ros_name>` on the wire, which ROS clients see as
`/carla/...`. Suffixes `/image`, `/camera_info`, `/point_cloud`.

Observed with one camera named `front` on a vehicle named `hero`:

```
/carla/hero/front/camera_info
/carla/hero/front/image           ~5-6 Hz at sensor_tick=0.1, frame_id "front"
/carla/hero/vehicle_control_cmd   subscriber; created because role_name=hero
/clock                            ~640 Hz (async mode, no fixed delta)
/tf
```

The measured image rate is below the 10 Hz `sensor_tick` implies because
offscreen rendering is the limit, not the tick.

**No `/carla/map`.** `CarlaMapPublisher` was removed in the UE5 line, so the
latched OpenDRIVE topic present on UE4 does not exist. `"map"` remains only as the
root TF frame id for actors with no parent.

### What is new in the UE5.8 ROS 2 layer

`LibCarla/source/carla/ros2/` carries 136 files against UE4's 96, adding a
`listeners/` directory, `BasicPublisher`/`BasicListener` abstractions, and a
native Autoware integration: `AutowareGNSSPublisher`,
`AutowareVehicleStatusPublisher`, `AutowareControlSubscriber` and 16 Autoware
message types (`GearReport`, `VelocityReport`, `TurnIndicatorsCommand`, `Control`,
`Lateral`, `Longitudinal`, …).

ROS 2 is **Linux-only**: `Carla.Build.cs:186-193` hardcodes `Binaries/Linux` and
`.so` with no platform branch, for every middleware. `Docs/ros2_native.md:19`
claims Windows FastDDS support; it is wrong.

## Shutdown

Two known, cosmetic-but-annoying behaviours:

- the `-game` server takes **SIGSEGV on SIGTERM** (apport logs signal 11),
- the Python client segfaults at interpreter teardown *after* printing all its
  results (exit 139 with the work complete).

So **exit codes are not a pass/fail signal** for anything scripted here. Judge by
the port being released and by the results themselves.

### Killing a server

`/proc/<pid>/comm` is truncated to 15 characters:

| Real name | What `comm` shows |
|---|---|
| `CarlaUnreal-Linux-Shipping` | `CarlaUnreal-Lin` |
| `UnrealEditor` | `UnrealEditor` |

`pkill -x CarlaUnreal-Linux-Shipping` therefore matches nothing and exits as if it
had worked, leaving the port held; the next launch fails with
`bind: Address already in use` and then Signal 11. Use the truncated name, and
wait on the port rather than on the process:

```bash
pkill -x CarlaUnreal-Lin
until ! (echo >/dev/tcp/127.0.0.1/2000) 2>/dev/null; do sleep 1; done
```

### Detaching

A plain `&` is not enough — the server inherits the shell's stdio and dies with
`close: Bad file descriptor` then Signal 11 when it goes away:

```bash
setsid nohup <command> >/tmp/carla.log 2>&1 </dev/null &
```

## Timing

| Event | Typical |
|---|---|
| `game` mode to RPC port, warm build | ~20 s |
| `package` to RPC port | ~7 s |
| first launch after a rebuild | 30-60 min (shader compilation) |
| `get_world()` | 0-2 s |
| large map load (`game`) | tens of seconds |
