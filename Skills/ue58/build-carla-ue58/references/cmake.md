# The UE 5.8 CMake build

Read off `ue58-dev` HEAD `718efd7cc` and its generated `Build/Release/Help.md`.
Regenerate the authoritative list at any time with `bash scripts/build.sh targets`
and `bash scripts/build.sh options` — those read the tree, not this file.

## UE4 → UE5 command mapping

| UE4 (`Makefile`) | UE 5.8 (CMake) |
|---|---|
| `make setup` | `cmake --preset Release` (dependencies are FetchContent) |
| `make LibCarla` | `cmake --build Build/Release --target carla-server carla-client` |
| `make PythonAPI` | `--target carla-python-api` |
| *(install the wheel)* | `--target carla-python-api-install` |
| `make CarlaUE4` | `--target carla-unreal` |
| *(editor target)* | `--target carla-unreal-editor` |
| `make launch` | `--target launch` |
| `make launch-only` | `--target launch-only` |
| `make package` | `--target package` (or `package-shipping`, `package-debug`, …) |
| `make check.LibCarla` | `--target libcarla_test_server libcarla_test_client` |
| `make import` | `Util/Tools/Import.py` / `Util/ImportAssets.sh` — no target |
| `make clean` | `--target clean` |
| `ARGS="--map X"` on package | `-DCARLA_MAPS_TO_COOK="/Game/Carla/Maps/X"` at configure |
| `Config/OptionalModules.ini` `Ros2 ON` | `-DENABLE_ROS2=ON` at configure |
| `UE4_ROOT` | `CARLA_UNREAL_ENGINE_PATH` |
| `Unreal/CarlaUE4/` | `Unreal/CarlaUnreal/` |
| `Dist/CARLA_*/LinuxNoEditor/CarlaUE4.sh` | `Build/<preset>/Package/Carla-<ver>-Linux-<cfg>/Linux/CarlaUnreal.sh` |

## Presets

`CMakePresets.json` defines `Common` (base, not selectable in practice),
`Development`, `Release`, `Debug`. Each configures into `Build/<name>/`.

```bash
cmake --preset Release
cmake --build Build/Release --target <target>
```

The same names exist on `ue5-dev`. `Docs/build_linux_ue5.md` mentions
`Linux-Release` / `Linux-Development`; those preset names are **not** in the
current `CMakePresets.json` on either branch — the doc is stale.

## Targets

**Libraries and tests**
`carla-server`, `carla-client`, `libcarla_test_server`, `libcarla_test_client`

**Python API**
`carla-python-api` (build only), `carla-python-api-install` (build + pip install)

**ROS 2**
`carla-ros2-native` — builds the middleware subproject. Only meaningful with
`-DENABLE_ROS2=ON`.

**Unreal**
`carla-unreal` (game/server), `carla-unreal-editor`, `launch` (build + open the
editor), `launch-only` (open without building), `check-unreal-content` (asserts
the assets are in place before the editor tries to open without them)

**Packaging** — each has a `carla-unreal-package-*` alias
`package` (= shipping), `package-shipping`, `package-debug`, `package-debuggame`,
`package-development`, `package-test`

**Examples**
`carla-example-client`, `carla-agent-demo`

**Meta**
`clean`, `carla-help` (prints `Help.md`)

## Options

Defaults from a configured tree. **All of these are cache variables: repeat them
on every re-configure or they revert to the default.**

| Option | Default | Notes |
|---|---|---|
| `BUILD_CARLA_CLIENT` / `BUILD_CARLA_SERVER` | ON | |
| `BUILD_PYTHON_API` | ON | |
| `BUILD_EXAMPLES` | ON | includes the Qt client, which often will not link — see below |
| `BUILD_LIBCARLA_TESTS` | ON | |
| `BUILD_CARLA_UNREAL` | ON | set OFF for a client-only build |
| `ENABLE_ROS2` | **OFF** | native ROS 2 |
| `ENABLE_ROS2_DEMO` | OFF | |
| `ENABLE_RSS` | **OFF** | ad-rss-lib; why `sensor.other.rss` and the `Rss*` classes are absent |
| `ENABLE_OSM2ODR` | **OFF** | |
| `ENABLE_PYTORCH` | **OFF** | |
| `ENABLE_PEP517` | ON | wheel build path |
| `ENABLE_STREETMAP` | ON | fetches the StreetMap plugin (`ue5-dev-carla` tag) |
| `CARLA_UNREAL_ENGINE_PATH` | *(env)* | the engine fork |
| `CARLA_UNREAL_RHI` | `vulkan` | |
| `CARLA_UNREAL_BUILD_TYPE` | `Development` | editor configuration |
| `CARLA_UNREAL_PACKAGE_BUILD_TYPE` | `Shipping` | package configuration |
| `CARLA_UNREAL_PACKAGE_NO_COMPRESSION` | OFF | |
| `CARLA_UNREAL_LOG_WINDOW` | ON | terminal alongside the editor |
| `CARLA_LAUNCH_ARGS` | `-log` | semicolon-separated, passed to the editor by `launch` |
| `CARLA_MAPS_TO_COOK` | *(empty = all)* | **`+`-separated package paths** |
| `CARLA_DLSS_SDK_PATH` | `$DLSS_SDK` → `~/SDKs/DLSS` | `disabled` builds without DLSS |
| `LIBCARLA_IMAGE_SUPPORTED_FORMATS` | `"png"` | `png;jpeg;tiff` |
| `PREFER_CLONE` | OFF | clone dependencies instead of downloading archives |
| `CARLA_UNREAL_CONTENT_PREFER_CLONE` | OFF | "extremely discouraged" per the option's own doc |
| `VERBOSE_CONFIGURE` | OFF | |
| `ENABLE_ALL_WARNINGS` / `ENABLE_WARNINGS_TO_ERRORS` | ON / OFF | |

Dependency pins are also cache variables (`CARLA_BOOST_VERSION` 1.90.0,
`CARLA_PROJ_VERSION` 9.7.0, `CARLA_SQLITE_VERSION` 3.50.04.00,
`CARLA_XERCESC_VERSION` 3.3.0, `CARLA_FASTDDS_VERSION` 2.14.6,
`CARLA_CYCLONEDDS_VERSION` 0.10.5, `CARLA_ZENOH_C_VERSION` 1.8.0, …) — useful to
know they exist, rarely worth changing.

### `CARLA_MAPS_TO_COOK` in detail

Package paths, **`+`-separated**, because the value is passed straight through to
UAT's `-MapsToCook=`. Filesystem paths and `;` separators both fail silently by
cooking nothing.

```
-DCARLA_MAPS_TO_COOK="/Game/Carla/Maps/Town10HD_Opt+/Game/Carla/Maps/Town12/Town12+/Game/Carla/Maps/Town13/Town13+/Game/Carla/Maps/Town_C"
```

Note the shape difference: small maps are `/Game/Carla/Maps/<Town>`, large
(World Partition) maps are `/Game/Carla/Maps/<Town>/<Town>`.
`scripts/build.sh configure` with `MAPS="Town10HD_Opt,Town12"` expands this for
you and warns about Town15.

## Repositories and branches

| Component | Repository | Branch |
|---|---|---|
| Unreal Engine fork | `github.com/CarlaUnreal/UnrealEngine` | `ue58-dev-carla` |
| CARLA | `github.com/carla-simulator/carla` | `ue58-dev` |
| Content | `bitbucket.org/carla-simulator/carla-content` | `ue58-dev-carla` |
| StreetMap plugin | `github.com/carla-simulator/StreetMap` | `ue5-dev-carla` (fetched by CMake) |
| DLSS SDK (optional) | `github.com/NVIDIA/DLSS` | default |

Content goes to `Unreal/CarlaUnreal/Content/Carla`.

**Do not run `CarlaSetup.sh`** for this branch: it clones `ue5-dev` and
`ue5-dev-carla`, i.e. UE 5.5.

The engine clone requires a GitHub account linked to Epic Games; without it the
repository 404s.

## Engine build

```bash
cd UnrealEngine5_carla
./Setup.sh && ./GenerateProjectFiles.sh && make
```

- **No `-j`.** Parallel jobs collide on the UnrealBuildTool mutex and die with
  `Failed (ConflictingInstance)`. Plain `make` is already parallel internally.
- **`make` can exit 0 with targets dead.** Verify
  `Engine/Binaries/Linux/UnrealEditor` exists.
- Piping build output through `tee` needs `set -o pipefail`, or a failure reports
  success.
- **Never run any UE or CARLA script under `sudo`.** It leaves `~/.nuget`,
  `~/.epic` and `~/.lldbinit` root-owned and every later build fails without
  naming the cause. Recover with
  `sudo chown -R $USER:$USER ~/.nuget ~/.epic ~/.lldbinit`.

## Known build-environment defects

**Qt example client cannot link.** System Qt5 is built against libstdc++ and links
system libpng/glib/ICU; CARLA links it with UE's `ld` under `-stdlib=libc++`
inside UE's sysroot, which never searches those paths →
`undefined reference to 'png_set_text@PNG16_0'`.
`Examples/QtClient/CMakeLists.txt:29-35` guards on Qt being *installed*, not
*linkable*. Workaround:
`-DCMAKE_DISABLE_FIND_PACKAGE_Qt5=ON -DCMAKE_DISABLE_FIND_PACKAGE_Qt6=ON`,
repeated on every re-configure.

**DLSS resolves only through `DLSS_SDK`.** `DLSSRRNGX.cpp:118-128` looks next to
the binary and under `$DLSS_SDK/lib/<Platform>/{rel,dev}`; nothing is ever staged
next to a binary, so only the environment variable ever resolves it — editor and
package alike. A package shipped elsewhere degrades silently to the NFOR denoiser,
and shipping builds emit no `LogDLSSRR`, so the recipient gets no signal.

**Large maps need the OFPA mount patch.** A large map's one-file-per-actor
packages live under `Content/Carla/__ExternalActors__` /
`__ExternalObjects__`, which is not a registered mount point, so no actor package
resolves — empty World Partition, black screen. The fix adds
`FCarlaModule::MountExternalPackageRoots()` (called from `StartupModule()`) which
scans `Content/*/__External{Actors,Objects}__/*` and registers each via
`FPackageName::RegisterMountPoint`. Verify after launch:

```bash
grep -a "Mounted external package root" Unreal/CarlaUnreal/Saved/Logs/CarlaUnreal.log
```

Caveat: every external package then resolves under two names (via the registered
mount and via the plain `/Game/` → `Content/` mount), so the cook logs 74–78
`Invalid duplicate copies of ExternalActor` warnings and picks alphabetically. It
happens to keep the name the map expects, so runtime is correct.

## Time budget, cold start

| Step | Duration |
|---|---|
| Engine clone + `Setup.sh` | 30–60 min |
| Engine `make` | 2–3 h |
| Content clone | 20–60 min |
| CARLA configure + LibCarla + Python API | 20–40 min |
| First `carla-unreal` | 30–60 min |
| First editor launch (shader compile) | 30–60 min |
| `package` | 1–2 h |

~400 GB free on a **local** disk. Never build on network or external storage.

## Verifying, and why exit codes are not enough

Three separate failure modes make a green build meaningless here:

1. the engine `make` can exit 0 with targets dead,
2. a relink can succeed against cached objects, producing a plugin with zero
   `carla::ros2` symbols even though `ENABLE_ROS2=ON` was configured,
3. UAT can report `BUILD SUCCESSFUL` over an internally inconsistent package.

`bash scripts/build.sh verify` checks the artifacts instead: editor binary,
LibCarla archives, wheel and importability, plugin `.so` count, and the
`carla::ros2` symbol count when ROS 2 was configured (218 on a good build of this
tree). It uses `nm -DC ... | grep -c`, never `grep -q`: `-q` exits on the first
match, `nm` dies of SIGPIPE, and `set -o pipefail` turns that into a false
failure.
