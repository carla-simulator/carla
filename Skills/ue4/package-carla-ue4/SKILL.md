---
name: package-carla-ue4
description: Cooks and packages CarlaUE4 into distributable tarballs under Dist/ — either the full simulator or a standalone asset package of selected maps and props for later import. ROS2=1 keeps the native ROS 2 interface in the cooked package (a plain cook silently drops it). Use when the user asks to "package CARLA", "make a Dist build", "cook the project", "export a map as a package", "package CARLA with ROS2", or needs a build with working camera and lidar sensors.
license: MIT
compatibility: Linux. Requires a built CarlaUnreal UE 4.26 checkout (UE4_ROOT), a carla ue4-dev source checkout, an active Python env whose `python3` imports `build` + `carla` (venv, conda or system — no manager assumed), and ~30 GB free disk. A full release takes 30-90 minutes.
metadata:
  group: ue4
  prerequisites: scripts/check_env.sh
  reference: references/packaging.md
---

# Package CarlaUE4

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

`make package` produces two different artifacts depending on `--packages`:

| Mode | Command | Output | Cook driven by |
|---|---|---|---|
| **Release** | `bash scripts/package.sh` | `Dist/CARLA_<tag>.tar.gz` | `Config/DefaultGame.ini` |
| **Asset package** | `PACKAGES=<Name> bash scripts/package.sh` | `Dist/<Name>_<tag>.tar.gz` | `<Name>.Package.json` |

## Instructions

Copy this checklist and track progress — the run is long and gated, so a skipped
step usually surfaces an hour in:

```
Package Progress:
- [ ] Step 1: Activate the client Python env, run check_env.sh, clear FAILs
- [ ] Step 2: Cook — 2a full release, or 2b a named asset package
- [ ] Step 3: Verify the artifact exists, is the right size, and is in Dist/
- [ ] (opt-in) Run or import the build — only if asked to run/load it
```

### Step 1: Activate your Python env, then check prerequisites

The wheel stage runs `python3 -m build`, so **have your CARLA client
environment active first** — the one whose `python3` imports `build` and
`carla`. Any manager works (venv, conda, system); the skill assumes none and
pins no version. If you drive this non-interactively, either activate the env
in the same shell, or point `CARLA_ENV_ACTIVATE` at its activate script — the
only hook the skill looks at. Only set `CARLA_PY_VERSION` if you
deliberately need a version-suffixed interpreter — and it must resolve *inside*
that env, or the wheel escapes to the wrong Python.

```bash
bash scripts/check_env.sh
```

Fix warnings about the wheel python and missing content before starting — both
cost most of the build time before they surface.

### Step 2a: Full release

```bash
bash scripts/package.sh 2>&1 | tee /tmp/carla_package.log
```

30-90 min cold, not resumable.

### Step 2b: Standalone asset package

For distributing maps and props, or for a cheap sensor-capable build containing
one town instead of thirteen.

A package is defined by a JSON found anywhere under `Content/` — the package
name is the filename minus `.Package.json`. It needs no matching directory, so
stock maps can be exported as-is:

```bash
# define it (writes Content/Carla/Config/OneTown.Package.json)
python3 scripts/package_json.py OneTown --map Town02 --carla-materials

# cook it
PACKAGES=OneTown bash scripts/package.sh
```

Each map needs `<MapName>.xodr` under an `OpenDrive/` directory somewhere in
`Content/`. `package_json.py --check` verifies this; a missing `.xodr` yields a
package that imports but has no road network.

### Step 3: Verify

`scripts/package.sh` already checks the expected artifact by name, size and
location, and fails loudly if it is missing — `make` can exit 0 having produced
nothing. Confirm independently:

```bash
ls -la "${CARLA_UE4_ROOT:?}"/Dist/
```

A release is several GB; low megabytes means the cook ran without content.

**This is where the skill stops by default.** The deliverable is the verified
artifact in `Dist/` (or in `PACKAGE_DEST`, below). Booting or importing it is a
separate, opt-in step — do it only when the request explicitly asks to run or
load the result.

### Placing artifacts outside Dist/

Set `PACKAGE_DEST` to a directory to land the artifact elsewhere. It is applied
after verification: `PACKAGE_DEST_MODE=move` (the default) relocates the verified
artifact and leaves no multi-GB duplicate in `Dist/`; `PACKAGE_DEST_MODE=copy`
keeps `Dist/` intact. So "package CARLA into <dir>" is a move, "package CARLA and
copy it to <dir>" is a copy.

```bash
# move the release into ~/dev/Carla/packages/ (nothing left in Dist/)
PACKAGE_DEST=~/dev/Carla/packages bash scripts/package.sh
# copy instead, keeping Dist/
PACKAGE_DEST=~/dev/Carla/packages PACKAGE_DEST_MODE=copy bash scripts/package.sh
```

### ROS 2 packages (`ROS2=1`)

```bash
ROS2=1 bash scripts/package.sh          # release WITH the native ROS 2 interface
```

`Package.sh` has **no** ROS 2 option — support is inherited from the editor
build. But `make package` **depends on** `CarlaUE4Editor`, so every cook re-runs
`BuildCarlaUE4.sh`, which rewrites `Unreal/CarlaUE4/Config/OptionalModules.ini`.
Cook without `ROS2=1` and that rewrite says `Ros2 OFF`: the package loses ROS 2
even though the checkout was built with it, with nothing in the log to say so.
Hence:

- `ROS2=1` appends `--ros2` to the forwarded `ARGS` (`Package.sh` and
  `BuildPythonAPI.sh` print a harmless `unrecognized option '--ros2'`; only
  `Setup.sh`/`BuildLibCarla.sh`/`BuildCarlaUE4.sh` act on it).
- Cooking **without** `ROS2=1` from a ROS-2-built checkout warns and pauses 5 s.
- After the cook, `package.sh` re-reads the ini and **fails** if `ROS2=1` did not
  end up as `Ros2 ON` — the staged binaries would have no ROS 2.
- If the middleware deps (`Build/{fast-dds,cyclone-dds,zenoh}-install`) are not
  built yet, this cook builds them first — add that to the 30-90 min.

Serve the result with [[run-carla-server]] `PACKAGED=1 ROS2=1` (the runtime
`--ros2` flag is separate); build-time detail in
[[build-carla-ue4]] `references/ros2.md`.

### Optional: run the packaged build

Only when the request asks to run or load the result (e.g. "package the engine
and Town15, then run the server and load it"). Packaging on its own boots
nothing.

```bash
# release: boot the packaged server, then load a town from a client
PACKAGED=1 bash ../run-carla-server/scripts/run_server.sh >/tmp/carla_pkg.log 2>&1 &
until nc -z 127.0.0.1 2000; do sleep 1; done
python -c "import carla; c=carla.Client('127.0.0.1',2000); c.set_timeout(60); \
           print('loaded', c.load_world('Town15').get_map().name)"
pkill -x CarlaUE4-Linux-        # NOT ...-Shipping: comm is truncated to 15 chars

# asset package: install into an extracted release first, from its ROOT
cp Dist/OneTown_TAG.tar.gz RELEASE/Import/
cd RELEASE && ./ImportAssets.sh
```

The packaged server process is `CarlaUE4-Linux-Shipping`, not `UE4Editor`.
Asset packages import only into a **packaged** release, never a source build,
and are platform-specific.

Flags (`PACKAGE_CONFIG`, `PACKAGE_ZIP`, `CLEAN_INTERMEDIATE`, `ARCHIVE_SUFIX`,
`TARGET_ARCHIVE`, `PACKAGE_DEST`, `PACKAGE_DEST_MODE`) and the JSON schema:
[references/packaging.md](references/packaging.md).

## Examples

**Example 1: full release (build only)**

User says: "package CARLA"

Check prerequisites, `bash scripts/package.sh`, verify size and location. Deliverable:
`Dist/CARLA_<tag>.tar.gz`, several GB. The skill stops here — nothing is booted.

**Example 2: export one town**

User says: "export Town02 as a package I can drop into a release"

`python3 scripts/package_json.py OneTown --map Town02 --carla-materials`, then
`PACKAGES=OneTown bash scripts/package.sh`. Result `Dist/OneTown_<tag>.tar.gz`
for `ImportAssets.sh`.

**Example 3: package, then run and load (opt-in)**

User says: "package the CARLA engine and Town15, then run the server and load
the packaged town"

Build the release (it contains the towns) and verify, then — because the request
asked for it — boot with `PACKAGED=1` and load the town from a client, per
"Optional: run the packaged build" above.

**Example 4: fast iteration**

User says: "I just need the staged build, skip the tarball"

`PACKAGE_ZIP=0 bash scripts/package.sh` leaves
`Dist/CARLA_Shipping_<tag>/LinuxNoEditor` runnable in place, skipping compression.

## Troubleshooting

**Error: `No module named build.__main__`**
Cause: the `PythonAPI.wheel` prerequisite runs `python3 -m build`, and the
active `python3` has no `build` module. It runs after the editor compile, before
the cook, so the compile is not wasted.
Solution: activate the env whose `python3` has `build` (or `pip install build`
into it) and re-run; compiled artifacts survive. `scripts/package.sh` verifies
this up front and forwards no version, so the active env's `python3` is used —
a hand-run `make package` uses whatever `python3` is on PATH.

**Error: `Package json file not found`**
Cause: no `<Name>.Package.json` anywhere under `Content/`.
Solution: `python3 scripts/package_json.py <Name> --map ...`. The name must match
the filename exactly, minus the suffix.

**Error: asset package imports but the map has no roads**
Cause: no `<MapName>.xodr` found under an `OpenDrive/` directory.
Solution: place it beside the `.umap`, named exactly like the map. Check with
`python3 scripts/package_json.py <Name> --check`.

**Error: package tarball absurdly large or corrupt**
Cause: content packages are built with `tar -rf`, which **appends**. A stale
`Dist/<Name>_*.tar` from an interrupted run is added to, not replaced.
Solution: `rm Dist/<Name>_*.tar` and re-run. `scripts/package.sh` refuses to
start when one is present.

**Error: release tarball is only a few MB**
Cause: cooked with no content.
Solution: build the carla content first, then re-package.

**Error: killed mid-cook, or the machine freezes**
Cause: OOM — the cook parallelises and each worker is memory-hungry.
Solution: free RAM or add swap; there is no parallelism knob (packaging.md P3).
Re-running is cheap — the cook is iterative, so cooked assets are reused.

## Outputs

- `Dist/CARLA_<tag>.tar.gz` — full release (Shipping; other configs carry the
  config in the name).
- `Dist/<Name>_<tag>.tar.gz` — standalone asset package.
- `Dist/CARLA_<config>_<tag>/LinuxNoEditor` — staged tree, runnable in place.

`<tag>` is the git short SHA, suffixed `-dirty` when tracked files are modified.
