# Packaging reference

Detail layer for `package-carla-ue4`. SKILL.md carries the procedure and common
failures. Everything here is verified against `Util/BuildTools/Package.sh`,
`Linux.mk`, `Import.py`, `PrepareAssetsForCookingCommandlet.cpp` and
`Util/ImportAssets.sh` in this checkout.

## Contents

- **Environment** — roots, skill knobs, and how they map onto `Package.sh` flags
- **Relocating artifacts out of Dist/** — `PACKAGE_DEST` / `PACKAGE_DEST_MODE`
- **What `make package` runs** — the three stages and the two cook paths
- **Package.json schema** — required keys, `path` format, `.xodr`/`.bin` sidecars
- **Installing an asset package** — `ImportAssets.sh` and its constraints
- **Cost** — time, disk, and why it is not resumable
- **P1** — the wheel stage needs `build` in the active `python3`
- **P2** — stale `.tar` corrupts the next run
- **P3** — cook OOM
- **Not covered** / **Related skills**

## Environment

Provided by `scripts/env.sh`, which derives the roots from its own location and
assumes no particular environment manager.

| Var | Default | Meaning |
|---|---|---|
| `UE4_ROOT` | none; export it | CarlaUnreal UE 4.26 fork, already built |
| `CARLA_UE4_ROOT` | `$PWD` if a checkout, else path-derived | carla source, branch ue4-dev |
| `CARLA_PY_VERSION` | unset | Leave empty: the wheel stage uses the active `python3`. Set only to force a version-suffixed interpreter (`python<X.Y>`), which must resolve inside the active env. |
| `CARLA_ENV_ACTIVATE` | unset | Optional path to an activate script to source — the manager-agnostic escape hatch for non-interactive runs. |

Skill knobs, mapped onto `Package.sh` flags:

| Var | Default | Flag |
|---|---|---|
| `PACKAGES` | `Carla` | `--packages=` |
| `PACKAGE_CONFIG` | `Shipping` | `--config=` |
| `PACKAGE_ZIP` | `1` | `0` adds `--no-zip` |
| `CLEAN_INTERMEDIATE` | `0` | `1` adds `--clean-intermediate` |
| `ARCHIVE_SUFIX` | empty | `--archive-sufix=` (CARLA's spelling) |
| `TARGET_ARCHIVE` | empty | `--target-archive=`, folds packages into one archive |
| `PACKAGE_DEST` | empty | not a `Package.sh` flag — post-build relocation dir |
| `PACKAGE_DEST_MODE` | `move` | `move` or `copy`; only acts when `PACKAGE_DEST` is set |

`--carsim` exists upstream but is not exposed; it rewrites the uproject.

### Relocating artifacts out of Dist/

`PACKAGE_DEST` and `PACKAGE_DEST_MODE` are skill-level, applied by
`scripts/package.sh` **after** verification — `make`/`Package.sh` always write
to `Dist/` first, and only artifacts that passed the size/existence checks are
relocated. `move` (the default) leaves nothing behind, so a release does not
sit duplicated as a ~10 GB copy; `copy` keeps `Dist/` as the canonical build
tree. The destination is created if absent; setting it to `Dist/` itself is a
no-op. With `TARGET_ARCHIVE` (several packages folded into one archive) the
single archive is relocated once. For `--no-zip`, the relocated entry is the
staged `CARLA_<config>_<tag>/` directory, not a tarball.

## What `make package` runs

`package: CarlaUE4Editor PythonAPI.wheel` then `Package.sh`, so:

1. **CarlaUE4Editor** — LibCarla server, osm2odr, plugins, editor compile
2. **PythonAPI.wheel** — `BuildPythonAPI.sh --build-wheel`
3. **Package.sh** — cook, stage, copy extras, archive

Two code paths inside `Package.sh`:

| | Release (`Carla`) | Asset package (any other name) |
|---|---|---|
| Cook | `RunUAT.sh BuildCookRun -cook -stage -archive -package -iterate` | `UE4Editor -run=PrepareAssetsForCooking`, then `-run=cook -cooksinglepackage` batched under a 1000-char map string |
| Selection | `Config/DefaultGame.ini` `MapsToCook` (13 towns) + `DirectoriesToAlwaysCook` | `<Name>.Package.json` |
| Staged | `Dist/CARLA_<config>_<tag>/LinuxNoEditor` | `Dist/<Name>_<tag>/` |
| Archive | `tar -czf` → `.tar.gz` | `tar -rf` → `.tar`, then `gzip -f` after the loop |

Both end as `.tar.gz`, which is what `ImportAssets.sh` globs for.

Artifact names (`<tag>` = `get_git_repository_version`: the branch name after
`ue4/`, else the short SHA plus `-dirty` when tracked files are modified):

- `Dist/CARLA_<tag>.tar.gz` — Shipping release (config omitted from the name)
- `Dist/CARLA_<config>_<tag>.tar.gz` — any other config
- `Dist/<Name>_<tag>.tar.gz` — asset package

## Package.json schema

Located by **recursive search** under `Unreal/CarlaUE4/Content` for
`<Name>.Package.json` (`GetFirstPackagePath`, first match wins). The package
name is the filename minus the suffix — it needs no directory of its own, which
is what allows exporting stock maps in place. `Import.py` writes to
`Content/<Name>/Config/`; the docs put export-only definitions in
`Content/Carla/Config/`. Both work.

```json
{
    "props": [
        {"name": "Bench", "path": "/Game/MyPkg/Static/Bench/SM_Bench.SM_Bench", "size": ""}
    ],
    "maps": [
        {"name": "MyMap", "path": "/Game/Carla/Maps/MyMap", "use_carla_materials": true}
    ]
}
```

**Both keys are mandatory even when empty** — the commandlet uses
`GetArrayField`, which fails hard on a missing key. `path` is the content path
with `Unreal/CarlaUE4/Content/` replaced by `/Game/`. The commandlet reads only
`path` from props; `name` and `size` are optional.

Per map, `Package.sh` also copies, if found anywhere under `Content/`:

- `<MapName>.xodr` — the OpenDRIVE network. Without it the map imports but has
  no roads, and nothing warns you.
- `<MapName>.bin` — navigation and Traffic Manager data.

Tagged-materials registries (instance segmentation) are generated by
`GenerateTaggedMaterialsRegistry` and cooked separately; when packaging
anything other than `Carla` alone, they are built for all packages at once.

## Installing an asset package

`ImportAssets.sh` is copied into every release. It runs
`find Import/ -type f -name "*.tar.gz"` and untars with `--keep-newer-files`,
so it must run from the **release root**, not from `Import/`:

```bash
cp Dist/MyMaps_<tag>.tar.gz <extracted-release>/
cd <extracted-release>
cp ../Dist/MyMaps_<tag>.tar.gz Import/
./ImportAssets.sh
```

Run it from the release root, not `Import/` — upstream
`tuto_A_create_standalone.md` says otherwise.

Constraints: asset packages import only into a **packaged** release, never a
source build, and a Linux package cannot be imported into a Windows release.

## Cost

- **Time:** 30-90 min cold for a release; shader compilation dominates. A
  single-town asset package is far cheaper. Code-only changes re-package
  quickly; content changes re-cook and do not.
- **Disk:** ~30 GB for staged tree plus tarball, on top of a ~120 GB build.
- **Not resumable.** A kill at minute 70 restarts the cook, and the cook
  `rm -Rf`s the staged folder on entry despite `-iterate`.

## P1 — the wheel stage needs `build` in the active `python3`

`PythonAPI.wheel` runs `/usr/bin/env python3 -m build` (default
`PY_VERSION_LIST=3`). It runs after the editor compile and **before** the cook,
so a miss costs the compile but not the cook. No environment manager is
required: any env whose `python3` imports `build` works.
`scripts/package.sh` verifies this up front (`carla_require_wheel_python`) and
forwards **no** `--python-version`, so the active env's `python3` is used as-is.

Set `CARLA_PY_VERSION` only when you need a specific interpreter, and it must
resolve inside the active env: a version-suffixed name like `python3.10` can
escape a venv to an unrelated interpreter that lacks `build`.

## P2 — stale `.tar` corrupts the next run

Asset packages archive with `tar -rf`, which **appends**. If a run dies between
the tar and the trailing `gzip -f`, a `Dist/<Name>_<tag>.tar` survives and the
next run appends to it, silently producing a corrupt, oversized archive.
The prerequisite checks warn; `scripts/package.sh` refuses to start.

## P3 — cook OOM

The cook parallelises and each worker is memory-hungry. This skill exposes no
knob for it: `make package` calls `Package.sh`, which calls `RunUAT.sh
BuildCookRun` — nothing in that chain takes a job count. Free RAM or add swap.
A killed cook is not wasted: `BuildCookRun` runs with `-iterate`, so already-
cooked assets are reused on the next attempt.

## Not covered

`DefaultGame.ini` editing (subset releases are better achieved with an asset
package), the Docker ingestion route (`Util/Docker/docker_tools.py`), Windows
packaging, and `make import` of new FBX assets — that last one is a separate
workflow with its own commandlets.
## P-ROS2-1: the cook succeeds, then UE4Editor deadlocks on shutdown

Verified 2026-08 on a ROS-2-enabled Shipping cook of this checkout.

Symptom: the cook log ends with

```
LogInit: Display: Success - 0 error(s), 34538 warning(s)
Execution of commandlet took:  109.12 seconds
LogShaderCompilers: Display: Shaders left to compile 0
LogHttp: Display: cleaning up 0 outstanding Http requests.
LogContentStreaming: Display: There are 1 unreleased StreamingManagers
```

...and then nothing, for as long as you let it run (51 minutes here). The cook is
finished and correct — `Saved/Cooked` holds the full output — but the editor never
exits: the process drops to **2 threads sleeping in `futex_wait_queue`** with no
I/O, and UAT blocks waiting for the child.

Diagnosis, without guessing:

```bash
ls -l --time-style=+%H:%M:%S <cook log>     # log frozen?
P=$(pgrep -f 'UE4Editor.*CarlaUE4.uproject')
grep -E '^State|^Threads' /proc/$P/status   # S (sleeping), Threads: 2
cat /proc/$P/wchan                          # futex_wait_queue
du -sh Unreal/CarlaUE4/Saved/Cooked         # cooked data present
```

Recovery: `kill <editor pid>` (never `pkill -f`, which also matches your own
shell). UAT then reports `Cook failed` / `Error_UnknownCookFailure` and `make`
exits 25 — a **false** failure, since the cook did complete. **Re-run
`package.sh`**: `Saved/Cooked` is reused and the second cook is incremental.
`package.sh` detects this case and says so.

Do not "fix" it by deleting `Saved/Cooked` — that throws away the work and the
next cook starts from zero.
