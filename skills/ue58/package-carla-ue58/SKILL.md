---
name: package-carla-ue58
description: Cooks and packages a shippable CARLA server from a UE 5.8 ue58-dev checkout via the CMake `package` targets (shipping/development/debug/test), scoping the cook with CARLA_MAPS_TO_COOK, and inspects an existing package for the failures a green build hides — stale pak/ucas mismatch, missing staged OpenDrive/TM road data, missing post-process profiles, and Town15's uncookable content. Use when the user asks to "package CARLA UE5", "cook a map", "build a shipping server", "make a distributable CARLA", or a package builds but renders black / has no maps.
license: MIT
compatibility: Linux. Needs a configured ue58-dev tree with the engine built, and ~150 GB free for cook + stage + archive on top of the build. VERIFIED end to end on ue58-dev HEAD 718efd7cc (engine 5.8, CARLA 0.10.0) - a 1-map Shipping package built from scratch in ~35 min producing 7.4 GB (6.6 GB .tar.gz), inspected with the engine's UnrealPak, and run. A 6-map cook is ~12 GB archived. Windows paths are documented from the code, not exercised.
metadata:
  group: ue58
  prerequisites: scripts/check_env.sh
  reference: references/packaging.md
---

# Package CARLA on UE 5.8

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

```bash
cmake --build Build/Release --target package          # = package-shipping
```

That is the whole command. Everything hard about packaging here is in the two
things around it:

- **The cook scope is a configure-time option, not a build argument.**
  `-DCARLA_MAPS_TO_COOK=` is baked into the CMake cache, so changing which maps
  ship means re-configuring — and re-specifying every other option at the same
  time.
- **`BUILD SUCCESSFUL` does not mean the package is usable.** Four separate
  defects produce a package that builds clean and then fails at runtime. All four
  are detectable after the fact, which is what this skill's `inspect` mode is for.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Decide the cook scope and re-configure for it
- [ ] Step 3: Package (1-2 h)
- [ ] Step 4: Inspect the artifact — do NOT trust the exit code
- [ ] Step 5: Run it (run-carla-ue58-server)
```

### Step 2: Cook scope

```bash
source scripts/env.sh

bash scripts/package.sh scope                       # what is configured right now
ROS2=1 MAPS="Town10HD_Opt,Town12,Town13,Town_C" \
    bash ../build-carla-ue58/scripts/build.sh configure
```

Package paths, **`+`-separated**. Small maps are `/Game/Carla/Maps/<Town>`; large
World Partition maps are `/Game/Carla/Maps/<Town>/<Town>`.

**Exclude Town15.** Its one-file-per-actor packages each carry a serialized
reference to a `MaterialInstanceDynamic` that lives in the map package — MIDs are
transient and never saved, so the import can never resolve. 573 unique
`LoadErrors`, and the cook fails on the error *count*:
`Failure - 573 error(s)` → `AutomationTool exiting with ExitCode=25`. It has never
cooked on Linux or Windows. This is a content defect, not a configuration mistake.

Leaving `CARLA_MAPS_TO_COOK` empty does **not** cook everything — `bCookAll=False`,
so UAT falls back to `DefaultGame.ini`'s five `MapsToCook` entries:
`Town10HD_Opt`, `OpenDriveMap`, `TestMaps/EmptyMap`, `Mine_01`, `Town15/Town15`.
Town15 is in that list, so **the out-of-the-box package build fails**. An explicit
scope is effectively mandatory. `check_env.sh` warns on both cases.

**Keep `OpenDriveMap` in the scope** unless you know you do not need it: it is the
host level for `client.generate_opendrive_world()`. Measured — with it excluded,
that call fails with a bare `std::exception` while the same call against `-game`
mode succeeds in 7.3 s.

### Step 3: Package

```bash
bash scripts/package.sh build                       # shipping (default)
CONFIG=development bash scripts/package.sh build    # package-development
```

or directly:

```bash
cmake --build Build/Release --target package             # shipping
cmake --build Build/Release --target package-development
cmake --build Build/Release --target package-debug
cmake --build Build/Release --target package-test
cmake --build Build/Release --target package-debuggame
```

Under the hood this is UAT `BuildCookRun` with `-pak -iostore`, so the output is
an IoStore container (`.ucas` + `.utoc`) plus a `.pak`, not a loose file tree.
Expect 1–2 h and ~12 GB.

**Delete `Build/<preset>/Package/` first if you care what ends up inside.** The
archive step copies over a previous archive *without deleting*
(`Unreal/Package/RemoveUnrealPackageExtraFiles.cmake` says so), so a package can
carry files from builds long past. Measured: a pre-existing package contained
`Carla/Maps/Town12/OpenDrive/Town12.xodr` that a clean build does not produce.

**Stop any running packaged server first.** It holds the pak files open; UAT then
fails to replace the `.ucas`, retries, and can still exit `BUILD SUCCESSFUL` with
a fresh `.pak`/`.utoc` beside a **stale `.ucas`** — internally inconsistent,
because the `.utoc` indexes the `.ucas` by byte offset. POSIX unlink semantics
make this less likely on Linux than on Windows, but the check costs nothing.

### Step 4: Inspect — the part that matters

```bash
bash scripts/package.sh inspect
```

Checks, on the artifact rather than the log:

| Check | Failure it catches |
|---|---|
| pak/ucas/utoc timestamps within one build | the stale-`.ucas` inconsistency above |
| `Town1x/OpenDrive` staged when that town is cooked | a large map with no road network |
| `Config/PostProcess` staged | **black screen**: missing per-town exposure profile |
| `OpenDriveMap` in the cook scope | `generate_opendrive_world()` failing with `std::exception` |
| shipping binary + launcher present and executable | a stage that dropped the binary |
| `VERSION` git hashes | which carla / content / engine commits actually went in |
| bundled wheel under `PythonAPI/carla/dist/` | a package whose client cannot be installed |

What `DirectoriesToAlwaysStageAsUFS` actually delivers, measured on a clean 1-map
build with the engine's own `UnrealPak -List` (80 UFS entries, 20 `.xodr`):

| Path | Ships? |
|---|---|
| `Carla/Maps/OpenDrive/*.xodr` | yes — all 19, regardless of cook scope |
| `Carla/Maps/TM/*.bin` | yes |
| `Carla/Maps/Town15/{OpenDrive,Nav,TM}` | yes (even though Town15 never cooks) |
| `Carla/Config/**` incl. `PostProcess` | yes — **`Carla/Config` IS recursive** |
| `Carla/Maps/Town1x/TM/*.bin` | yes |
| `Carla/Maps/Town1x/OpenDrive/*.xodr` | **NO** |

So the gap is exactly **two** entries, not five: `Carla/Maps/Town12/OpenDrive` and
`Carla/Maps/Town13/OpenDrive`. A cleanly built package that cooks Town12 or Town13
ships those towns without OpenDRIVE. (Why the `TM` subdir stages and `OpenDrive`
does not is not explained by the ini.)

### Step 5: Run it

```bash
cd ../run-carla-ue58-server && bash scripts/run_server.sh package
```

A packaged server works for clients — measured: `get_world()`, spawning and
navigation are all fine. What is broken is **map handling**:
`get_available_maps()` returns `[]`, and `load_world()` resolves only
`/Game/Carla/Maps/<Name>`, so a cooked map loads by exact name while nested large
maps (`Town12/Town12`) and imported maps under `/Game/<Pkg>/Maps/` do not. Use
`-game` mode when you need discovery or map switching. Detail and numbers in
[[run-carla-ue58-server]].

## Examples

**Example 1: "package CARLA with just Town10"**

`MAPS="Town10HD_Opt"` re-configure, `package.sh build`, `package.sh inspect`. The
fastest useful package; skips the large-map staging concerns entirely.

**Example 2: "my packaged Town_C renders black"**

`package.sh inspect`. If `Config/PostProcess` is missing from the pak, that is the
cause — `Default.json` uses `AEM_Manual` exposure where the per-town profiles use
`AEM_Histogram`, so without the profile the scene draws correct geometry at the
wrong exposure. Add the staging entry and re-package.

**Example 3: "the package built but has no maps"**

Check `scope` first: an empty `CARLA_MAPS_TO_COOK` cooks everything and fails on
Town15, and a `;`-separated or filesystem-path value cooks nothing while still
reporting success.

**Example 4: "which commits are in this package?"**

`package.sh inspect` prints the `VERSION` file: carla, content and engine git
hashes. Worth recording alongside any results you publish.

## Troubleshooting

**Problem: `Failure - 573 error(s), 78 warning(s)` / `ExitCode=25 (Error_UnknownCookFailure)`**
Cause: Town15 in the cook scope.
Solution: exclude it. Nothing you configure can fix it; the content is wrong.

**Problem: cook succeeds, package has no maps**
Cause: `CARLA_MAPS_TO_COOK` in the wrong form — filesystem paths, or `;` instead
of `+`.
Solution: `package.sh scope` shows the current value; the `MAPS=` knob on
`build.sh configure` builds a correct one.

**Problem: `BUILD SUCCESSFUL` but the server crashes on start**
Cause: possibly the stale-`.ucas` inconsistency from packaging over a running
server.
Solution: `package.sh inspect` compares timestamps; stop servers and re-package.

**Problem: packaged large map loads empty / black screen**
Cause: either the OFPA mount patch is missing (empty World Partition) or the
post-process profiles were not staged (wrong exposure). The two look similar; the
distinguishing symptom is that with the exposure problem the traffic lights and
signs still draw correctly.
Solution: `check_env.sh` reports the patch; `package.sh inspect` reports staging.

**Problem: the cook logs dozens of `Invalid duplicate copies of ExternalActor`**
Cause: expected side effect of the OFPA mount patch — every external package
becomes reachable under two names, and the AssetRegistry discards one
alphabetically. 74–78 warnings per cook.
Solution: none needed; it keeps the name the map expects. Worth removing properly
by scoping the mount, but it is not a failure.

**Problem: Zen server does not survive cook → stage**
Cause: known both platforms, different fixes. On Linux the route used is
`[Zen.AutoLaunch] LimitProcessLifetime=False`, which needs no manual server;
Windows uses `-NoZenAutoLaunch` and requires `zenserver.exe --port 8558` already
running.
Solution: the Linux setting; do not copy the Windows recipe.

**Problem: packaged map switching does not work / `get_available_maps()` is empty**
Cause: two raw-filesystem checks that cannot work inside a `.pak`
(`GetAllMapNames()` uses `FindFilesRecursive(..., "*.umap")`, `LoadNewEpisode()`
gates on `FPaths::FileExists()`). Fixed on `ue58-dev-windows` via AssetRegistry
discovery; **not merged into `ue58-dev`**.
Solution: no fix on this branch. Use the editor for multi-map work, or cherry-pick
that change.

## Outputs

`Build/<preset>/Package/Carla-<version>-Linux-<config>/` containing
`Linux/CarlaUnreal.sh` + the shipping binary, `Linux/CarlaUnreal/Content/Paks/`
(`pakchunk0-Linux.{pak,ucas,utoc}` + `global.{ucas,utoc}`), a bundled `PythonAPI/`
with its wheel, and `VERSION` with the three git hashes — plus a `.tar.gz` of the
same. `inspect` is read-only.

Cook/stage mechanics, the staging list, and the full defect register are in
[references/packaging.md](references/packaging.md).
