# Lessons — building CARLA ue4-dev on Linux (Ubuntu 24.04)

Battle-log of every non-obvious thing hit while building this end-to-end. Each
lesson: **symptom → root cause → fix → where it's encoded**. New lessons append
here as the build proceeds; durable ones also live in the step scripts/SKILL.md.

---

### L1 — Wrong branch ships a different build system
- **Symptom:** local `carla/` was on `ue5-dev`; root had `CarlaSetup.sh` + `CMakePresets.json`. `make` targets absent.
- **Cause:** `ue5-dev` (UE 5.3) uses a CMake/`CarlaSetup` flow; `ue4-dev` (UE 4.26) uses the classic **Makefile** flow. They are parallel branches.
- **Fix:** `git checkout ue4-dev`. Verify with `git branch --show-current`.
- **Encoded:** `scripts/check_env.sh` warns if not on `ue4-dev`.

### L2 — Disk: need ~120 GB, not "31 GB"
- **Symptom:** first check showed 12–22 G free, 100% used.
- **Cause:** docs quote CARLA ~31 GB but omit that the UE4 fork balloons to **~80 GB** after Setup.sh + compile; plus intermediate build dirs.
- **Fix:** ensure ≥120 GB free before starting.
- **Encoded:** `scripts/check_env.sh` disk check (threshold 120 G).

### L3 — Ubuntu 24.04 is buildable via the in-repo compat shim
- **Symptom:** docs say 20.04/22.04 only.
- **Cause:** three 24.04 issues — old bundled `ld` vs glibc≥2.36 (`.relr.dyn`), PEP 668 externally-managed python, CMake ≥4.0 policy rejection.
- **Fix:** `Util/BuildTools/Ubuntu24Compat.sh` (sourced by Setup.sh) auto-patches all three: wraps CC/CXX with `-fuse-ld=lld`, sets `_SKIP_PIP_INSTALL`, wraps cmake with `-DCMAKE_POLICY_VERSION_MINIMUM=3.5`.
- **Encoded:** SKILL.md "Ubuntu 24.04 deltas"; `scripts/01` installs `lld`.

### L4 — `lld` is mandatory on 24.04, and it really is used
- **Symptom:** without lld the shim hard-fails: "bundled linker cannot link on this system".
- **Cause:** UE4's bundled `ld` (2019) can't read `.relr.dyn` from glibc≥2.36.
- **Fix:** `sudo apt install lld`. Confirmed active — UE4 build log shows `Link (lld)`.
- **Encoded:** `scripts/01_install_deps.sh` (in PKGS), `scripts/00` FAILs on 24.04 if missing.

### L5 — Anaconda `base` poisons the build toolchain
- **Symptom:** `python3`→anaconda 3.13, `ninja`/`cmake` also from conda.
- **Cause:** anaconda base on PATH; python 3.13 too new for CARLA's boost.python.
- **Fix:** build the client in a dedicated env with a compatible interpreter — **3.10-3.12 all build on this HEAD** (boost 1.90; the original ">3.10 breaks" note was pre-upgrade); 3.13/anaconda-`base` remain too new. Any manager (venv, conda, pyenv); don't rely on system/anaconda `base`. The skill is manager-agnostic: activate the env, then `scripts/02_client_env.sh` installs deps into it and `scripts/env.sh` derives its exact minor.
- **Encoded:** `scripts/env.sh` (interpreter resolution), `scripts/02_client_env.sh` (deps).

### L6 — numpy must be < 2.0
- **Cause:** bindings compiled against numpy 1.x C-API; import crashes under 2.x.
- **Fix:** pin `numpy<2.0.0` in the env (got 1.26.4).
- **Encoded:** `scripts/02_client_env.sh`.

### L7 — Python version must be consistent across boost and the wheel
- **Cause:** `--python-version` given to `make PythonAPI` forwards ARGS to the `setup` target, so boost.python (Setup.sh) and the wheel (BuildPythonAPI.sh) bind to one interpreter. Mismatch → `ImportError` on `import carla`.
- **Fix:** pass `--python-version=3.10` AND activate the matching env.
- **Encoded:** `scripts/04_build_pythonapi.sh`.

### L8 — UE4 must be fully built before CARLA
- **Cause:** CARLA's `CC`/`CXX` point into UE4's bundled clang-10 + libc++ + OpenSSL (`$UE4_ROOT/Engine/Extras/.../v17_clang-10.0.1-centos7`). These exist only after UE4 `Setup.sh` + `make`.
- **Fix:** build UE4 first; gate step 04 on `UE4Editor` existing.
- **Encoded:** `scripts/03` then `scripts/04`; both check the editor binary.

### L9 — UE4 `make` must be single-threaded
- **Cause:** parallel UBT invocations race / OOM; docs warn against `-j`.
- **Fix:** plain `make` (no `-j`) for the UE4 top-level build.
- **Encoded:** `scripts/03_build_ue4.sh`.

### L10 — UE4 Setup.sh's apt step needs sudo but no-ops when satisfied
- **Cause:** `Engine/Build/BatchFiles/Linux/Setup.sh` runs `sudo apt-get install build-essential`, guarded by a `PackageIsInstalled` check.
- **Fix:** ensure `build-essential` is pre-installed → the sudo branch is skipped → UE Setup runs unattended in the background. (`GitDependencies --prompt` does not block on a clean depth-1 clone.)
- **Encoded:** verified build-essential present before backgrounding step 03.

### L11 — sudo has no TTY under Claude's `!` shell
- **Symptom:** `sudo: a terminal is required to read the password`.
- **Cause:** the `!`/non-interactive shell has no controlling terminal; sudo can't prompt.
- **Fix:** run the apt step in a real terminal (or pre-authorize sudo). Agents can't satisfy interactive sudo — must hand off to the user.
- **Encoded:** SKILL.md flags step 01 as "needs sudo / run in a real terminal".

### L12 — `conda activate` ≠ the conda binary being on PATH (historical)
- **Symptom:** `CondaError: Run 'conda init' before 'conda activate'` in a script, even though `which conda` works.
- **Cause:** `conda activate` is a *shell function* defined by `etc/profile.d/conda.sh`; a non-interactive shell never sourced it. The binary alone can't activate.
- **Superseded:** the skill no longer runs `conda activate`. `scripts/env.sh` resolves whatever interpreter the caller's ACTIVE env already provides (manager-agnostic), so this class of failure is designed out. Kept as history for anyone driving conda by hand: source `"$(conda info --base)/etc/profile.d/conda.sh"` before `conda activate`, or use `conda run -n <env> …`.

### L13 — A background wrapper's exit code can mask the real failure
- **Symptom:** task notification said "exit code 0" while the build actually died at conda activate.
- **Cause:** the launch wrapper ended with `echo "rc=$?"`; the *wrapper's* last command (echo) succeeded, so the harness saw 0.
- **Fix:** treat the **log contents / produced artifacts** as ground truth, not the wrapper's exit code. Append `rc=$?` to the log and grep it; verify the actual output (wheel, binary) exists.
- **Encoded:** step relaunch appends `rc` into the log; verification checks for real artifacts.

### L14 — Preflight "Content present" false-positive
- **Symptom:** preflight reported content present seconds after the clone started.
- **Cause:** `git clone` creates the target dir immediately; the ~31 GB checkout finishes much later.
- **Fix:** require `Content/Carla/.git` AND ≥1 non-`.git` entry (working tree populated only at checkout end).
- **Encoded:** `scripts/check_env.sh` content check.

### L15 — `make package` also needs the client env active (wheel sub-build)
- **Symptom:** after the long editor compile, `make package` died: `/usr/bin/python3.10: No module named build.__main__`, `make: *** [PythonAPI.wheel] Error 1`. No `Dist/` produced.
- **Cause:** the `package` target's `PythonAPI.wheel` prerequisite runs `python${CARLA_PY_VERSION} -m build`. With the client env not active, that resolved to **system** python (which lacks the `build` module). Step 04 worked because it resolved the active interpreter; step 06 originally didn't.
- **Fix:** activate `${CARLA_CONDA_ENV}` in step 06 too (any step that triggers the wheel build must). The editor compile is cached, so re-running only redoes the wheel + cook → `Dist/`.
- **Encoded:** `scripts/06_build_editor.sh` now sources `env.sh` and resolves the active client interpreter, exactly like step 04.

### L13 (reconfirmed) — the masked exit code bit us again here
- `make package` failed with `Error 1`, the wrapper appended `package rc=2`, yet the task notification still reported "exit code 0". Ground-truth verification (grep the logged `rc=`, check for `Dist/`) is what caught it. **Never trust the harness exit summary for backgrounded build chains.**

### L16 — Never run `make package`/`make PythonAPI` without `UE4_ROOT` exported (baked-wrong compiler wrapper)
- **Symptom:** `make package` died in seconds on LibCarla: `clang++.sh: line 2: /Engine/Extras/ThirdPartyNotUE/SDKs/.../bin/clang++: No such file or directory` — note the leading `/Engine`, missing the checkout prefix. `PACKAGE_RC=2`.
- **Cause:** CARLA's Setup.sh → Ubuntu24Compat.sh writes `Build/clang++.sh` as `exec "${CXX}" -fuse-ld=lld ...`, where `${CXX}` = `${UE4_ROOT}/Engine/Extras/.../clang++`. Running `make` directly (not via the skill step) left `UE4_ROOT` **unset**, so the wrapper baked `/Engine/...` — a nonexistent absolute path. Every compile then fails.
- **Fix:** always drive package/PythonAPI through the skill step (`scripts/06_build_editor.sh`, `scripts/04_build_pythonapi.sh`), which `source env.sh` → exports `UE4_ROOT`. To recover after a bad bake: `rm -f Build/clang{,++}.sh` and rerun via the step — Setup.sh regenerates the wrapper with the correct absolute path on the next configure.
- **Encoded:** step 06/04 source env.sh; this lesson.

### L17 — Headless RPC server without a cook: use `-game -nullrhi`, NOT `-RenderOffScreen`
- **Context:** to test a newly-authored asset (e.g. a vehicle) over the RPC API you do
  **not** need the 30–60min `make package` cook. The editor binary can serve the game
  directly against the **uncooked** editor project.
- **Symptom:** `UE4Editor CarlaUE4.uproject /Game/Carla/Maps/Town02 -game
  -RenderOffScreen ...` boots, opens RPC port 2000, then **SIGSEGVs the render thread**
  a few seconds in: `FDistanceFieldVolumeTexture::IsValidDistanceFieldVolume()` →
  `FPrimitiveSceneShaderData` → `UpdateGPUSceneInternal` → `Render_CARLA`
  (`invalid read at 0x1a`). (The trailing crash in the log is a red herring — the
  *CrashReportClient* itself dies on `libGL`/dummy-SDL; scroll UP to the first
  `Signal 11` for the real stack.)
- **Root cause:** mesh **distance-field / GPU-scene** data is generated during the
  **cook** (DDC). Uncooked meshes have null distance-field volumes; the real renderer
  dereferences one and crashes. Rendering-on = the crash path.
- **Fix:** launch with **`-nullrhi`** (no render thread at all) instead of
  `-RenderOffScreen`. RPC + physics + traffic-manager/autopilot all run; the server is
  ready in ~15–20s and a `carla.Client` can spawn + drive. Full command:
  `UE4Editor CarlaUE4.uproject /Game/Carla/Maps/Town02 -game -nullrhi -nosound
  -carla-rpc-port=2000 -carla-streaming-port=2001`.
  Pick a **light map** (Town01/Town02) to minimise first-load time.
- **Caveat:** `-nullrhi` produces **no camera/lidar images** — sensor pipelines need a
  real RHI (`-RenderOffScreen` on a **cooked/packaged** build, which has valid distance
  fields). So: `-nullrhi` for spawn/registration/physics smoke-tests without a cook;
  `make package` (or `-RenderOffScreen` on the package) only when you need rendering.
- **Process hygiene:** kill the server with `pkill -x UE4Editor` — `pkill -f
  CarlaUE4.uproject` also matches (and kills) the launching shell itself (exit 144).
- **Want a VISIBLE WINDOW (not headless)?** Drop `-nullrhi`, add `-windowed
  -ResX=1280 -ResY=720`, and disable the crashing feature at load via an `-ini:`
  override (no file edit):
  `-ini:Engine:[/Script/Engine.RendererSettings]:r.GenerateMeshDistanceFields=False`.
  Verified: Town02 window on `DISPLAY=:1`, RPC ready ~20s, `vehicle.ford.testcar`
  spawned + drove. Real rendering minus DF shadows/AO. (Equivalent: flip
  `r.GenerateMeshDistanceFields=True→False` in `Config/DefaultEngine.ini`, revert
  after.) Encoded as `WINDOW=1` in add-carla-vehicle `scripts/run_server.sh`.
- **Encoded:** this lesson; add-carla-vehicle [[add-carla-vehicle]] V11 + SKILL.md +
  `scripts/run_server.sh` (`WINDOW=1`).

---
_Open items (update as build finishes): `make package` cook/shader time on this host; headless server GPU/Vulkan requirement; final `generate_traffic.py` round-trip._
