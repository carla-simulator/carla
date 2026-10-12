# Lessons — run-carla-server

Battle-log. Each: **symptom → root cause → fix → where encoded**.
These were originally learned inside [[add-carla-vehicle]] /
[[build-carla-ue4]] (L17, V11, P6) and are consolidated here because
serving is its own capability; the source lessons remain authoritative history.

---

### S1 — Uncooked + real renderer headless = render-thread SIGSEGV; use `-nullrhi`
- **Symptom:** `UE4Editor <proj> <map> -game -RenderOffScreen` boots, opens RPC
  2000, then SIGSEGVs seconds later in
  `FDistanceFieldVolumeTexture::IsValidDistanceFieldVolume()`. The LAST crash in
  the log is the CrashReportClient dying on libGL/dummy-SDL — scroll UP to the
  first `Signal 11` for the real stack.
- **Root cause:** mesh distance-field / GPU-scene data is generated during the
  **cook**. Uncooked meshes have null distance-field volumes; the renderer
  dereferences one.
- **Fix:** headless on uncooked content → `-nullrhi` (no render thread at all).
  RPC, physics, Traffic Manager all work; no camera/lidar images. For sensor
  images, cook (`make package`) and run the package with `-RenderOffScreen`.
- **Encoded:** `scripts/run_server.sh` default mode; build L17.

### S2 — Windowed on uncooked content: disable distance-field generation via `-ini:`
- **Symptom:** same SIGSEGV as S1 when running windowed (real renderer).
- **Fix:** pass
  `-ini:Engine:[/Script/Engine.RendererSettings]:r.GenerateMeshDistanceFields=False`
  (no file edit needed). Verified: Town02 window on `DISPLAY=:1`, vehicle
  spawned + drove. Costs DF shadows/AO only.
- **Encoded:** `scripts/run_server.sh` `WINDOW=1` branch.

### S3 — `pkill -f CarlaUE4.uproject` kills your own shell
- **Symptom:** stopping the server aborted the calling script with exit 144.
- **Root cause:** `pkill -f` matches the *command line*, and the launching
  shell's own args contain the pattern — it self-matches.
- **Fix:** `pkill -x UE4Editor` (exact process name), or resolve PIDs with
  `pgrep` and kill them explicitly.
- **Encoded:** `scripts/run_server.sh` header; ue4-editor-python P6.

### S4 — Wait on the RPC port, not a fixed sleep
- **Symptom:** clients connecting after a fixed sleep raced first-load shader
  compilation and timed out on slower boots (heavy maps, cold DDC).
- **Fix:** poll: `until nc -z 127.0.0.1 <port>; do sleep 1; done`. Boot time
  varies: measured **~38 s** for Town02 headless cold on this host (`LogLoad:
  Took 32.3 seconds to LoadMap`), less warm, minutes for heavy maps / cold
  shaders.
- **Portability trap:** the bash idiom `(echo >/dev/tcp/127.0.0.1/2000)` is a
  **bash builtin, not a syscall** — under zsh (the default shell here) it fails
  every time, so a poll loop using it reports "never opened" while the server is
  happily listening. Cost a false negative on a working server. Use `nc -z`
  (present at /usr/bin/nc) or `ss -ltn | grep :2000`, which are shell-agnostic.
- **Encoded:** SKILL.md quick start.
