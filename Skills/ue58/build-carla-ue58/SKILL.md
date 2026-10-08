---
name: build-carla-ue58
description: Builds CARLA on Unreal Engine 5.8 from the ue58-dev branch using its CMake build system — configure a preset, then build the carla-unreal / carla-unreal-editor / carla-python-api-install / launch targets — with ROS 2, DLSS, RSS and cook-scope options, and the engine-side build of the CarlaUnreal UE 5.8 fork. There is no Makefile and no Util/BuildTools in this tree, so every UE4 `make` recipe is invalid here. Use when the user asks to "build CARLA UE5", "build ue58-dev", "compile CARLA with cmake", "rebuild the Python API", or hits a CMake configure or target failure.
license: MIT
compatibility: Linux (Ubuntu 22.04/24.04). Needs CMake >= 3.28, ninja, git-lfs, Python 3.8-3.12, the CarlaUnreal/UnrealEngine fork on branch ue58-dev-carla, and ~400 GB free on a LOCAL disk. Verified against ue58-dev HEAD 718efd7cc, engine 5.8, CARLA 0.10.0.
metadata:
  group: ue58
  prerequisites: scripts/check_env.sh
  reference: references/cmake.md
---

# Build CARLA on UE 5.8

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

**This tree has no `Makefile` and no `Util/BuildTools/`.** Every UE4 recipe —
`make CarlaUE4`, `make PythonAPI`, `make launch`, `make package`, `make import` —
does not exist. The whole build is CMake:

```bash
cmake --preset Release                              # configure (once per option change)
cmake --build Build/Release --target <target>       # build
```

Two consequences that cause most of the confusion:

- **Options live in the CMake cache, not on the build line.** `-DENABLE_ROS2=ON`
  is passed to `--preset`, and **must be repeated on every re-configure**. Forget
  it and you get a working build that silently has no ROS 2 in it.
- **The engine is a separate, prior build.** `CARLA_UNREAL_ENGINE_PATH` must point
  at the CarlaUnreal fork on branch **`ue58-dev-carla`** (UE 5.8). The `ue5-dev`
  branch uses a *different* engine fork (`ue5-dev-carla`, UE 5.5) — see the
  [[build-carla-ue5]] skill for that one.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear every FAIL
- [ ] Step 2: Build the engine, if UnrealEditor is not there yet
- [ ] Step 3: Configure a preset with the options you actually want
- [ ] Step 4: Build the targets, in order
- [ ] Step 5: Verify the artifacts (not the exit codes)
```

### Step 1: Preflight

```bash
source scripts/env.sh
bash scripts/check_env.sh
```

This reads the CMake cache and tells you what the tree was *actually* configured
with — the only honest answer to "was this built with ROS 2?" — plus the engine
version, the OFPA large-map patch state, and whether a past `sudo` poisoned
`~/.nuget` / `~/.epic`.

### Step 2: The engine

```bash
git clone -b ue58-dev-carla https://github.com/CarlaUnreal/UnrealEngine.git UnrealEngine5_carla
cd UnrealEngine5_carla
./Setup.sh && ./GenerateProjectFiles.sh && make
export CARLA_UNREAL_ENGINE_PATH=$PWD
```

**`make` takes no `-j`.** Parallel jobs kill each other on the UnrealBuildTool
mutex (`Failed (ConflictingInstance)`); plain `make` is already parallel. Budget
2–3 h. The clone needs a GitHub account linked to Epic Games, or it 404s.

**`make` can exit 0 with targets dead.** Check the binary, never the exit code:

```bash
ls -l $CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Linux/UnrealEditor
```

### Step 3: Configure

```bash
source scripts/env.sh

bash scripts/build.sh configure                       # Release, defaults
ROS2=1 bash scripts/build.sh configure                # + ROS 2 native
ROS2=1 MAPS="Town10HD_Opt,Town12,Town13" bash scripts/build.sh configure
```

or by hand:

```bash
cd "$CARLA_UE58_ROOT"
cmake --preset Release -DENABLE_ROS2=ON \
  -DCARLA_MAPS_TO_COOK="/Game/Carla/Maps/Town10HD_Opt+/Game/Carla/Maps/Town12/Town12"
```

Presets are `Release`, `Development`, `Debug` (plus the `Common` base). The
options worth knowing:

| Option | Effect |
|---|---|
| `-DENABLE_ROS2=ON` | native ROS 2; builds `carla-ros2-native` and links it into the plugin |
| `-DCARLA_MAPS_TO_COOK=` | restrict the cook. **`+`-separated package paths**, not filesystem paths, not `;` |
| `-DCARLA_UNREAL_ENGINE_PATH=` | engine fork path (else the env var) |
| `-DCARLA_UNREAL_RHI=vulkan` | RHI for the editor/game |
| `-DCARLA_DLSS_SDK_PATH=` | DLSS SDK; `disabled` builds without it |
| `-DENABLE_RSS=ON`, `-DENABLE_OSM2ODR=ON`, `-DENABLE_PYTORCH=ON` | all default **OFF** |
| `-DCARLA_UNREAL_PACKAGE_BUILD_TYPE=Shipping` | package configuration |
| `-DCMAKE_DISABLE_FIND_PACKAGE_Qt5=ON -DCMAKE_DISABLE_FIND_PACKAGE_Qt6=ON` | when system Qt is installed but not linkable with the UE toolchain |

`bash scripts/build.sh options` prints the full list with defaults, read from the
tree rather than from this table.

### Step 4: Build

```bash
bash scripts/build.sh libcarla        # carla-server + carla-client
bash scripts/build.sh pythonapi       # carla-python-api-install (builds + installs the wheel)
bash scripts/build.sh unreal          # carla-unreal        (the game/server target)
bash scripts/build.sh editor          # carla-unreal-editor
bash scripts/build.sh launch          # build + open the editor
bash scripts/build.sh targets         # every available target, from the tree
```

or directly:

```bash
cmake --build Build/Release                                  # default targets
cmake --build Build/Release --target carla-unreal
cmake --build Build/Release --target carla-python-api-install
cmake --build Build/Release --target launch
```

Order matters on a cold tree: LibCarla → Python API → `carla-unreal` → editor.
`carla-python-api-install` both builds the wheel and pip-installs it, so it is the
one to use rather than `carla-python-api`.

First `carla-unreal` build is 30–60 min; the first editor launch spends another
30–60 min compiling shaders with no progress output that means anything.

### Step 5: Verify artifacts, not exit codes

```bash
bash scripts/build.sh verify
```

Checks the things an exit code will not: the editor binary, the plugin `.so`, the
Python wheel and whether `carla` imports, and — when ROS 2 was configured —
whether `libcarla-ros2-native.so` exists and the plugin actually carries
`carla::ros2` symbols. A CMake target can report success while the artifact it
was supposed to produce is stale or absent.

## Examples

**Example 1: "build CARLA UE5 with ROS 2"**

`check_env.sh`, then `ROS2=1 bash scripts/build.sh configure`, then `libcarla`,
`pythonapi`, `unreal`, `verify`. Confirm with `verify` that `carla::ros2` symbols
are present — that is what distinguishes a real ROS 2 build from a forgotten
`-DENABLE_ROS2=ON`.

**Example 2: "I rebuilt and ROS 2 stopped working"**

Almost certainly a re-configure without the flag. `check_env.sh` prints
`ENABLE_ROS2` straight from the cache; if it says `OFF`, re-configure with the
flag and rebuild `carla-unreal`.

**Example 3: "just rebuild the Python API after a client change"**

`bash scripts/build.sh pythonapi`. No need to touch the engine or the editor.

**Example 4: "open the editor"**

`bash scripts/build.sh launch` (builds first) or `launch-only` (does not).
Running a server instead is [[run-carla-ue58-server]].

## Troubleshooting

**Problem: `make: *** No rule to make target 'CarlaUE4'` / `Makefile not found`**
Cause: a UE4 recipe on a UE5 tree.
Solution: use the CMake targets above. `bash scripts/build.sh targets` lists them.

**Problem: `Failed (ConflictingInstance)` during the engine build**
Cause: `make -j` on the engine — the jobs fight over the UnrealBuildTool mutex.
Solution: plain `make`.

**Problem: the build "succeeded" but nothing changed**
Cause: a target can report success while its artifact is stale, and the engine
`make` can exit 0 with dead targets.
Solution: `bash scripts/build.sh verify`; check binary timestamps.

**Problem: ROS 2 was configured but no topics appear**
Cause: either `ENABLE_ROS2` is not actually in the cache, or the server was
started without the runtime flag. Both are needed.
Solution: `check_env.sh` for the first; `-ros2` (or `--ros2`, both accepted on
current ue58-dev) for the second — see [[run-carla-ue58-server]].

**Problem: `undefined reference to 'png_set_text@PNG16_0'` while linking the Qt example**
Cause: system Qt5 is built against libstdc++ and links system libpng/glib/ICU;
CARLA links it under UE's `-stdlib=libc++` inside UE's sysroot, which never
searches those paths. The guard checks whether Qt is *installed*, not *linkable*.
Solution: re-configure with
`-DCMAKE_DISABLE_FIND_PACKAGE_Qt5=ON -DCMAKE_DISABLE_FIND_PACKAGE_Qt6=ON`.

**Problem: every build fails after one `sudo` run**
Cause: a sudo'd UE or CARLA script left `~/.nuget`, `~/.epic`, `~/.lldbinit`
root-owned.
Solution: `sudo chown -R $USER:$USER ~/.nuget ~/.epic ~/.lldbinit`. `check_env.sh`
detects this.

**Problem: `CMake 3.28 or higher is required`**
Cause: Ubuntu 22.04 ships 3.22.
Solution: install 3.28+ (e.g. into `/opt`) and put it first on `PATH`.

**Problem: Town12/Town13 open with an empty world and a black screen**
Cause: the OFPA mount patch is not applied — a large map's one-file-per-actor
packages live under `Content/Carla/__ExternalActors__` / `__ExternalObjects__`,
which is not a registered mount point.
Solution: apply the patch adding `FCarlaModule::MountExternalPackageRoots()` and
rebuild `carla-unreal`; verify with
`grep -a "Mounted external package root" Unreal/CarlaUnreal/Saved/Logs/CarlaUnreal.log`.
`check_env.sh` reports whether the patch is present.

**Problem: `CarlaSetup.sh` cloned the wrong branches**
Cause: it targets `ue5-dev` / `ue5-dev-carla`, i.e. UE 5.5.
Solution: do not run it for ue58-dev. Clone the repos explicitly as in Step 2.

**Problem: the build is glacial or fails oddly on a mounted volume**
Cause: network or external storage.
Solution: build on a local disk. `check_env.sh` names the backing device.

## Outputs

`Build/<preset>/` with LibCarla libraries, the Python wheel under
`Build/<preset>/PythonAPI/dist/`, the plugin binaries under
`Unreal/CarlaUnreal/Plugins/Carla/Binaries/Linux/`, and the editor buildable and
launchable. Packaging is [[package-carla-ue58]]; running a server is
[[run-carla-ue58-server]].

Target and option reference, the UE4→UE5 command mapping, and time budgets are in
[references/cmake.md](references/cmake.md).
