"""Full-clip hybrid run: ego follows the recorded trajectory, synthetic cars drive on the proxy road, every
frame = engine colour+distance+opacity at the camera pose + CARLA layer -> depth-tested composite with cast
shadow (hybrid_video_finish.py). The CARLA layer is path traced by `sensor.camera.rt_lens` in the recorded lens
(camera_model=ftheta): pass A shows the shadow catchers alone, pass B the synthetic actors (cars, walkers) over
the catchers with their instance and distance from the path tracer's own AOVs (`rt_lens_instance`,
`rt_lens_distance`); the sun comes from the illumination probe and the sky light from the probe panorama.

usage: hybrid_video.py OUT_DIR --scene 7c2cf6cd --usdz PATH [--fps 20] [--sun 280,40,10 | --sun auto:illum.json:sun_calib.json]
       [--cars "lincoln.mkz:220,220,225:15:0" ...]  (bp:color:ahead_m:lane_offset  lane_offset: 0 = ego lane,
        +1 = one lane left, -1 = one lane right, "opp" = opposite carriageway via left lanes)
       [--keep-world] [--frames N] [--start-frac 0.0] [--tm-speed 0]
Every exit (done, --calib-only, SystemExit, uncaught error, Ctrl-C) passes through teardown(): actors destroyed,
sky light map cleared, cvars and asynchronous mode restored.
"""
import argparse, atexit, json, os, sys, time, zipfile, queue, socket, struct
import numpy as np
import grpc
from scipy.spatial.transform import Rotation as Rot, Slerp

HERE = os.path.dirname(os.path.abspath(__file__)); NUREC_DIR = os.path.dirname(HERE)
sys.path.insert(0, NUREC_DIR); sys.path.insert(0, HERE)
import carla  # noqa: E402
import msgpack  # noqa: E402
from utils import mat_to_carla_transform  # noqa: E402
from projection_functions import get_t_rig_enu_from_ecef  # noqa: E402
from nre.grpc.protos import sensorsim_pb2 as s, sensorsim_pb2_grpc as g, common_pb2 as c  # noqa: E402

OPTICAL_TO_FLU = np.array([[0.0, 0.0, 1.0], [-1.0, 0.0, 0.0], [0.0, -1.0, 0.0]])
OPTICAL_BASIS = np.eye(4); OPTICAL_BASIS[:3, :3] = OPTICAL_TO_FLU

ap = argparse.ArgumentParser()
ap.add_argument("out"); ap.add_argument("--scene", required=True); ap.add_argument("--usdz", required=True)
ap.add_argument("--port", type=int, default=3000); ap.add_argument("--nre-port", type=int, default=46436)
ap.add_argument("--fps", type=float, default=20.0); ap.add_argument("--frames", type=int, default=0)
ap.add_argument("--start-frac", type=float, default=0.0)
ap.add_argument("--camera", default="camera_front_wide_120fov")
ap.add_argument("--sun", default="280,40,10", help="CARLA azimuth,altitude,cloudiness or auto:<illum_probe json>:<sun_calib json>")
ap.add_argument("--exposure", type=float, default=0.44, help="r.EyeAdaptation.LensAttenuation for the raster path (the rt_lens sensors use --rt-exposure-comp)")
ap.add_argument("--cars", action="append", default=[])
ap.add_argument("--walkers", action="append", default=[], help="bp:ahead_m:side:speed_kmh[:delay_s[:mode]]  bp = walker.pedestrian suffix (e.g. 0050) or 'random'; "
                "side = R (right shoulder), L (left shoulder / median) or a lateral offset in m from the ego lane (+ = right); mode = along (default) | back | cross")
ap.add_argument("--settle-ticks", type=int, default=20, help="ticks of physics before the capture so spawned cars (+0.3 m) and walkers (+1 m) rest on the road; late starters are settled one by one and re-parked")
ap.add_argument("--keep-world", action="store_true"); ap.add_argument("--tm-speed", type=float, default=0.0, help="TM global speed difference %% (negative = faster)")
ap.add_argument("--tm-port", type=int, default=8000)
ap.add_argument("--skip-a", action="store_true", help="reuse existing A_*.npz")
ap.add_argument("--ego", action="store_true", help="spawn a visible ego proxy vehicle (off: none)")
ap.add_argument("--drive", default="physics", choices=["kinematic", "tm", "physics"], help="kinematic = follow lane waypoints at the given speed (physics off, wheels do not spin); physics = lane follower with throttle/steer control (wheels spin); tm = Traffic Manager autopilot")
ap.add_argument("--recorded", default="none", choices=["none", "nearest", "interp"], help="recorded NuRec actors: none = parked out of view (hybrid mode), interp = played back with pose interpolation, nearest = nearest recorded sample")
ap.add_argument("--skip-e", action="store_true", help="reuse existing E_*.npz")
ap.add_argument("--furniture", type=int, default=0, help="carla.OpenDrive.StreetFurniture for the generated proxy world (0 = bare road)")
ap.add_argument("--rt-spp", type=int, default=16, help="path-tracer samples per pixel (the sensor clamps to r.PathTracing.MaxFramePassCount, 64 on the hybrid server; noise on car px 8: 5.7, 16: 5.0, 32: 4.8, 64: 3.7 at 54/54/74/116 ms per tick)")
ap.add_argument("--rt-exposure-comp", default="auto", help="exposure_compensation (EV) of the rt_lens sensor under exposure_mode=manual, or 'auto' = solve it per scene before the passes so the "
                "proxy road matches the neural road luminance (exposure_calib.py; solved value + residual go to meta.json). 5.0 was the raster-layer match at --exposure 0.44, not a neural match")
ap.add_argument("--calib-frames", type=int, default=8, help="auto exposure: frames (spread over the clip) rendered by both sides for the solve")
ap.add_argument("--calib-start", type=float, default=5.0, help="auto exposure: EV the solve starts from (also the value measured first)")
ap.add_argument("--calib-tol", type=float, default=0.05, help="auto exposure: stop when |median road log2 ratio| is below this (EV)")
ap.add_argument("--calib-only", action="store_true", help="run the exposure measurement/solve (exposure_calib.json + .jpg in OUT) and exit without capturing")
ap.add_argument("--catcher-tags", default="Roads,Sidewalks,RoadLines", help="semantic labels that receive the cars' cast shadow")
ap.add_argument("--skymap", type=int, default=1, help="light the CARLA scene with the probe panorama through the sky light (needs --sun auto:...)")
ap.add_argument("--skymap-intensity", type=float, default=0.0, help="sky light intensity for the panorama (0 = skymap.DEFAULT_INTENSITY)")
a = ap.parse_args()
a.rt_exposure_auto = str(a.rt_exposure_comp).lower() == "auto"
a.rt_exposure_comp = a.calib_start if a.rt_exposure_auto else float(a.rt_exposure_comp)     # the solve below replaces it in auto mode
CALIB = None
os.makedirs(a.out, exist_ok=True)


def fail(msg): raise SystemExit("hybrid_video: " + msg)


# ---------------- engine: trajectory, cameras, dynamic objects ----------------
st = g.SensorsimServiceStub(grpc.insecure_channel(f"localhost:{a.nre_port}", options=[("grpc.max_receive_message_length", 256 << 20)]))
try:
    scenes = list(st.get_available_scenes(c.Empty(), timeout=60).scene_ids)
except grpc.RpcError as e: fail(f"no NuRec engine on port {a.nre_port} ({e.code().name}): start it via hybrid_run.py")
sid = next((x for x in scenes if a.scene in x), None)
if sid is None: fail(f"engine on port {a.nre_port} does not serve scene '{a.scene}' ({scenes})")
cam = {k.logical_id: k for k in st.get_available_cameras(s.AvailableCamerasRequest(scene_id=sid), timeout=900).available_cameras}[a.camera]


def M(pose):
    q = pose.quat; T = np.eye(4); T[:3, :3] = Rot.from_quat([q.x, q.y, q.z, q.w]).as_matrix()
    T[:3, 3] = [pose.vec.x, pose.vec.y, pose.vec.z]; return T


def P(T):
    q = Rot.from_matrix(T[:3, :3]).as_quat()
    return c.Pose(vec=c.Vec3(x=T[0, 3], y=T[1, 3], z=T[2, 3]), quat=c.Quat(w=q[3], x=q[0], y=q[1], z=q[2]))


traj = st.get_available_trajectories(s.AvailableTrajectoriesRequest(scene_id=sid), timeout=120).available_trajectories[0].trajectory.poses
tt = np.array([p.timestamp_us for p in traj], dtype=np.int64)
Ts = [M(p.pose) for p in traj]
R2C = M(cam.rig_to_camera)
rots = Rot.from_matrix(np.array([T[:3, :3] for T in Ts])); slerp = Slerp(tt.astype(np.float64), rots)
pos = np.array([T[:3, 3] for T in Ts])


def rig_at(t_us):
    t_us = int(np.clip(t_us, tt[0], tt[-1]))
    T = np.eye(4); T[:3, :3] = slerp(float(t_us)).as_matrix()
    T[:3, 3] = np.array([np.interp(t_us, tt, pos[:, i]) for i in range(3)]); return T


dyn = st.get_dynamic_objects(s.AvailableDynamicObjectsRequest(scene_id=sid), timeout=120).dynamic_objects
dyn_t = [(d.id, np.array([p.timestamp_us for p in d.trajectory.poses], dtype=np.int64), list(d.trajectory.poses)) for d in dyn if d.trajectory.poses]


def objs_at(t_us):
    out = []
    for oid, ts, ps in dyn_t:
        if a.recorded == "none":
            T = M(ps[0].pose); T[2, 3] -= 200.0                       # parked far underground: not rendered
            out.append(s.DynamicObject(track_id=oid, pose_pair=s.PosePair(start_pose=P(T), end_pose=P(T)))); continue
        if not (ts[0] <= t_us <= ts[-1]): continue
        if a.recorded == "nearest":
            k = int(np.argmin(np.abs(ts - t_us))); pose = ps[k].pose
        else:
            k = int(np.searchsorted(ts, t_us)) - 1; k = max(0, min(k, len(ps) - 2)); al = (t_us - ts[k]) / max(ts[k + 1] - ts[k], 1)
            Ta, Tb = M(ps[k].pose), M(ps[k + 1].pose); T = np.eye(4); T[:3, 3] = Ta[:3, 3] * (1 - al) + Tb[:3, 3] * al
            T[:3, :3] = Rot.from_matrix(np.stack([Ta[:3, :3], Tb[:3, :3]])).mean(weights=[1 - al, al]).as_matrix() if 0 < al < 1 else (Tb if al >= 1 else Ta)[:3, :3]
            pose = P(T)
        out.append(s.DynamicObject(track_id=oid, pose_pair=s.PosePair(start_pose=pose, end_pose=pose)))
    return out


# the recorded lens: output resolution and polynomial straight from the scene's camera spec
fp = cam.intrinsics.ftheta_param
a.w, a.h = int(cam.intrinsics.resolution_w), int(cam.intrinsics.resolution_h)
spec = s.CameraSpec(logical_id=cam.logical_id, trajectory_idx=cam.trajectory_idx, resolution_w=a.w, resolution_h=a.h, shutter_type=s.GLOBAL, ftheta_param=fp)
FT = dict(cx=float(fp.principal_point_x), cy=float(fp.principal_point_y), poly=[float(x) for x in fp.pixeldist_to_angle_poly], max_angle=float(fp.max_angle))
print("f-theta lens:", FT, f"{a.w}x{a.h}", flush=True)


def rt_lens_attributes():
    """The recorded f-theta lens on sensor.camera.rt_lens: camera_model=ftheta takes NuRec's pixel-distance -> angle polynomial
    verbatim (r in pixels with fx = 1/W), no inversion, no table."""
    return dict(camera_model="ftheta", distortion_coeffs=",".join("%.12g" % c_ for c_ in FT["poly"]), theta_max_deg="%.6f" % np.degrees(FT["max_angle"]),
                fx="%.10f" % (1.0 / a.w), fy="%.10f" % (1.0 / a.h), cx="%.10f" % (FT["cx"] / a.w), cy="%.10f" % (FT["cy"] / a.h),
                image_size_x=str(a.w), image_size_y=str(a.h), samples_per_pixel=str(a.rt_spp), enable_denoiser="true",
                exposure_mode="manual", exposure_compensation="%.4f" % a.rt_exposure_comp)


RTLENS = rt_lens_attributes()


def render_engine(Tcam, t_us):
    req = s.RGBRenderRequest(scene_id=sid, resolution_h=a.h, resolution_w=a.w, camera_intrinsics=spec, frame_start_us=int(t_us), frame_end_us=int(t_us) + 1,
                             sensor_pose=s.PosePair(start_pose=P(Tcam), end_pose=P(Tcam)), dynamic_objects=objs_at(t_us), image_format=s.RGB_UINT8_PLANAR, image_quality=-7.0)
    try: b = st.render_rgb(req, timeout=600).image_bytes
    except grpc.RpcError as e: fail(f"engine render_rgb failed at t={int(t_us)} us: {e.code().name} {e.details()}")
    if b[:4] != b"HYBR": fail(f"engine on port {a.nre_port} is not the patched serve-grpc (no HYBR payload for image_quality -7): start it via hybrid_run.py")
    h, w = struct.unpack("<II", b[4:12]); o = 12
    rgb = np.frombuffer(b[o:o + 3 * h * w], dtype=np.uint8).reshape(3, h, w).transpose(1, 2, 0); o += 3 * h * w
    dist = np.frombuffer(b[o:o + 4 * h * w], dtype="<f4").reshape(h, w); o += 4 * h * w
    opa = np.frombuffer(b[o:o + 4 * h * w], dtype="<f4").reshape(h, w)
    return rgb.copy(), dist.copy(), opa.copy()


# frame times
t0 = tt[0] + int(a.start_frac * (tt[-1] - tt[0])); dt_us = int(1e6 / a.fps)
n_frames = a.frames if a.frames > 0 else int((tt[-1] - t0) // dt_us)
frame_t = [int(t0 + k * dt_us) for k in range(n_frames)]
print(f"scene {sid}: {n_frames} frames at {a.fps} fps", flush=True)

# ---------------- CARLA proxy world ----------------
with zipfile.ZipFile(a.usdz) as z:
    xodr = z.read("map.xodr").decode(); rig = json.loads(z.read("rig_trajectories.json"))
t_sc = get_t_rig_enu_from_ecef(np.array(rig["T_world_base"]), xodr)
client = carla.Client("localhost", a.port); client.set_timeout(600.0)
try: world = client.get_world()
except RuntimeError as e: fail(f"no CARLA server on port {a.port} ({e})")
if not (a.keep_world and world.get_map().name.endswith("OpenDriveMap")):
    sk0 = socket.create_connection(("localhost", a.port), timeout=10)      # street furniture on/off before the world is generated
    sk0.sendall(msgpack.packb([0, 1, "console_command", [[False], f"carla.OpenDrive.StreetFurniture {a.furniture}"]], use_bin_type=True)); sk0.settimeout(10); sk0.recv(1 << 16); sk0.close()
    print(f"generating OpenDRIVE world (StreetFurniture={a.furniture}) ...", flush=True)
    client.generate_opendrive_world(xodr, carla.OpendriveGenerationParameters(vertex_distance=2.0, max_road_length=500.0, wall_height=0.0, additional_width=0.6,
                                                                              smooth_junctions=True, enable_mesh_visibility=True))
    world = client.get_world()
cmap = world.get_map()
settings = world.get_settings(); settings.synchronous_mode = True; settings.fixed_delta_seconds = 1.0 / a.fps; world.apply_settings(settings)


def console(cmd):
    sk = socket.create_connection(("localhost", a.port), timeout=10)
    sk.sendall(msgpack.packb([0, 1, "console_command", [[False], cmd]], use_bin_type=True)); sk.settimeout(10); sk.recv(1 << 16); sk.close()


actors = []; ego = None; kin = []; phys = []; walkers = []; tm = None; SKYMAP = False; TORN_DOWN = False


def teardown():
    """Restore the server for the next client: cvars, sky light map, asynchronous mode, actors. Registered with atexit right
    after synchronous mode is switched on, so every exit path (including SystemExit from fail() and uncaught exceptions) runs
    it once; each step is independent so a dead server does not hide the original error."""
    global TORN_DOWN
    if TORN_DOWN: return
    TORN_DOWN = True
    steps = [("cvars", lambda: (console("r.EyeAdaptation.MethodOverride -1"), console("r.EyeAdaptation.LensAttenuation 0.78"), console("r.MotionBlur.Amount 0.5"))),
             ("sky light map", lambda: world.clear_sky_light_map() if SKYMAP else None),
             ("autopilot off", lambda: [v.set_autopilot(False, a.tm_port) for v in actors if isinstance(v, carla.Vehicle)]),
             ("asynchronous mode", lambda: (world.tick(), setattr(settings, "synchronous_mode", False), setattr(settings, "fixed_delta_seconds", None), world.apply_settings(settings))),
             ("TM asynchronous", lambda: tm.set_synchronous_mode(False) if tm is not None else None),
             ("destroy actors", lambda: client.apply_batch([carla.command.DestroyActor(x.id) for x in actors]) if actors else None)]
    for name, step in steps:
        try: step()
        except Exception as e: print(f"teardown: {name} failed ({e})", flush=True)


atexit.register(teardown)

a.sun_illum_json = None
if a.sun.startswith("auto:"):
    # auto:<illum_probe json>:<carla_sun_calib json>  -> sun direction measured in the neural scene, mapped into CARLA's convention
    illum_json, calib_json = a.sun[5:].split(":")
    il = json.load(open(illum_json)); cb = json.load(open(calib_json))["fit"]; a.sun_illum_json = illum_json
    if not il.get("sun"): fail(f"{illum_json} has no sun (probe found no candidate above the skyline and no sky gradient); pass --sun az,alt,cloud")
    d_rig = np.array(il["sun"]["dir_rig"]); Rw = (t_sc @ np.array(il["rig_pose"]))[:3, :3]
    d_scn = Rw @ d_rig; d_carla = np.array([d_scn[0], -d_scn[1], d_scn[2]])          # scenario ENU (right-handed) -> CARLA (y flipped)
    bearing = np.degrees(np.arctan2(d_carla[1], d_carla[0]))
    az = ((bearing - cb["c"]) / cb["k"]) % 360.0; alt = float(np.degrees(np.arcsin(np.clip(d_carla[2], -1, 1))))
    cloud = float(np.clip(2.0 * (il["sky_lum_std"] - 20.0), 0.0, 80.0))
    print(f"sun auto: rig az {il['sun']['az_deg']:.1f} el {il['sun']['el_deg']:.1f} ({il['sun'].get('method', '?')}, confidence {il['sun'].get('confidence', float('nan')):.2f}) "
          f"-> CARLA azimuth {az:.1f} altitude {alt:.1f} cloudiness {cloud:.0f}", flush=True)
    a.sun = f"{az:.2f},{alt:.2f},{cloud:.1f}"
az, alt, cloud = [float(x) for x in a.sun.split(",")]
world.set_weather(carla.WeatherParameters(cloudiness=cloud, sun_azimuth_angle=az, sun_altitude_angle=alt))

# sky light from the probe panorama (ambient + reflections); the sun stays a directional light
if a.skymap and a.sun_illum_json is not None and hasattr(world, "set_sky_light_map"):
    import skymap  # noqa: E402
    pano_rig, il_meta = skymap.load_probe(os.path.splitext(a.sun_illum_json)[0] + ".jpg", a.sun_illum_json)
    M_rig = skymap.rig_to_carla_matrix(rig_pose=t_sc @ np.array(il_meta["rig_pose"]))     # same scene->CARLA mapping as the sun above
    pano_carla = skymap.rig_pano_to_carla(pano_rig, M_rig)
    skymap.apply(world, pano_carla, a.skymap_intensity or skymap.DEFAULT_INTENSITY, 512); SKYMAP = True
    print(f"sky light map set from {a.sun_illum_json}: {pano_carla.shape[1]}x{pano_carla.shape[0]}, intensity {a.skymap_intensity or skymap.DEFAULT_INTENSITY:.0f}", flush=True)
elif hasattr(world, "has_sky_light_map") and world.has_sky_light_map():
    world.clear_sky_light_map(); print("stale sky light map cleared", flush=True)

console("r.EyeAdaptation.MethodOverride 3"); console(f"r.EyeAdaptation.LensAttenuation {a.exposure}")
# the engine renders each pose sharp; motion blur on the CARLA layer (camera at highway speed) smears the sky into every silhouette
console("r.MotionBlur.Amount 0")
bpl = world.get_blueprint_library()


def cam_tf_at(t_us):
    Tcam = rig_at(t_us) @ R2C
    return mat_to_carla_transform(t_sc @ Tcam @ np.linalg.inv(OPTICAL_BASIS)), Tcam


def spawn_cam(kind, tf, attrs):
    try: bp = bpl.find(kind)
    except (IndexError, KeyError, RuntimeError): fail(f"no blueprint {kind}: the CARLA server predates the hybrid sensors (rebuild the server from this tree)")
    for k, val in attrs.items():
        if not bp.has_attribute(k): fail(f"{kind} has no attribute '{k}': the CARLA server predates the hybrid sensors (rebuild the server from this tree)")
        bp.set_attribute(k, val)
    if bp.has_attribute("enable_dlss"): bp.set_attribute("enable_dlss", "false")
    q = queue.Queue(); cm = world.spawn_actor(bp, tf); cm.listen(q.put); return cm, q


def rt_cams(tf0, actor_ids, with_actors):
    """Pass A: rt_lens showing the catchers alone. Pass B: rt_lens colour of the actors over the catchers, plus the path tracer's own
    primary-hit AOVs in the same lens: tag (R = label, G/B = id16) and Euclidean distance (float32 m)."""
    if not with_actors:
        return {"catch": spawn_cam("sensor.camera.rt_lens", tf0, dict(RTLENS, show_only_tags=a.catcher_tags))}
    aux = dict(RTLENS, show_only_actor_ids=",".join(str(i) for i in actor_ids), shadow_catcher_tags=a.catcher_tags)
    return {"rgb": spawn_cam("sensor.camera.rt_lens", tf0, aux), "instance_segmentation": spawn_cam("sensor.camera.rt_lens_instance", tf0, aux),
            "depth": spawn_cam("sensor.camera.rt_lens_distance", tf0, aux)}


def arr(img): return np.frombuffer(img.raw_data, dtype=np.uint8).reshape((img.height, img.width, 4)).copy()   # copy: the buffer dies with the Image


def dist_arr(img):
    """carla.DistanceImage (sensor.camera.rt_lens_distance): float32 metres per pixel; misses come back at 1e5 m (far plane)."""
    return np.frombuffer(img.raw_data, dtype=np.float32).reshape((img.height, img.width)).copy()


def capture(cams):
    """Latest frame of every sensor (None when nothing arrived within 30 s)."""
    fr = {}
    for k, (cm, q) in cams.items():
        try:
            img = q.get(timeout=30)
            while not q.empty(): img = q.get_nowait()
            fr[k] = img
        except queue.Empty:
            fr[k] = None
    return fr


def step_walkers(t_us):
    """Kinematic walkers: WalkerControl every tick (direction follows the road for along/back, fixed for cross)."""
    for wk in walkers:
        tsec = (t_us - frame_t[0]) / 1e6 - wk["delay"]
        w_ = wk["actor"]
        if tsec < 0 or wk["done"]:
            w_.apply_control(carla.WalkerControl(speed=0.0)); continue
        loc = w_.get_location()
        if wk["mode"] == "cross":
            d = wk["dir"]
            if (loc - wk["start"]).distance(carla.Location()) > wk["cross_m"]: wk["done"] = True
        else:
            wq = cmap.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Any)
            d = wq.transform.get_forward_vector() if wq is not None else wk["dir"]
            if wk["mode"] == "back": d = carla.Vector3D(-d.x, -d.y, 0.0)
        d = carla.Vector3D(d.x, d.y, 0.0); wk["dir"] = d
        w_.apply_control(carla.WalkerControl(direction=d, speed=wk["speed"]))


SYN_TAGS = (12, 13) + tuple(range(14, 20))      # Pedestrian, Rider, vehicles (crp::CityObjectLabel)
WARMUP = {}


def actor_ids_in(inst):
    """Set of actor ids (GetUniqueID) the tag AOV labels as synthetic actors."""
    syn = np.isin(inst[:, :, 2], SYN_TAGS)
    return frozenset((inst[:, :, 1].astype(np.int32) | (inst[:, :, 0].astype(np.int32) << 8))[syn].tolist())


def warm_up(cams, tag, with_cars):
    """Tick until the new sensors render: ray-tracing PSOs compile on the first ticks after a spawn and the affected materials come
    back black / far-plane / untagged meanwhile (2026-09-08, 3 fixed ticks: the walkers and most car pixels were missing from
    pass-B frames 0-1 in colour, tag and distance alike, present from frame 2). Pass A: a lit catchers frame after >= 3 ticks.
    Pass B: the set of actor ids the tag AOV labels unchanged for 3 consecutive lit ticks, >= 6 ticks in all (actors out of view stay
    absent consistently). The tick count and the ids go to run.log and meta.json."""
    hist = []
    for n in range(1, 61):
        world.tick(); fr = capture(cams)
        lit = all(v is not None for v in fr.values()) and (arr(fr["rgb" if with_cars else "catch"])[:, :, :3].max(-1) > 8).mean() >= 0.02
        if not with_cars:
            if lit and n >= 3: break
            continue
        hist.append(actor_ids_in(arr(fr["instance_segmentation"])) if lit else None)
        if n >= 6 and len(hist) >= 3 and hist[-1] is not None and all(h == hist[-1] for h in hist[-3:]): break
    else: fail(f"pass {tag}: the rt_lens sensors delivered no stable rendered frame in 60 warm-up ticks (see {a.out}/carla.log)")
    WARMUP[tag] = dict(ticks=n, actor_ids=sorted(hist[-1]) if with_cars else None)
    print(f"  pass {tag}: sensors warmed up after {n} ticks" + (f", actor ids in view {sorted(hist[-1])}" if with_cars else ""), flush=True)


def run_pass(tag, with_cars, actor_ids=()):
    tf0, _ = cam_tf_at(frame_t[0])
    cams = rt_cams(tf0, actor_ids, with_cars)
    world.get_spectator().set_transform(tf0)
    warm_up(cams, tag, with_cars)
    t_start = time.time()
    for k, t_us in enumerate(frame_t):
        tf, Tcam = cam_tf_at(t_us)
        for cm, _ in cams.values(): cm.set_transform(tf)
        if with_cars and ego is not None:
            ego.set_transform(mat_to_carla_transform(t_sc @ rig_at(t_us)))
        if with_cars:
            for pc in phys:
                tsec = (t_us - frame_t[0]) / 1e6 - pc["delay"]
                if tsec < 0: continue
                v_ = pc["actor"]
                if not pc["started"]:
                    if pc["delay"] > 0: v_.set_transform(pc["settled"]); v_.set_simulate_physics(True)      # teleport to the settled pose, no drop
                    fv = pc["settled"].get_forward_vector(); v_.set_target_velocity(carla.Vector3D(fv.x * pc["speed"], fv.y * pc["speed"], 0.0)); pc["started"] = True; continue
                tf_ = v_.get_transform(); loc = tf_.location; vel = v_.get_velocity(); spd = float(np.hypot(vel.x, vel.y))
                j0 = pc["idx"]; path = pc["path"]
                j = min(range(j0, min(j0 + 40, len(path))), key=lambda q: path[q].transform.location.distance(loc)); pc["idx"] = j
                if j + 2 >= len(path):
                    v_.set_simulate_physics(False); v_.set_transform(carla.Transform(carla.Location(loc.x, loc.y, loc.z - 500.0), tf_.rotation)); pc["path"] = path[:j + 1]; continue
                look = int(max(6.0, 0.45 * spd)); tgt = path[min(j + look, len(path) - 1)].transform.location
                yaw = np.radians(tf_.rotation.yaw); ang = np.arctan2(tgt.y - loc.y, tgt.x - loc.x) - yaw; ang = (ang + np.pi) % (2 * np.pi) - np.pi
                err = pc["speed"] - spd
                v_.apply_control(carla.VehicleControl(throttle=float(np.clip(0.55 + 0.12 * err, 0.0, 1.0)), steer=float(np.clip(ang * 1.5, -1.0, 1.0)), brake=float(np.clip(-0.15 * err, 0.0, 1.0)) if err < -1.5 else 0.0))
            for kc in kin:
                s_m = kc["speed"] * ((t_us - frame_t[0]) / 1e6 - kc["delay"])
                if s_m < 0: continue                      # still parked underground
                i = int(s_m); fr_ = s_m - i
                if i + 1 >= len(kc["path"]):
                    p_ = kc["path"][-1].transform; kc["actor"].set_transform(carla.Transform(carla.Location(p_.location.x, p_.location.y, p_.location.z - 500.0), p_.rotation)); continue
                p0, p1 = kc["path"][i].transform, kc["path"][i + 1].transform
                loc = carla.Location(p0.location.x + (p1.location.x - p0.location.x) * fr_, p0.location.y + (p1.location.y - p0.location.y) * fr_,
                                     p0.location.z + (p1.location.z - p0.location.z) * fr_ + kc["dz"])
                kc["actor"].set_transform(carla.Transform(loc, p0.rotation))
            step_walkers(t_us)
        world.tick()
        fr = capture(cams)
        if with_cars and k < 6 and (phys or walkers):
            print(f"  z@{k}: cars " + " ".join(f"{pc['actor'].get_location().z:.3f}" for pc in phys if pc["started"]) + " | walkers " + " ".join(f"{wk['actor'].get_location().z:.3f}" for wk in walkers), flush=True)
        # in synchronous mode the path-traced lens sensors deliver the render of THIS tick (blocking batched readback, same path as
        # the pinhole cameras; carla.RTLens.SyncModeBlockingReadback), so colour, tag and distance are one frame and none may be missing
        missing = [name for name, img in fr.items() if img is None]
        if missing: fail(f"pass {tag} frame {k}: no frame from sensor(s) {missing} within 30 s (see the server log; a Vulkan device loss shows there)")
        if not with_cars:
            # catchers-only colour = the "no actors" reference for the cast-shadow ratio (finish reuses B's labels/distance)
            np.savez(f"{a.out}/A_{k:04d}.npz", rgb=arr(fr["catch"])[:, :, 2::-1].copy(), T_cam=Tcam, t_us=t_us)
        else:
            inst = arr(fr["instance_segmentation"])
            sem_ = inst[:, :, 2].copy(); inst_ = (inst[:, :, 1].astype(np.uint32) + inst[:, :, 0].astype(np.uint32) * 256)
            np.savez(f"{a.out}/B_{k:04d}.npz", rgb=arr(fr["rgb"])[:, :, 2::-1].copy(), sem=sem_, inst=inst_, dist=dist_arr(fr["depth"]), T_cam=Tcam, t_us=t_us, cam_tf=str(tf))
            if k < 8: print(f"  tag@{k}: {int(np.isin(sem_, SYN_TAGS).sum())} actor px, ids {sorted(actor_ids_in(inst))}", flush=True)     # a late dropout shows here
        if k % 50 == 0: print(f"  {tag} frame {k}/{n_frames} {time.time() - t_start:.0f}s", flush=True)
    for cm, _ in cams.values(): cm.stop()
    world.tick()
    client.apply_batch([carla.command.DestroyActor(cm.id) for cm, _ in cams.values()]); world.tick()


# exposure of the path-traced layer: one EV measured against the neural scene on the road (exposure_calib.py), before any pass
if a.rt_exposure_auto or a.calib_only:
    import exposure_calib
    ec, CALIB = exposure_calib.solve(world, spawn_cam, capture, arr, dist_arr, cam_tf_at, render_engine, frame_t, RTLENS, a.catcher_tags, ec0=a.rt_exposure_comp,
                                     n_frames=a.calib_frames, tol=a.calib_tol, out_dir=a.out, fixed=not a.rt_exposure_auto)
    if a.rt_exposure_auto: a.rt_exposure_comp = ec; RTLENS = rt_lens_attributes()
    if a.calib_only: print("calibration only: done", flush=True); sys.exit(0)
print(f"rt capture: rt_lens ftheta ({a.rt_spp} spp, DLSS-RR, exposure_compensation {a.rt_exposure_comp:+.2f} EV{' solved' if a.rt_exposure_auto else ''})", flush=True)
# pass A: no actors (the catchers-only show-only sensor)
if not (a.skip_a and os.path.exists(f"{a.out}/A_{n_frames - 1:04d}.npz")): run_pass("A", False)
else: print("pass A reused", flush=True)

# spawn synthetic cars on lanes relative to the ego's start pose
tf_rig0 = mat_to_carla_transform(t_sc @ rig_at(frame_t[0]))
wp0 = cmap.get_waypoint(tf_rig0.location, project_to_road=True, lane_type=carla.LaneType.Driving)
if wp0 is None: fail("the ego start pose projects onto no driving lane of the proxy world (wrong scene / usdz?)")
print("ego start waypoint road", wp0.road_id, "lane", wp0.lane_id, flush=True)
tm = client.get_trafficmanager(a.tm_port); tm.set_synchronous_mode(True); tm.set_global_distance_to_leading_vehicle(4.0)
if a.tm_speed: tm.global_percentage_speed_difference(a.tm_speed)
if a.ego:
    ego_bp = bpl.find("vehicle.lincoln.mkz"); ego = world.try_spawn_actor(ego_bp, carla.Transform(tf_rig0.location + carla.Location(z=0.3), tf_rig0.rotation))
    if ego is not None:
        ego.set_simulate_physics(False); actors.append(ego)
spawned = []
for spec_ in a.cars:
    parts_ = spec_.split(":"); bp_name, color, ahead, lane = parts_[:4]; speed_kmh = float(parts_[4]) if len(parts_) > 4 else 0.0; delay_s = float(parts_[5]) if len(parts_) > 5 else 0.0
    wp = wp0
    if lane == "opp":
        # reference = where the ego will be when this car starts, so "ahead" is measured from there
        wp_ref = wp0
        if delay_s > 0:
            tf_ref = mat_to_carla_transform(t_sc @ rig_at(frame_t[0] + int(delay_s * 1e6)))
            wp_ref = cmap.get_waypoint(tf_ref.location, project_to_road=True, lane_type=carla.LaneType.Driving) or wp0
        # the oncoming carriageway may be a separate OpenDRIVE road: probe laterally to the left of the
        # ego lane and accept the first driving waypoint whose heading opposes the ego's
        fwd0 = wp_ref.transform.get_forward_vector(); right0 = wp_ref.transform.get_right_vector(); found = None
        for off in (6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0, 24.0):
            probe = carla.Location(wp_ref.transform.location.x - right0.x * off, wp_ref.transform.location.y - right0.y * off, wp_ref.transform.location.z)
            w_ = cmap.get_waypoint(probe, project_to_road=True, lane_type=carla.LaneType.Driving)
            if w_ is None: continue
            f_ = w_.transform.get_forward_vector()
            if fwd0.x * f_.x + fwd0.y * f_.y < -0.5 and w_.transform.location.distance(probe) < 3.0:
                found = w_; print(f"  oncoming lane found {off} m left: road {w_.road_id} lane {w_.lane_id}", flush=True); break
        if found is None: print("no oncoming lane found for", spec_, flush=True); continue
        wp = found
    else:
        n = int(lane)
        for _ in range(abs(n)):
            nxt = wp.get_left_lane() if n > 0 else wp.get_right_lane()
            if nxt is None or nxt.lane_type != carla.LaneType.Driving: break
            wp = nxt
    ahead = float(ahead)
    if lane == "opp":
        nxts = wp.previous(abs(ahead)) if ahead > 0 else wp.next(abs(ahead))
    else:
        nxts = wp.next(ahead) if ahead > 0 else wp.previous(-ahead)
    if not nxts: print("no waypoint for", spec_, flush=True); continue
    wpt = nxts[0]
    bp = bpl.find("vehicle." + bp_name)
    if bp.has_attribute("color"): bp.set_attribute("color", color)
    tf = wpt.transform; tf.location.z += 0.3
    if a.drive in ("kinematic", "physics") and delay_s > 0: tf.location.z -= 500.0 + 6.0 * len(spawned)      # delayed cars start parked underground, stacked apart
    v = world.try_spawn_actor(bp, tf)
    if v is None: print("spawn failed", spec_, flush=True); continue
    if a.drive == "tm":
        v.set_autopilot(True, a.tm_port)
        if speed_kmh > 0: tm.set_desired_speed(v, speed_kmh)
    elif a.drive == "physics":
        path = [wpt]
        for _ in range(900):
            nx = path[-1].next(1.0)
            if not nx: break
            path.append(nx[0])
        v.set_simulate_physics(False)          # parked (possibly underground) until its start time; the follower enables physics
        phys.append(dict(actor=v, path=path, speed=speed_kmh / 3.6, delay=delay_s, started=False, idx=0))
        print("  physics path", len(path), "m", flush=True)
    else:
        v.set_simulate_physics(False)
        path = [wpt]
        for _ in range(900):
            nx = path[-1].next(1.0)
            if not nx: break
            path.append(nx[0])
        kin.append(dict(actor=v, path=path, speed=speed_kmh / 3.6, dz=0.05, delay=delay_s))
        print("  kinematic path", len(path), "m", flush=True)
    actors.append(v); spawned.append(dict(id=int(v.id), bp=bp_name, color=color, road=int(wpt.road_id), lane=int(wpt.lane_id), speed=speed_kmh))
    print("spawned", spawned[-1], flush=True)
# synthetic pedestrians on the shoulders (or at a lateral offset) relative to the ego's start pose; kinematic (WalkerControl)
import random
spawned_walkers = []
for spec_ in a.walkers:
    parts_ = spec_.split(":"); bp_name, ahead, side = parts_[:3]; speed_kmh = float(parts_[3]) if len(parts_) > 3 else 4.0
    delay_s = float(parts_[4]) if len(parts_) > 4 else 0.0; mode = parts_[5] if len(parts_) > 5 else "along"
    ahead = float(ahead); nxts = wp0.next(ahead) if ahead > 0 else wp0.previous(-ahead)
    if not nxts: print("no waypoint for walker", spec_, flush=True); continue
    wpt = nxts[0]
    if side in ("R", "L"):
        # outermost driving lane of this carriageway on that side, then half a lane + 1.5 m onto the shoulder
        w_ = wpt
        while True:
            nx = w_.get_right_lane() if side == "R" else w_.get_left_lane()
            if nx is None or nx.lane_type != carla.LaneType.Driving or nx.lane_id * w_.lane_id < 0: break
            w_ = nx
        sgn = 1.0 if side == "R" else -1.0; lat = sgn * (w_.lane_width / 2 + 1.5); base = w_.transform
    else:
        lat = float(side); base = wpt.transform; w_ = wpt
    # total driving width across this carriageway (a crosser walks the shoulder offset plus all lanes)
    width = wpt.lane_width
    for step in ("left", "right"):
        q_ = wpt
        while True:
            q_ = q_.get_left_lane() if step == "left" else q_.get_right_lane()
            if q_ is None or q_.lane_type != carla.LaneType.Driving or q_.lane_id * wpt.lane_id < 0: break
            width += q_.lane_width
    right = base.get_right_vector(); fwd = base.get_forward_vector()
    loc = None
    for lat_try in (lat, np.sign(lat) * (w_.lane_width / 2 - 0.7) if lat != 0 else 0.0):     # no mesh under the shoulder -> road edge
        cand = carla.Location(base.location.x + right.x * lat_try, base.location.y + right.y * lat_try, base.location.z + 5.0)
        gp = world.ground_projection(cand, 20.0)
        if gp is not None: loc = carla.Location(cand.x, cand.y, gp.location.z + 1.0); lat = lat_try; break
        print(f"  walker {spec_}: no ground at lateral {lat_try:.1f} m", flush=True)
    if loc is None: print("walker: no ground for", spec_, flush=True); continue
    bp = random.choice(list(bpl.filter("walker.pedestrian.*"))) if bp_name == "random" else bpl.find("walker.pedestrian." + bp_name)
    if bp.has_attribute("is_invincible"): bp.set_attribute("is_invincible", "true")
    if mode == "cross": d = carla.Vector3D(-np.sign(lat) * right.x, -np.sign(lat) * right.y, 0.0)
    elif mode == "back": d = carla.Vector3D(-fwd.x, -fwd.y, 0.0)
    else: d = carla.Vector3D(fwd.x, fwd.y, 0.0)
    yaw = float(np.degrees(np.arctan2(d.y, d.x)))
    w = world.try_spawn_actor(bp, carla.Transform(loc, carla.Rotation(yaw=yaw)))
    if w is None: print("walker spawn failed", spec_, flush=True); continue
    walkers.append(dict(actor=w, speed=speed_kmh / 3.6, delay=delay_s, mode=mode, dir=d, start=loc, cross_m=abs(lat) + width + 1.0, done=False))
    actors.append(w); spawned_walkers.append(dict(id=int(w.id), bp=bp.id, ahead=ahead, side=side, lateral=float(lat), speed=speed_kmh, delay=delay_s, mode=mode))
    print("spawned walker", spawned_walkers[-1], f"z {loc.z:.2f}", flush=True)
if walkers:
    world.tick(); step_walkers(frame_t[0])
    for wk in walkers: print(f"  walker {wk['actor'].id} at {wk['actor'].get_location()}", flush=True)
# settle: a spawned car floats 0.3 m and a walker 1 m above the road until physics drops them, and NuRec shows it in the
# first frames; late starters are dropped one at a time at their start pose (several share one), recorded, re-parked
def settle(n=None):
    for _ in range(a.settle_ticks if n is None else n): world.tick()
if a.drive == "tm":
    settle(max(int(a.fps), a.settle_ticks))
else:
    for pc in phys:
        if pc["delay"] <= 0: pc["actor"].set_simulate_physics(True)      # physics on now, no throttle: it rests until pass B starts it
    settle()
    for pc in phys:
        if pc["delay"] <= 0: pc["settled"] = pc["actor"].get_transform()
    for i_, pc in enumerate(p for p in phys if p["delay"] > 0):
        p0 = pc["path"][0].transform; v_ = pc["actor"]
        v_.set_transform(carla.Transform(p0.location + carla.Location(z=0.3), p0.rotation)); v_.set_simulate_physics(True); settle()
        pc["settled"] = v_.get_transform(); v_.set_simulate_physics(False)
        v_.set_transform(carla.Transform(carla.Location(p0.location.x, p0.location.y, p0.location.z - 500.0 - 6.0 * i_), p0.rotation))
    for pc in phys: print(f"  car {pc['actor'].id} settled: dz {pc['settled'].location.z - pc['path'][0].transform.location.z - 0.3:+.3f} m from the spawn height, pitch {pc['settled'].rotation.pitch:+.2f}", flush=True)
    for wk in walkers: print(f"  walker {wk['actor'].id} settled: dz {wk['actor'].get_location().z - wk['start'].z:+.3f} m", flush=True)
    world.tick()
json.dump(dict(scene=sid, frames=int(n_frames), fps=a.fps, frame_t=frame_t, cars=spawned, walkers=spawned_walkers, sun=a.sun, w=a.w, h=a.h, ftheta=FT, furniture=a.furniture,
               capture_mode="rt", skymap=SKYMAP, rt=dict(spp=a.rt_spp, exposure_comp=a.rt_exposure_comp, exposure_mode="auto" if a.rt_exposure_auto else "fixed", exposure_calib=CALIB),
               catcher_tags=a.catcher_tags),
          open(f"{a.out}/meta.json", "w"))

# pass B: with cars
run_pass("B", True, actor_ids=[sp_["id"] for sp_ in spawned] + [sw_["id"] for sw_ in spawned_walkers])
meta = json.load(open(f"{a.out}/meta.json")); meta["warmup_ticks"] = WARMUP; json.dump(meta, open(f"{a.out}/meta.json", "w"))

# engine pass
t_start = time.time()
for k, t_us in enumerate(frame_t):
    if a.skip_e and os.path.exists(f"{a.out}/E_{k:04d}.npz"): continue
    _, Tcam = cam_tf_at(t_us)
    rgb, dist, opa = render_engine(Tcam, t_us)
    np.savez(f"{a.out}/E_{k:04d}.npz", color=rgb, distance=dist, opacity=opa)
    if k % 50 == 0: print(f"  engine frame {k}/{n_frames} {time.time() - t_start:.0f}s", flush=True)

teardown()
print("done", flush=True)
