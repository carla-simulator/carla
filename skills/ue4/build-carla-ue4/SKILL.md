---
name: build-carla-ue4
description: Builds CARLA (branch ue4-dev, Unreal Engine 4.26) from source on Linux end-to-end — UE4 fork, the Carla server (editor C++ modules), the LibCarla Python client wheel, and content — then verifies them against a from-source server. Optionally builds the native ROS 2 interface in (ROS2=1 → --ros2, Fast-DDS/CycloneDDS/Zenoh), which is compile-time only and cannot be enabled later. Use when the user asks to "build CARLA from source", "compile CARLA ue4-dev", "set up CARLA on Ubuntu (incl. 24.04)", "build CARLA with ROS2 support", or "produce a CARLA server + client wheel". Cooking a distributable Dist/ package is a separate skill (package-carla-ue4); running the build is run-carla-server.
license: MIT
compatibility: Linux x86_64 (Ubuntu 20.04/22.04/24.04). Requires an Epic-linked GitHub account (the UE4 fork is private), ~120 GB free disk, an NVIDIA GPU for the rendering server, and an active Python client env (venv, conda, or system — no manager assumed). A full build takes several hours.
metadata:
  group: ue4
  prerequisites: scripts/check_env.sh
  reference: references/lessons.md
  ros2: references/ros2.md
  requires: run-carla-server
---

# Build CARLA ue4-dev on Linux (end-to-end)

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Builds CARLA from the **`ue4-dev`** branch (Unreal Engine **4.26**) on Ubuntu
20.04 / 22.04 / **24.04**. Each step is an idempotent script under `scripts/`;
the scripts ARE the procedure — running them in order performs and verifies the
build. Not for `ue5-dev` — that branch uses a different (CMake/CarlaSetup) build
system entirely (L1).

## Scope

This skill compiles CARLA's **source artifacts** and proves them, nothing more:

1. **Engine** (if needed) — the CarlaUnreal UE 4.26 fork → `UE4Editor` (step 03).
2. **Server** (if needed) — the Carla editor C++ modules incl the server plugin
   and CarlaTools, runnable uncooked via `-nullrhi` (step 06, `make CarlaUE4Editor`).
3. **PythonAPI** (if needed) — LibCarla client + boost bindings → wheel installed
   into the active env (step 04).
4. **Launch** (opt-in) — open the UE4 editor UI for interactive work
   (`TARGET=launch` on step 06, `make launch`).

Each build step is idempotent and skips when its artifact is already present
(`FORCE=1` to rebuild). Two adjacent jobs are **out of scope** and delegated:

- **Cooking/packaging** a distributable `Dist/` tarball → **[[package-carla-ue4]]**
  (`make package`). Do not cook here.
- **Running/serving** the build (headless smoke-tests, windowed demos) →
  **[[run-carla-server]]**. Step 07 uses it to verify.

> **Full battle-log:** [`references/lessons.md`](references/lessons.md) — every
> non-obvious failure hit during this build, with root cause + fix. Read it
> before debugging. `L#` citations below point at it.

## Instructions

Copy this checklist and track progress — the build is long and gated, so a
skipped step usually surfaces an hour in:

```
Build Progress:
- [ ] Step 0: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 1: apt deps (sudo — real terminal)   |  Step 2: prepare Python client env
- [ ] Step 3: build the UE4 fork (engine)  ||  Step 5: fetch content   (parallel, long)
- [ ] Step 6: build the server — make CarlaUE4Editor (needs 3)
- [ ] Step 4: build PythonAPI + install wheel (needs 2+3)
- [ ] Step 7: verify — boot from-source server (run-carla-server) + generate_traffic.py (needs 4+5+6)
- [ ] (opt-in) Step 6 TARGET=launch: open the UE4 editor UI (make launch)
- [ ] (separate skill) cook a Dist/ package -> package-carla-ue4
```

## Prerequisites

- Linux, x86_64. Ubuntu 20.04/22.04 supported; **24.04 works** via the in-repo
  compat shim (see deltas below).
- ~**120 GB** free disk (UE4 ~80 GB + content ~31 GB + intermediate builds, L2).
- A CUDA-capable NVIDIA GPU (RTX-class) for the rendering server.
- **A Python client env, active** — venv, conda, or system; no manager is
  assumed (mechanism in "Python client env" below). 3.10–3.12 all build on this
  HEAD (L5's ">3.10 breaks" caveat is stale). No env yet? Create one, e.g.:
  ```bash
  python3.12 -m venv ~/carla-client && source ~/carla-client/bin/activate   # venv
  # or:  conda create -y -n carla-ue4 python=3.12 && conda activate carla-ue4
  ```
- **Epic-linked GitHub account** — the UE4 fork is private to the Epic Games
  org. Before step 03 the user MUST link their account
  (https://www.unrealengine.com/en-US/ue-on-github) and be able to clone
  `https://github.com/CarlaUnreal/UnrealEngine.git` (branch `carla`). If that
  clone returns `repository not found`/`403`, the account is not linked —
  STOP and surface exactly this; nothing downstream can proceed.

Set the roots (`scripts/env.sh` resolves them, all overridable):

- `UE4_ROOT` — the CarlaUnreal UE 4.26 fork checkout. **No default** — export it.
- `CARLA_UE4_ROOT` — the carla `ue4-dev` source checkout. Defaults to `$PWD` if
  it is a checkout, else a path-derived guess; export it otherwise.

## Quick start

```bash
cd skills/build-carla-ue4
source ~/carla-client/bin/activate     # activate ANY compatible client env first
export UE4_ROOT=/path/to/UnrealEngine_4.26
export CARLA_UE4_ROOT=/path/to/carla   # or run from inside the checkout

bash scripts/check_env.sh
bash scripts/01_install_deps.sh          # sudo — run in a real terminal (L11)
bash scripts/02_client_env.sh &
bash scripts/03_build_ue4.sh &           # long
bash scripts/05_fetch_content.sh &       # long
wait
bash scripts/06_build_editor.sh          # TARGET=launch also opens the editor UI
bash scripts/04_build_pythonapi.sh
bash scripts/07_verify.sh

# cook a distributable package (separate skill), only if you need one:
#   bash ../package-carla-ue4/scripts/package.sh
```

## Steps

| # | Script | What | Notes |
|---|--------|------|-------|
| 0 | `scripts/check_env.sh` | Preflight report | read-only |
| 01 | `scripts/01_install_deps.sh` | apt deps incl `lld`, `libtiff-dev`, `g++-12` | **needs sudo** |
| 02 | `scripts/02_client_env.sh` | install client build deps + `numpy<2` into the **active** env | no sudo; env-manager-agnostic |
| 03 | `scripts/03_build_ue4.sh` | **engine** — UE4 fork: Setup → GenerateProjectFiles → make | ~10GB dl + ~1h; **no `-j`** (L9); skips if `UE4Editor` built |
| 04 | `scripts/04_build_pythonapi.sh` | **PythonAPI** — LibCarla client + boost + wheel → install to active env | needs 02+03; skips if `import carla` works (`FORCE=1`) |
| 05 | `scripts/05_fetch_content.sh` | `git clone` carla-content (bitbucket) → Content/Carla | ~31GB; parallel-safe |
| 06 | `scripts/06_build_editor.sh` | **server** — `make CarlaUE4Editor` (Carla plugin + CarlaTools); `TARGET=launch` → `make launch` (opt-in editor UI); `ROS2=1` adds the native ROS 2 interface | needs 03; incremental, no cook; skips if plugin `.so` built and the ROS 2 flag matches (`FORCE=1`) |
| 07 | `scripts/07_verify.sh` | boot **from-source** server via [[run-carla-server]] (uncooked `-nullrhi`), run `generate_traffic.py` | proof; needs 04+05+06 |

### ROS 2 native interface (`ROS2=1`, opt-in)

CARLA can publish DDS topics from **inside** the server (no `carla-ros-bridge`).
That support is **compile-time only** — decide it here:

```bash
ROS2=1 bash scripts/06_build_editor.sh      # == make CarlaUE4Editor ARGS="--ros2"
```

One flag, three consumers, because `Linux.mk` forwards `ARGS` down
`CarlaUE4Editor → LibCarla.server.release → setup`: `Setup.sh` builds Fast-DDS +
CycloneDDS + Zenoh from source into `Build/*-install` (long, cached),
`BuildLibCarla.sh` builds `carla_ros2`, `BuildCarlaUE4.sh` writes `Ros2 ON` into
`Unreal/CarlaUE4/Config/OptionalModules.ini` — the file `Carla.Build.cs` reads to
define `WITH_ROS2`.

Three things to know, all consequences of that ini being **sticky global state**:

- **A plain (`ROS2=0`) re-run turns it back OFF** — the ini is rewritten every
  build. Step 06 detects the flip and rebuilds instead of skipping.
- **`make package` re-runs the editor build**, so [[package-carla-ue4]] needs
  `ROS2=1` too or the cooked package loses ROS 2 — silently.
- **`parse-options: unrecognized option '--ros2'`** on stderr is expected and
  harmless (`BuildUE4Plugins.sh` drops unknown options).

Building it in does **not** turn it on: the server also needs the `--ros2`
runtime flag ([[run-carla-server]] `ROS2=1`). No ROS 2 installation is required
to build or run — only to consume the topics. Full detail, including how to prove
the support is really in the binary, in [`references/ros2.md`](references/ros2.md).

Step 06 (`server`) doubles as the cheap incremental recompile after touching
`Unreal/CarlaUE4/Plugins/` — binaries only, no cook — and is the target
[[add-carla-vehicle]] points at when it reports
`STATUS=REBUILD_CARLATOOLS_REQUIRED`. (Packaging/running stay out of scope — see
"Scope".)

Every step sources `scripts/env.sh`, which resolves the roots and — for the
Python steps — the active interpreter, assuming no environment manager.
**Never run `make CarlaUE4Editor`/`make PythonAPI` outside the steps with
`UE4_ROOT` unset** — the generated compiler wrapper bakes a broken path and all
compiles fail until it is regenerated (L16).

## Python client env (no manager assumed)

The client build (boost.python bindings + wheel) must bind to **one**
interpreter, so `scripts/env.sh` derives the exact X.Y of the **active**
env's `python3` and forwards `--python-version` to both stages (L7). You bring
the env; the skill never creates one:

- Activate any env before step 02 (venv/conda/pyenv/system).
- Non-interactive? Point `CARLA_ENV_ACTIVATE` at its activate script — an
  optional hook, and the only one the skill looks at.
- Set `CARLA_PY_VERSION` **only** to force a specific minor (e.g. `3.10`); it
  must then resolve as `python<pin>` inside that env. Left unset, the active
  interpreter's own minor is used.
- `numpy < 2` is required — the bindings crash on import under numpy 2.x (L6).
  Step 02 pins it into the active env.

## Ubuntu 24.04 deltas (beyond the official docs)

`ue4-dev` ships `Util/BuildTools/Ubuntu24Compat.sh` (sourced by Setup.sh) which
auto-patches three issues at build time:

1. **Old bundled linker vs glibc ≥ 2.36** — UE4's 2019 `ld` can't read
   `.relr.dyn`; the compiler is wrapped with `-fuse-ld=lld` ⇒ **`lld` must be
   installed** (step 01 does; the shim hard-fails otherwise, L4).
2. **PEP 668 externally-managed Python** — the build sets `_SKIP_PIP_INSTALL`
   and leaves the wheel in `PythonAPI/carla/dist/`; step 04 installs it into the
   active env.
3. **CMake ≥ 4.0** rejects old `cmake_minimum_required` — cmake is wrapped to
   inject `-DCMAKE_POLICY_VERSION_MINIMUM=3.5`.

## Verify

`scripts/07_verify.sh` is the gate: it boots the **uncooked, from-source** server
via [[run-carla-server]] (`UE4Editor -game -nullrhi`) — no cook needed — waits for
the RPC port, and runs `generate_traffic.py` against it from the active client
env. Judge steps by their **artifacts** (wheel importable, `UE4Editor` binary,
`libUE4Editor-Carla*.so`), never by a background wrapper's exit code (L13). The
uncooked server has RPC + physics + traffic but **no sensor images**; for a
cooked, camera/lidar-capable server, package with [[package-carla-ue4]] and serve
it with [[run-carla-server]] `PACKAGED=1`.

## Examples

**Example 1: full build from source (the common case)**

User says: "build CARLA ue4-dev from source"

Activate any client env, export `UE4_ROOT` (+ `CARLA_UE4_ROOT` if not running
from the checkout), then run the Quick start in order: `check_env.sh` → `01` →
`02`+`03`+`05` (parallel) → `06` (server) → `04` (PythonAPI) → `07` (verify).
Result: `UE4Editor` binary, the Carla server plugin (`libUE4Editor-Carla*.so`),
and a client wheel installed in the env — proven by step 07 booting the
from-source server and running `generate_traffic.py`. No `Dist/` package is
produced; cook one with [[package-carla-ue4]] only if you need a distributable.

**Example 2: build against a plain venv, no conda (manager-agnostic)**

User says: "build the CARLA client using my venv at ~/carla-client, pinned to 3.10"

```bash
source ~/carla-client/bin/activate     # any manager — venv here
export CARLA_PY_VERSION=3.10           # only needed to force a specific minor
bash scripts/02_client_env.sh          # installs deps + numpy<2 into the venv
bash scripts/04_build_pythonapi.sh     # boost + wheel bind to the venv's python3.10
```

`env.sh` resolves the venv's interpreter. Result:
the wheel installed into `~/carla-client`.

**Example 3: rebuild only the server / CarlaTools editor modules**

User says: "I edited C++ under Plugins/CarlaTools, recompile it" (or add-carla-vehicle
reported `STATUS=REBUILD_CARLATOOLS_REQUIRED`)

```bash
FORCE=1 bash scripts/06_build_editor.sh    # make CarlaUE4Editor — binaries only, no cook
```

Incremental (minutes). `FORCE=1` because the plugin `.so` already exists — the
step is otherwise idempotent. Result:
`Unreal/CarlaUE4/Plugins/CarlaTools/Binaries/Linux/libUE4Editor-CarlaTools*.so`.

**Example 4: build the editor for interactive work (open the UI)**

User says: "build CARLA and open it in the UE4 editor"

```bash
TARGET=launch bash scripts/06_build_editor.sh   # builds CarlaUE4Editor + opens the editor UI
```

Default `TARGET=server` builds the headless server modules; `TARGET=launch`
builds them and opens the editor UI. Packaging a distributable is a different
job — [[package-carla-ue4]].

## Troubleshooting

**Error: `No module named build.__main__` during the PythonAPI wheel (step 04)**
Cause: `make PythonAPI`'s wheel stage runs `python<ver> -m build`, and the
resolved interpreter lacks the `build` module — usually no client env was active,
so it fell through to system python (L15). (This also bites `make package` in
[[package-carla-ue4]] for the same reason.)
Solution: activate the client env (the one step 02 installed into) and re-run.

**Error: `clang++.sh: /Engine/.../clang++: No such file or directory`**
Cause: `make CarlaUE4Editor`/`make PythonAPI` was run with `UE4_ROOT` unset, so
the generated `Build/clang{,++}.sh` wrapper baked a `/Engine/...` path missing the
checkout prefix (L16).
Solution: `rm -f Build/clang{,++}.sh` and rerun via the skill step (which exports
`UE4_ROOT`); Setup.sh regenerates the wrapper correctly.

**Error: `import carla` crashes / `ImportError`**
Cause: numpy ≥ 2 in the env (L6), or boost and the wheel bound to different
interpreters (L7).
Solution: ensure `numpy<2` (step 02), and drive the build through step 04 so
`env.sh` keeps one interpreter across boost + wheel.

**Error: `sudo: a terminal is required` (step 01)**
Cause: the non-interactive shell has no TTY for sudo (L11).
Solution: run step 01 in a real terminal, or pre-authorize sudo. Agents can't
satisfy interactive sudo — hand off to the user.

**Error: UE4 clone returns `repository not found` / `403` (step 03)**
Cause: the GitHub account is not linked to Epic — the fork is private.
Solution: link at unrealengine.com/en-US/ue-on-github, confirm you can clone
`CarlaUnreal/UnrealEngine.git`, then rerun.

## Outputs

- Engine — UE4 editor binary: `${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor`
- Server — Carla editor C++ modules: `Unreal/CarlaUE4/Plugins/*/Binaries/Linux/*.so`
  (incl `libUE4Editor-Carla*.so` and `libUE4Editor-CarlaTools*.so`), runnable
  uncooked via [[run-carla-server]]
- PythonAPI — client wheel installed into the active client env (`import carla`)
- Content — maps/assets: `${CARLA_UE4_ROOT}/Unreal/CarlaUE4/Content/Carla`

Not produced here: a `Dist/CARLA_*` package — cook that with [[package-carla-ue4]].

## Build system note

`ue4-dev` uses the classic **Makefile** flow (`make PythonAPI`, `make launch`,
`make package`, `make CarlaUE4Editor`) — NOT the root `CarlaSetup.sh`/CMake
flow seen on `ue5-dev`. The readthedocs "latest" Linux page matches this flow,
but this branch HEAD is modernized (a newer boost, the Ubuntu24 shims); trust
the scripts here over stale prose.
