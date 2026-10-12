#!/usr/bin/env python3
"""One-command hybrid rendering: CARLA actors composited into a NuRec scene with occlusion from the engine's
own depth, in the recorded camera lens, under the scene's own sun, harmonized.

    hybrid_run.py --scene 7c2cf6cd --out /path/to/run [--frames 400] [--harmonizer single|temporal|both|none]
    hybrid_run.py --scene 7c2cf6cd --out /path/to/run --scenic hwy_cutin.scenic [--scenic-param ego_speed=95] [--seed 3]

Brings up what is missing (patched NuRec engine on --nre-port, CARLA server on --carla-port; a CARLA server it started is
stopped again once the capture is done, so the harmonizers get its GPU memory), then runs:
  1. illum_probe.py         sun / sky from the neural scene (rig frame)
  2. hybrid_video.py        bare proxy world, sun + sky light map from the probe, exposure solved against the neural road,
                            `rt_lens` (+ instance / distance AOVs) path tracing the recorded lens: catchers alone (pass A),
                            physics-driven synthetic cars and walkers over the catchers (pass B); engine colour+distance+opacity
  3. hybrid_video_finish.py depth-tested composite with cast shadow
  4. harmonizer             single-frame (engine's model, in the NuRec container) and/or temporal (NVIDIA/harmonizer)
  5. make_videos.sh         hybrid_final.mp4, neural_only.mp4, naive_paste.mp4, compare_*.mp4 (+ temporal variants)
Environment: NUREC_SAMPLES (scene root), NRE_CONTAINER (default nre_carla_cosmos), HARMONIZER_DIR, UE_ROOT, DLSS_SDK.
"""
import argparse, atexit, glob, json, os, shutil, signal, socket, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__)); NUREC_DIR = os.path.dirname(HERE)
CARLA_ROOT = os.path.abspath(os.path.join(NUREC_DIR, "..", "..", "..", ".."))
DEFAULT_CARS = ["ue4.chevrolet.impala:235,235,235:30:0:108", "ue4.bmw.grantourer:170,25,30:60:1:115", "ue4.audi.tt:40,80,170:95:0:112",
                "nissan.patrol:180,180,185:20:1:95", "ue4.ford.crown:235,235,235:80:opp:105", "ue4.mercedes.ccc:170,25,30:180:opp:105",
                "ue4.ford.mustang:220,190,40:280:opp:100", "dodge.charger:60,110,60:280:opp:105:4", "ue4.chevrolet.impala:180,180,185:280:opp:105:8",
                "ue4.bmw.grantourer:40,80,170:280:opp:110:12", "ue4.ford.crown:200,200,205:280:opp:105:15.5"]
# CARLA sun convention, calibrated 2026-09-07 on the live server (carla_sun_calib.py + perspective sweep):
# bearing of the direction toward the sun in CARLA world coordinates (atan2 y, x) = sun_azimuth_angle + 180 deg
SUN_CALIB = dict(fit=dict(k=1, c=180.0), note="sun bearing (CARLA world, atan2 y,x) = sun_azimuth_angle + 180")

ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
ap.add_argument("--scene", required=True, help="scene id or a unique substring of it")
ap.add_argument("--out", required=True); ap.add_argument("--frames", type=int, default=0, help="0 = whole clip")
ap.add_argument("--fps", type=float, default=20.0); ap.add_argument("--start-frac", type=float, default=0.0)
ap.add_argument("--samples", default=os.environ.get("NUREC_SAMPLES", os.path.join(CARLA_ROOT, "..", "nurec_samples", "sample_set", "26.04_release")))
ap.add_argument("--cars", action="append", default=None, help="bp:r,g,b:ahead_m:lane|opp:speed_kmh[:delay_s] (default: the run-9 set)")
ap.add_argument("--walkers", action="append", default=[], help="bp:ahead_m:side:speed_kmh[:delay_s[:mode]] (see hybrid_video.py); --cars none = no cars")
ap.add_argument("--harmonizer", default="both", choices=["single", "temporal", "both", "none"])
ap.add_argument("--timestep", type=int, default=250, help="temporal Harmonizer strength")
ap.add_argument("--carla-port", type=int, default=3000); ap.add_argument("--nre-port", type=int, default=46436)
ap.add_argument("--container", default=os.environ.get("NRE_CONTAINER", "nre_carla_cosmos"))
ap.add_argument("--harmonizer-dir", default=os.environ.get("HARMONIZER_DIR", os.path.join(CARLA_ROOT, "..", "harmonizer")))
ap.add_argument("--exposure", type=float, default=0.44)
ap.add_argument("--rt-spp", type=int, default=16, help="path-tracer samples per pixel of the rt_lens sensors")
ap.add_argument("--rt-exposure-comp", default="auto", help="rt_lens exposure_compensation (EV) or 'auto': measured per scene against the neural road before the passes (hybrid_video.py --rt-exposure-comp, exposure_calib.py); 5.0 was the raster-layer match")
ap.add_argument("--calib-frames", type=int, default=8, help="frames used by the auto exposure solve")
ap.add_argument("--skymap", type=int, default=1, help="drive the CARLA sky light with the probe panorama (0 = atmosphere only)")
ap.add_argument("--skymap-intensity", type=float, default=0.0, help="0 = skymap.DEFAULT_INTENSITY")
ap.add_argument("--skip-capture", action="store_true", help="reuse the layers already in --out (A_/B_/E_ npz): rerun only composite, harmonizers, videos")
ap.add_argument("--no-start", action="store_true", help="fail instead of starting missing servers")
ap.add_argument("--keep-world", action="store_true", help="reuse the loaded OpenDRIVE world (same scene, furniture already off)")
ap.add_argument("--scenic", default=None, help="Scenic scenario (.scenic) that drives the ego and every other agent instead of the --cars/--walkers layout: "
                "the capture step becomes scenic/hybrid_scenic.py (see scenic/README.md); a bare name resolves under scenic/")
ap.add_argument("--scenic-param", action="append", default=[], help="K=V global parameter for the Scenic scenario (repeatable)")
ap.add_argument("--seed", type=int, default=0, help="Scenic sampling seed")
a = ap.parse_args()
if a.scenic and not os.path.exists(a.scenic) and os.path.exists(os.path.join(HERE, "scenic", a.scenic)): a.scenic = os.path.join(HERE, "scenic", a.scenic)
if a.scenic and not os.path.exists(a.scenic): raise SystemExit(f"no such Scenic scenario: {a.scenic}")
a.samples = os.path.realpath(a.samples); a.harmonizer_dir = os.path.realpath(a.harmonizer_dir)     # no '..' : the paths are also used inside containers
OUT = os.path.abspath(a.out); os.makedirs(OUT, exist_ok=True); PY = sys.executable
LOG = open(os.path.join(OUT, "run.log"), "a")


def log(msg):
    line = f"[{time.strftime('%H:%M:%S')}] {msg}"; print(line, flush=True); LOG.write(line + "\n"); LOG.flush()


def sh(cmd, check=True, **kw):
    log("$ " + (cmd if isinstance(cmd, str) else " ".join(cmd)))
    r = subprocess.run(cmd, shell=isinstance(cmd, str), stdout=LOG, stderr=subprocess.STDOUT, **kw)
    if check and r.returncode != 0: raise SystemExit(f"step failed ({r.returncode}), see {OUT}/run.log")
    return r.returncode


def port_open(port):
    try:
        with socket.create_connection(("localhost", port), timeout=2): return True
    except OSError: return False


usdz = sorted(glob.glob(os.path.join(a.samples, f"*{a.scene}*", "*.usdz")))
if not usdz: raise SystemExit(f"no usdz for scene '{a.scene}' under {a.samples}")
usdz = usdz[0]; log(f"scene {a.scene}: {usdz}")

# ---------- 0. servers ----------
def ensure_engine():
    sh(["docker", "exec", a.container, "mkdir", "-p", "/tmp/hybrid/site"])
    sh(["docker", "cp", os.path.join(HERE, "engine_patch", "sitecustomize.py"), f"{a.container}:/tmp/hybrid/site/sitecustomize.py"])
    sh(["docker", "cp", os.path.join(HERE, "engine_patch", "harmonize.py"), f"{a.container}:/tmp/hybrid/harmonize.py"])
    sh(["docker", "exec", a.container, "bash", "-c",
        "head -n -1 /app/run > /tmp/hybrid/run2 && printf 'export PYTHONPATH=\"/tmp/hybrid/site:${PYTHONPATH}\"\\nexec \"${PYCENA_CC_BINARY}\" \"$@\"\\n' >> /tmp/hybrid/run2 && "
        "head -n -1 /app/run > /tmp/hybrid/runpy && printf 'exec python3 \"$@\"\\n' >> /tmp/hybrid/runpy && chmod +x /tmp/hybrid/run2 /tmp/hybrid/runpy"])
    if port_open(a.nre_port):
        log(f"engine on {a.nre_port} already up"); return
    if a.no_start: raise SystemExit(f"no engine on port {a.nre_port}")
    # stale servers inside the container keep the port (SO_REUSEPORT) -> kill them first
    sh(["docker", "exec", a.container, "bash", "-c", f"pkill -f 'serve-grpc.*--port={a.nre_port}' || true"], check=False)
    glob_ = os.path.join(a.samples, "*", "*.usdz")
    sh(["systemd-run", "--user", "--collect", "--unit", f"nre-hybrid-{a.nre_port}", "-p", f"StandardOutput=file:{OUT}/engine.log", "-p", f"StandardError=file:{OUT}/engine.log",
        "docker", "exec", "-e", "RUNFILES_DIR=/app/run.runfiles", "-w", "/app", a.container, "/tmp/hybrid/run2", "serve-grpc", "--artifact-glob", glob_,
        f"--port={a.nre_port}", "--host=localhost", "--no-test-scenes-are-valid", "--enable-editing-actors", "--no-enable-harmonizer", "--cache-size", "1"])
    for _ in range(120):
        if port_open(a.nre_port): break
        time.sleep(5)
    else: raise SystemExit("engine did not come up")
    log("engine started, waiting for the scene list")


def wait_scene():
    probe = (f"import sys, grpc; sys.path.insert(0, '{NUREC_DIR}'); from nre.grpc.protos import sensorsim_pb2_grpc as g, common_pb2 as c\n"
             f"st = g.SensorsimServiceStub(grpc.insecure_channel('localhost:{a.nre_port}'))\n"
             f"ids = list(st.get_available_scenes(c.Empty(), timeout=60).scene_ids); print(ids); sys.exit(0 if any('{a.scene}' in x for x in ids) else 1)")
    for _ in range(120):
        r = subprocess.run([PY, "-c", probe], capture_output=True, text=True)
        if r.returncode == 0: log("engine serves " + r.stdout.strip()); return
        time.sleep(5)
    raise SystemExit(f"engine on {a.nre_port} does not list scene '{a.scene}' (check --samples)")


STARTED_CARLA = False
CARLA_UNIT = f"hybrid-carla-{a.carla_port}"


def carla_unit_active():
    """True when the server on the port is one this script started (this run, or a run that died before stopping it)."""
    return subprocess.run(["systemctl", "--user", "is-active", "--quiet", CARLA_UNIT]).returncode == 0


def stop_carla_unit(why):
    if carla_unit_active():
        sh(["systemctl", "--user", "stop", CARLA_UNIT], check=False); log(f"CARLA server {CARLA_UNIT} stopped ({why})")


def ensure_carla():
    global STARTED_CARLA
    if port_open(a.carla_port):
        # a server left by a run of this script that died mid-capture is ours to stop later; a user's own server is left alone
        STARTED_CARLA = carla_unit_active()
        log(f"CARLA on {a.carla_port} already up" + (f" ({CARLA_UNIT}, left by an earlier run)" if STARTED_CARLA else "")); return
    if a.no_start: raise SystemExit(f"no CARLA server on port {a.carla_port}")
    ue = os.environ.get("UE_ROOT", os.path.join(CARLA_ROOT, "..", "UnrealEngine_5"))
    editor = os.path.join(ue, "Engine", "Binaries", "Linux", "UnrealEditor"); uproject = os.path.join(CARLA_ROOT, "Unreal", "CarlaUnreal", "CarlaUnreal.uproject")
    lock = os.path.join(CARLA_ROOT, "..", ".omc", "ue.lock"); pre = ["/usr/bin/flock", lock] if os.path.isdir(os.path.dirname(lock)) else []
    env = [f"--setenv=DLSS_SDK={os.environ.get('DLSS_SDK', os.path.expanduser('~/SDKs/DLSS'))}", f"--setenv=DISPLAY={os.environ.get('DISPLAY', ':1')}"]
    sh(["systemd-run", "--user", "--collect", "--unit", f"hybrid-carla-{a.carla_port}", "-p", f"StandardOutput=file:{OUT}/carla.log", "-p", f"StandardError=file:{OUT}/carla.log", *env, *pre,
        editor, uproject, "-game", "-RenderOffScreen", "-nosound", "-log", "-unattended", "-nosplash", f"-carla-rpc-port={a.carla_port}",
        f"-carla-streaming-port={a.carla_port + 1}", f"-carla-secondary-port={a.carla_port + 2}"])
    for _ in range(90):
        if subprocess.run([PY, "-c", f"import carla; c=carla.Client('localhost',{a.carla_port}); c.set_timeout(5.0); c.get_server_version()"], capture_output=True).returncode == 0: break
        time.sleep(5)
    else: raise SystemExit("CARLA did not come up")
    STARTED_CARLA = True; log("CARLA started")


# a server this script started must not outlive a failed run: the next run would reuse it and its harmonizers would then
# run beside the server's ~8 GB and OOM (SystemExit from a failed step, KeyboardInterrupt and SIGTERM all end up here)
signal.signal(signal.SIGTERM, lambda *_: sys.exit(143))
atexit.register(lambda: STARTED_CARLA and stop_carla_unit("run ended"))


SKIP_CAPTURE = a.skip_capture and os.path.exists(os.path.join(OUT, "meta.json"))
ensure_engine(); wait_scene()
if not SKIP_CAPTURE: ensure_carla()

# ---------- 1. illumination ----------
illum = os.path.join(OUT, "illum"); calib = os.path.join(OUT, "sun_calib.json"); json.dump(SUN_CALIB, open(calib, "w"), indent=1)
if not os.path.exists(illum + ".json"):
    sh([PY, os.path.join(HERE, "illum_probe.py"), illum, "--scene", a.scene, "--frac", "0.5", "--port", str(a.nre_port)])
il = json.load(open(illum + ".json"))
if not il.get("sun"): raise SystemExit(f"illumination probe found no sun in {a.scene} ({illum}.json lists the rejected candidates); no --sun override in hybrid_run.py")
log(f"sun in rig frame: az {il['sun']['az_deg']:.1f} el {il['sun']['el_deg']:.1f} ({il['sun']['method']}, confidence {il['sun']['confidence']:.2f}, "
    f"{len(il.get('sun_candidates', []))} candidates)")

# ---------- 2. capture ----------
common = ["--port", str(a.carla_port), "--nre-port", str(a.nre_port), "--fps", str(a.fps), "--sun", f"auto:{illum}.json:{calib}", "--exposure", str(a.exposure),
          "--furniture", "0", "--rt-spp", str(a.rt_spp), "--rt-exposure-comp", str(a.rt_exposure_comp), "--calib-frames", str(a.calib_frames),
          "--skymap", str(a.skymap), "--skymap-intensity", str(a.skymap_intensity)]
if a.frames: common += ["--frames", str(a.frames)]
if a.keep_world: common += ["--keep-world"]
if a.scenic:
    # Scenic drives the ego (with the recorded camera on it) and every other agent; -O strips the parser asserts of Scenic's OpenDRIVE reader
    cmd = [PY, "-u", "-O", os.path.join(HERE, "scenic", "hybrid_scenic.py"), OUT, "--scene", a.scene, "--usdz", usdz, "--scenario", os.path.abspath(a.scenic), "--seed", str(a.seed)] + common
    for kv in a.scenic_param: cmd += ["--param", kv]
else:
    cmd = [PY, "-u", os.path.join(HERE, "hybrid_video.py"), OUT, "--scene", a.scene, "--usdz", usdz, "--drive", "physics", "--recorded", "none", "--start-frac", str(a.start_frac)] + common
    for c in (a.cars or DEFAULT_CARS):
        if c != "none": cmd += ["--cars", c]
    for w in a.walkers: cmd += ["--walkers", w]
if SKIP_CAPTURE:
    m = json.load(open(os.path.join(OUT, "meta.json")))
    log(f"capture skipped (--skip-capture): {m['frames']} frames, exposure {m['rt']['exposure_comp']:+.2f} EV ({m['rt']['exposure_mode']})")
else:
    sh(cmd)
    if STARTED_CARLA and a.harmonizer != "none":
        # the capture is done and the harmonizers need the GPU memory the server holds (~9 GB; the temporal model OOMs beside
        # it and the two engines on 32 GB): stop the server this script started, leave one the user started alone
        stop_carla_unit("capture done, GPU freed for the harmonizers"); STARTED_CARLA = False

# ---------- 3. composite ----------
sh([PY, os.path.join(HERE, "hybrid_video_finish.py"), OUT])
F = os.path.join(OUT, "frames")

# ---------- 4. harmonizers ----------
if a.harmonizer in ("single", "both"):
    sh(["docker", "exec", a.container, "rm", "-rf", "/tmp/hybrid/frames"]); sh(["docker", "cp", F, f"{a.container}:/tmp/hybrid/frames"])
    sh(["docker", "exec", "-e", "RUNFILES_DIR=/app/run.runfiles", "-w", "/app", a.container, "/tmp/hybrid/runpy", "/tmp/hybrid/harmonize.py", "/tmp/hybrid/frames", "--res=1088x1920"])
    sh(["docker", "cp", f"{a.container}:/tmp/hybrid/frames/.", F + "/"])
if a.harmonizer in ("temporal", "both"):
    H = os.path.abspath(a.harmonizer_dir); tag = os.path.basename(OUT.rstrip("/")); tin = os.path.join(H, f"_in_{tag}")
    if os.path.isdir(tin): shutil.rmtree(tin, ignore_errors=True)
    os.makedirs(tin, exist_ok=True)
    for p in sorted(glob.glob(os.path.join(F, "comp_*.png"))): os.link(p, os.path.join(tin, os.path.basename(p))) if os.stat(p).st_dev == os.stat(tin).st_dev else shutil.copy(p, tin)
    sh(["docker", "run", "--rm", "--gpus", "all", "--ipc=host", "-v", f"{H}:/work", "-v", f"{H}/text2image_patched.py:/usr/local/lib/python3.12/dist-packages/cosmos_predict2/pipelines/text2image.py:ro",
        "-w", "/work/src", "--entrypoint", "python3", "harmonizer-cosmos-env", "inference_pix2pix_turbo_harmonizer.py", "--input_image", f"/work/_in_{tag}",
        "--model_path", "/work/models/diffusion_harmonizer.pkl", "--model_identifier", "temporal", "--timestep", str(a.timestep), "--resolution", "1360", "--use_sched"])
    for p in sorted(glob.glob(os.path.join(f"{tin}_temporal", "comp_*.png"))): shutil.copy(p, os.path.join(F, "temporal_" + os.path.basename(p)[5:]))
    shutil.rmtree(tin, ignore_errors=True)

# ---------- 5. videos ----------
if a.harmonizer in ("single", "both"): sh(["bash", os.path.join(HERE, "make_videos.sh"), OUT, str(int(a.fps))])
if a.harmonizer in ("temporal", "both"):
    sh(f"ffmpeg -y -loglevel error -framerate {int(a.fps)} -i {F}/temporal_%04d.png -c:v libx264 -pix_fmt yuv420p -crf 18 {OUT}/hybrid_temporal.mp4")
    sh(f"ffmpeg -y -loglevel error -framerate {int(a.fps)} -i {F}/neural_%04d.png -framerate {int(a.fps)} -i {F}/temporal_%04d.png -filter_complex "
       f"\"[0:v]scale=960:540,drawtext=text='neural scene':x=10:y=10:fontsize=24:fontcolor=yellow[a];[1:v]scale=960:540,drawtext=text='hybrid + temporal Harmonizer':x=10:y=10:fontsize=24:fontcolor=yellow[b];[a][b]hstack\" "
       f"-c:v libx264 -pix_fmt yuv420p -crf 18 {OUT}/compare_neural_vs_temporal.mp4")
if a.harmonizer == "none":
    sh(f"ffmpeg -y -loglevel error -framerate {int(a.fps)} -i {F}/comp_%04d.png -c:v libx264 -pix_fmt yuv420p -crf 18 {OUT}/hybrid_composite.mp4")
log("done: " + ", ".join(sorted(os.path.basename(p) for p in glob.glob(os.path.join(OUT, "*.mp4")))))
