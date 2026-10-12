# Packaging on UE 5.8

Verified against the `Carla-0.10.0-Linux-Shipping` package built from
`ue58-dev` `b264b583a` (engine `3f57cff9a`, content `7ba5c7525`).

## The targets

```bash
cmake --build Build/<preset> --target package              # = package-shipping
cmake --build Build/<preset> --target package-shipping
cmake --build Build/<preset> --target package-development
cmake --build Build/<preset> --target package-debug
cmake --build Build/<preset> --target package-debuggame
cmake --build Build/<preset> --target package-test
```

Each has a `carla-unreal-package[-config]` alias. Under the hood: UAT
`BuildCookRun` (`Engine/Build/BatchFiles/RunUAT.sh`) with `-pak -iostore` — the
`-pak -iostore` pair landed upstream and is in `Unreal/CMakeLists.txt:445-446`.

Relevant configure-time options:

| Option | Effect |
|---|---|
| `CARLA_UNREAL_PACKAGE_BUILD_TYPE` | default `Shipping`; what plain `package` produces |
| `CARLA_MAPS_TO_COOK` | cook scope, `+`-separated package paths |
| `CARLA_UNREAL_PACKAGE_NO_COMPRESSION` | skip the compression step |

## Output layout

```
Build/<preset>/Package/
├── Carla-0.10.0-Linux-Shipping/
│   ├── VERSION                       carla / content / engine git hashes
│   ├── CHANGELOG  LICENSE  README
│   ├── Linux/
│   │   ├── CarlaUnreal.sh            2-line shim -> the binary below
│   │   ├── CarlaUnreal/
│   │   │   ├── Binaries/Linux/CarlaUnreal-Linux-Shipping   (192 MB)
│   │   │   └── Content/Paks/
│   │   │       ├── pakchunk0-Linux.pak    237 MB   loose/UFS entries
│   │   │       ├── pakchunk0-Linux.ucas   9.6 GB   IoStore container
│   │   │       ├── pakchunk0-Linux.utoc   6.8 MB   index into the .ucas
│   │   │       ├── global.ucas           3.2 MB
│   │   │       └── global.utoc           4 KB
│   │   └── Engine/
│   ├── PythonAPI/
│   │   ├── carla/dist/carla-0.10.0-cp310-cp310-linux_x86_64.whl
│   │   ├── examples/                 25 scripts
│   │   └── util/
│   └── Tools/
├── Carla-0.10.0-Linux-Shipping.tar.gz   11 GB
└── StagedBuilds/
```

Total ~12 GB for a 4-map cook (Town10HD_Opt, Town12, Town13, Town_C + OpenDriveMap
+ EmptyMap).

`VERSION` is worth recording with any published result:

```
Carla git hash:         b264b583aaae75e3af47b4f3fc9191ec3312f892
Content git hash:       7ba5c75256e6528e3e32511d7d83af6d742d2988
UnrealEngine git hash:  3f57cff9a439012526b60c7f39d6b7bbcafbab83
```

## Cook scope

`-DCARLA_MAPS_TO_COOK` is passed straight through as UAT's `-MapsToCook=`, so it
wants **`+`-separated package paths**. Two shapes:

```
/Game/Carla/Maps/Town10HD_Opt          small maps
/Game/Carla/Maps/Town12/Town12         large (World Partition) maps: doubled
```

A `;` separator or a filesystem path cooks nothing while still reporting success.
Leaving it empty cooks everything — including Town15, which fails the build.

## Defect register

### Town15 cannot be cooked — content defect, both platforms

Every Town15 one-file-per-actor package carries a serialized reference to a
`MaterialInstanceDynamic` that lives in the map package:

```
LoadErrors: Error: /Game/__ExternalActors__/Carla/Maps/Town15/Town15/A/DQ/6DLDVM9MVXZZFFBT5YOSX2 :
  Failed import for MaterialInstanceDynamic
  /Game/Carla/Maps/Town15/Town15.Town15:MaterialInstanceDynamic_1250
```

MIDs are transient runtime objects and are never saved, so the import can never
resolve. 583 error lines, 573 unique, all Town15 — the other maps produce none. The
cook fails on the error count:

```
Failure - 573 error(s), 78 warning(s)
AutomationTool exiting with ExitCode=25 (Error_UnknownCookFailure)
```

The Windows pak listing shows `Town15 0` entries "correctly excluded", so it has
never cooked on either platform. Exclude it until the content is fixed.

### Default cook scope, and why the default build fails

`CARLA_MAPS_TO_COOK` empty means no `-MapsToCook=`, and UAT falls back to
`DefaultGame.ini` where `bCookAll=False` and `bCookMapsOnly=False`:

```
+MapsToCook=(FilePath="/Game/Carla/Maps/Town10HD_Opt")
+MapsToCook=(FilePath="/Game/Carla/Maps/OpenDriveMap")
+MapsToCook=(FilePath="/Game/Carla/Maps/TestMaps/EmptyMap")
+MapsToCook=(FilePath="/Game/Carla/Maps/Mine_01")
+MapsToCook=(FilePath="/Game/Carla/Maps/Town15/Town15")
```

Five maps, not all of them — and Town15 is one, so the out-of-the-box package
build fails. The boot map is separate (`DefaultEngine.ini`:
`GameDefaultMap`/`ServerDefaultMap` = `Town10HD_Opt`), which is why a package with
no map argument starts on Town10HD_Opt.

`OpenDriveMap` in that default list is not decoration: it is the host level for
`client.generate_opendrive_world()`. Measured — excluded from the cook scope, that
call fails with a bare `std::exception`; against `-game` on the same tree it
succeeds in 7.3 s and the resulting world reports its map name as
`Carla/Maps/OpenDriveMap`.

### Cook scope controls loadable maps — measured

Same tree, same commit, only `CARLA_MAPS_TO_COOK` changed, both packages built
clean:

| | 6-map scope | 1-map scope |
|---|---|---|
| package / archive | 33 GB / 12 GB | **7.4 GB / 6.6 GB** |
| `pakchunk0-Linux.ucas` | 9.6 GB | **5.7 GB** |
| cooked top-level `.umap` | Town10HD_Opt, Town_C, OpenDriveMap | **Town10HD_Opt only** |
| Town12 / Town13 WP cells | 2,068 / 6,194 | **none** |
| `load_world('Town_C')` | OK | **fails** |
| `generate_opendrive_world()` | (not tested) | **fails** |

### The archive directory is cumulative

`Unreal/Package/RemoveUnrealPackageExtraFiles.cmake` states it: "The archive step
copies over a previous archive without deleting". Measured — a pre-existing
package contained `Carla/Maps/Town12/OpenDrive/Town12.xodr` which a clean build
does not stage at all. **Delete `Build/<preset>/Package/` before a build whose
contents you intend to reason about**, or you inspect a union of every build ever
run in that tree. This is also why the same script removes stale staging manifests
and a leftover `ue.projectstore` (which would make the packaged binary try to
stream content over the network).

### Staging is an explicit list, and it has gaps

`DirectoriesToAlwaysStageAsUFS` decides what non-asset data ships. Two facts make
this bite:

1. large maps keep their road data in `Carla/Maps/<Town>/{OpenDrive,TM}`,
2. **`Carla/Config` is not staged recursively** — the list carries
   `Carla/Config/Mine_01` separately, which proves it.

Measured on a clean 1-map build with `UnrealPak -List` (80 UFS entries under
`Content/`, 20 of them `.xodr`):

| Path | Ships on a clean build? |
|---|---|
| `Carla/Maps/OpenDrive/*.xodr` | yes — all 19, whatever the cook scope |
| `Carla/Maps/TM/*.bin` | yes |
| `Carla/Maps/Town15/{OpenDrive,Nav,TM}` | yes (Town15 never cooks, its loose data ships anyway) |
| `Carla/Config/**` including `PostProcess` | yes — **`Carla/Config` IS recursive** |
| `Carla/Maps/Town1x/TM/*.bin` | yes |
| `Carla/Maps/Town1x/OpenDrive/*.xodr` | **NO** |

So **two** entries are missing, not five: `Carla/Maps/Town12/OpenDrive` and
`Carla/Maps/Town13/OpenDrive`. The TM subdirs and the post-process profiles
already ship. Why `TM` stages and `OpenDrive` does not is not explained by the
ini, and worth reporting upstream as such.

Missing road data ships a map with no usable network. Missing
`Config/PostProcess` would be worse and less obvious (it does ship on a clean
build, so this is the failure to watch for rather than one to expect): `Default.json` uses `AEM_Manual`
exposure where the per-town profiles (`Town10HD_Opt.json`, `Town_C.json`) use
`AEM_Histogram`, so the map **renders black** with a correct scene behind it. The
tell that distinguishes it from the OFPA problem is that traffic lights and signs
still draw correctly.

Upstream this is Issue 13, fixed on `ue58-dev-windows` for Town12 only and **not
merged to `ue58-dev`**; on Linux the two `OpenDrive` entries are what is missing.

`package.sh inspect` checks all five by grepping the pak index — the index stores
paths as plain strings, so it needs no UnrealPak.

### Packaging over a running server can corrupt the archive

The server holds the pak files open; UAT fails to copy the `.ucas`, retries, and
can still exit `BUILD SUCCESSFUL` leaving a fresh `.pak`/`.utoc` beside a **stale
`.ucas`**. Since the `.utoc` indexes the `.ucas` by byte offset, the result is
internally inconsistent. Observed on Windows with `.pak`/`.utoc` at 19:08 next to
a `.ucas` from 16:14. POSIX unlink semantics make it unlikely on Linux, but
`package.sh build` refuses to start with a server running and
`package.sh inspect` compares timestamps (18 s spread on a good build).

### Packaged map discovery does not work

`get_available_maps()` returns `[]` and `load_world()` fails for large maps —
`GetAllMapNames()` uses `FindFilesRecursive(..., "*.umap")` and `LoadNewEpisode()`
gates on `FPaths::FileExists()`, neither of which sees inside a `.pak`.
AssetRegistry discovery exists on `ue58-dev-windows`, not merged. Small maps still
load by exact name. Detail and measurements in [[run-carla-ue58-server]].

### OFPA duplicate-name warnings

With the large-maps mount patch applied, every external package is reachable both
via the registered mount (`/Game/__ExternalActors__/Carla/...`) and via the plain
`/Game/` → `Content/` mount, so the cook logs 74-78

```
LogAssetRegistry: Warning: Invalid duplicate copies of ExternalActor ...
  Discarding: /Game/Carla/__ExternalObjects__/...  Keeping: /Game/__ExternalObjects__/...
```

and resolves alphabetically. It happens to keep the name the map expects, so
runtime is correct. Not a failure; worth fixing by scoping the mount.

### Zen server across cook → stage

Hit on both platforms with different fixes. Linux uses
`[Zen.AutoLaunch] LimitProcessLifetime=False`, which needs no manual server.
Windows uses `-NoZenAutoLaunch=127.0.0.1` and requires `zenserver.exe --port 8558`
already running — `-NoZenAutoLaunch` points UAT at an external server, it does not
start one. Do not copy the Windows recipe onto Linux.

### DLSS is not staged

`DLSSRRNGX.cpp:118-128` resolves the runtime next to the binary or under
`$DLSS_SDK/lib/<Platform>/{rel,dev}`. Nothing is ever copied next to a packaged
binary, so only the environment variable resolves it. A package shipped elsewhere
degrades silently to the NFOR denoiser / spatial upscale, and shipping builds emit
no `LogDLSSRR`, so the recipient gets no signal. `SetupDLSS.sh` notes the SDK is
deliberately not vendored for NVIDIA licensing reasons, so whether the snippets may
be redistributed inside a built application is a licensing question first.

## Fix status against `ue58-dev`

| Fix | Windows | On `ue58-dev` |
|---|---|---|
| `-pak -iostore` | needed | **yes**, upstream |
| AssetRegistry map discovery (Issue 1) | needed | **no** |
| `-culturesToStage=en` ICU (Issue 2) | needed | **no** — never bit on Linux |
| Zen survives cook→stage (Issue 7) | `-NoZenAutoLaunch` | different route |
| Town12 `DirectoriesToAlwaysStageAsUFS` (Issue 13) | needed | **no** — and it needs to cover Town13 + `Config/PostProcess` too |
| Cook-scope exclusion | — | Linux-side, not upstream |

## Time and space

| Stage | Cost |
|---|---|
| Shipping target compile | ~1,000 build steps |
| Cook (4 maps) | the bulk of the time |
| Stage / pak / iostore / archive | minutes |
| Total | 1-2 h, ~12 GB output |

Plus the build tree itself. Budget ~150 GB free for cook + stage + archive.
