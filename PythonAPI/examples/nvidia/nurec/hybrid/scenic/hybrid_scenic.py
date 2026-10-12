"""Scenic-driven hybrid capture: a Scenic scenario controls the ego and every other agent on the proxy road of a NuRec
scene, the ego carries the recorded camera, and every frame is composited by hybrid_video_finish.py exactly like a
hybrid_video.py run (same A_/B_/E_ npz layout and meta.json).

Where hybrid_video.py replays the recorded ego trajectory and moves synthetic cars around it, here the ego is a physics
vehicle driven by the scenario's behaviour, so the camera goes where the scenario takes it: the path-traced actor layer
(`sensor.camera.rt_lens` + instance/distance AOVs, camera_model=ftheta) is rendered by sensors attached to the ego, the
catchers-only pass (A) is replayed afterwards over the ego's captured camera poses, and the neural scene (E) is rendered
at those poses, each at the recorded time closest to the camera position along the recorded drive (the recorded actors
are parked out of view). The scene's road network is handed to Scenic as OpenDRIVE (scenic_xodr.py cleans up the
junction records the exporter writes), so Scenic's lanes coincide with the proxy world CARLA generates from the same file.

usage: hybrid_scenic.py OUT_DIR --scene 7c2cf6cd --usdz PATH --scenario hwy_cutin.scenic [--seed 0] [--param K=V ...]
       [--frames 0] [--fps 20] [--sun auto:illum.json:sun_calib.json] [--rt-spp 16] [--rt-exposure-comp auto] ...
The scenario receives the globals ego_x/ego_y/ego_heading (Scenic frame, lane centre of the recorded start pose),
clip_end_x/clip_end_y (Scenic frame, recorded end pose), ego_speed_kmh (recorded start speed), timestep and map/carla_map.
Every exit passes through teardown(): actors destroyed, sky light map cleared, cvars and asynchronous mode restored.
"""
import argparse, atexit, json, os, sys, time, zipfile, queue, socket, struct, random
import numpy as np
import grpc
from scipy.spatial.transform import Rotation as Rot, Slerp

HERE = os.path.dirname(os.path.abspath(__file__)); HYBRID = os.path.dirname(HERE); NUREC_DIR = os.path.dirname(HYBRID)
for p in (NUREC_DIR, HYBRID, HERE): sys.path.insert(0, p)
import carla  # noqa: E402
import msgpack  # noqa: E402
from utils import mat_to_carla_transform, carla_transform_to_nurec  # noqa: E402
from projection_functions import get_t_rig_enu_from_ecef  # noqa: E402
from nre.grpc.protos import sensorsim_pb2 as s, sensorsim_pb2_grpc as g, common_pb2 as c  # noqa: E402
import scenic_xodr  # noqa: E402

OPTICAL_TO_FLU = np.array([[0.0, 0.0, 1.0], [-1.0, 0.0, 0.0], [0.0, -1.0, 0.0]])
OPTICAL_BASIS = np.eye(4); OPTICAL_BASIS[:3, :3] = OPTICAL_TO_FLU
SYN_TAGS = (12, 13) + tuple(range(14, 20))      # Pedestrian, Rider, vehicles (crp::CityObjectLabel)

ap = argparse.ArgumentParser()
ap.add_argument("out"); ap.add_argument("--scene", required=True); ap.add_argument("--usdz", required=True)
ap.add_argument("--scenario", required=True, help="Scenic file (see hwy_cutin.scenic / urban_lead_brake.scenic)")
ap.add_argument("--seed", type=int, default=0, help="Scenic sampling seed")
ap.add_argument("--param", action="append", default=[], help="K=V passed to the scenario as a global parameter (numbers are converted)")
ap.add_argument("--port", type=int, default=3000); ap.add_argument("--nre-port", type=int, default=46436)
ap.add_argument("--fps", type=float, default=20.0)
ap.add_argument("--frames", type=int, default=0, help="max frames (0 = until the scenario terminates or the ego passes the recorded end, at most --max-seconds)")
ap.add_argument("--max-seconds", type=float, default=0.0, help="cap on the simulation (0 = twice the recorded drive + 5 s: a scenario whose ego never reaches the route end would otherwise fill the disk with frames)")
ap.add_argument("--camera", default="camera_front_wide_120fov")
ap.add_argument("--sun", default="280,40,10", help="CARLA azimuth,altitude,cloudiness or auto:<illum_probe json>:<sun_calib json>")
ap.add_argument("--exposure", type=float, default=0.44, help="r.EyeAdaptation.LensAttenuation for the raster path (the rt_lens sensors use --rt-exposure-comp)")
ap.add_argument("--keep-world", action="store_true"); ap.add_argument("--tm-port", type=int, default=8000)
ap.add_argument("--furniture", type=int, default=0, help="carla.OpenDrive.StreetFurniture for the generated proxy world (0 = bare road)")
ap.add_argument("--rt-spp", type=int, default=16, help="path-tracer samples per pixel (clamped by the sensor to r.PathTracing.MaxFramePassCount)")
ap.add_argument("--rt-exposure-comp", default="auto", help="exposure_compensation (EV) of the rt_lens sensor, or 'auto' = solved against the neural road on the recorded trajectory (exposure_calib.py)")
ap.add_argument("--calib-frames", type=int, default=8); ap.add_argument("--calib-start", type=float, default=5.0); ap.add_argument("--calib-tol", type=float, default=0.05)
ap.add_argument("--catcher-tags", default="Roads,Sidewalks,RoadLines", help="semantic labels that receive the cars' cast shadow")
ap.add_argument("--skymap", type=int, default=1); ap.add_argument("--skymap-intensity", type=float, default=0.0)
ap.add_argument("--warmup-min", type=int, default=20, help="minimum ticks before the first frame (settles the spawned cars; the sensors' own readiness is checked on top)")
ap.add_argument("--rig-height", type=float, default=None, help="rig origin height above the road in m (default: measured from the recorded start pose)")
a = ap.parse_args()
a.rt_exposure_auto = str(a.rt_exposure_comp).lower() == "auto"
a.rt_exposure_comp = a.calib_start if a.rt_exposure_auto else float(a.rt_exposure_comp)
CALIB = None
os.makedirs(a.out, exist_ok=True)


def fail(msg): raise SystemExit("hybrid_scenic: " + msg)


def parse_param(kv):
    k, v = kv.split("=", 1)
    try: return k, int(v)
    except ValueError: pass
    try: return k, float(v)
    except ValueError: return k, v


# ---------------- engine: trajectory, camera ----------------
st = g.SensorsimServiceStub(grpc.insecure_channel(f"localhost:{a.nre_port}", options=[("grpc.max_receive_message_length", 256 << 20)]))
try: scenes = list(st.get_available_scenes(c.Empty(), timeout=60).scene_ids)
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
R2C = M(cam.rig_to_camera); R2C_INV = np.linalg.inv(R2C)
rots = Rot.from_matrix(np.array([T[:3, :3] for T in Ts])); slerp = Slerp(tt.astype(np.float64), rots)
pos = np.array([T[:3, 3] for T in Ts])
seg = np.diff(pos, axis=0); seg_len = np.linalg.norm(seg, axis=1); arc = np.concatenate([[0.0], np.cumsum(seg_len)])


def rig_at(t_us):
    t_us = int(np.clip(t_us, tt[0], tt[-1]))
    T = np.eye(4); T[:3, :3] = slerp(float(t_us)).as_matrix()
    T[:3, 3] = np.array([np.interp(t_us, tt, pos[:, i]) for i in range(3)]); return T


def project_on_drive(p):
    """Closest point of the recorded drive (engine frame) to p: (recorded time us, arc length m, lateral distance m)."""
    d = p[None, :] - pos[:-1]; L2 = np.maximum(seg_len ** 2, 1e-9)
    al = np.clip((d * seg).sum(1) / L2, 0.0, 1.0); q = pos[:-1] + al[:, None] * seg
    dist = np.linalg.norm(q - p[None, :], axis=1); k = int(np.argmin(dist))
    return int(tt[k] + al[k] * (tt[k + 1] - tt[k])), float(arc[k] + al[k] * seg_len[k]), float(dist[k])


dyn = st.get_dynamic_objects(s.AvailableDynamicObjectsRequest(scene_id=sid), timeout=120).dynamic_objects
parked = []
for d in dyn:
    if not d.trajectory.poses: continue
    T = M(d.trajectory.poses[0].pose); T[2, 3] -= 200.0       # recorded actors parked far underground: not rendered
    parked.append(s.DynamicObject(track_id=d.id, pose_pair=s.PosePair(start_pose=P(T), end_pose=P(T))))

fp = cam.intrinsics.ftheta_param
a.w, a.h = int(cam.intrinsics.resolution_w), int(cam.intrinsics.resolution_h)
spec = s.CameraSpec(logical_id=cam.logical_id, trajectory_idx=cam.trajectory_idx, resolution_w=a.w, resolution_h=a.h, shutter_type=s.GLOBAL, ftheta_param=fp)
FT = dict(cx=float(fp.principal_point_x), cy=float(fp.principal_point_y), poly=[float(x) for x in fp.pixeldist_to_angle_poly], max_angle=float(fp.max_angle))
print("f-theta lens:", FT, f"{a.w}x{a.h}", flush=True)


def rt_lens_attributes():
    return dict(camera_model="ftheta", distortion_coeffs=",".join("%.12g" % c_ for c_ in FT["poly"]), theta_max_deg="%.6f" % np.degrees(FT["max_angle"]),
                fx="%.10f" % (1.0 / a.w), fy="%.10f" % (1.0 / a.h), cx="%.10f" % (FT["cx"] / a.w), cy="%.10f" % (FT["cy"] / a.h),
                image_size_x=str(a.w), image_size_y=str(a.h), samples_per_pixel=str(a.rt_spp), enable_denoiser="true",
                exposure_mode="manual", exposure_compensation="%.4f" % a.rt_exposure_comp)


RTLENS = rt_lens_attributes()


def render_engine(Tcam, t_us):
    req = s.RGBRenderRequest(scene_id=sid, resolution_h=a.h, resolution_w=a.w, camera_intrinsics=spec, frame_start_us=int(t_us), frame_end_us=int(t_us) + 1,
                             sensor_pose=s.PosePair(start_pose=P(Tcam), end_pose=P(Tcam)), dynamic_objects=parked, image_format=s.RGB_UINT8_PLANAR, image_quality=-7.0)
    try: b = st.render_rgb(req, timeout=600).image_bytes
    except grpc.RpcError as e: fail(f"engine render_rgb failed at t={int(t_us)} us: {e.code().name} {e.details()}")
    if b[:4] != b"HYBR": fail(f"engine on port {a.nre_port} is not the patched serve-grpc (no HYBR payload for image_quality -7): start it via hybrid_run.py")
    h, w = struct.unpack("<II", b[4:12]); o = 12
    rgb = np.frombuffer(b[o:o + 3 * h * w], dtype=np.uint8).reshape(3, h, w).transpose(1, 2, 0); o += 3 * h * w
    dist = np.frombuffer(b[o:o + 4 * h * w], dtype="<f4").reshape(h, w); o += 4 * h * w
    opa = np.frombuffer(b[o:o + 4 * h * w], dtype="<f4").reshape(h, w)
    return rgb.copy(), dist.copy(), opa.copy()


dt_us = int(1e6 / a.fps)
rec_frame_t = [int(t) for t in np.arange(tt[0], tt[-1], dt_us)]        # the recorded drive's frame times (exposure calibration)
print(f"scene {sid}: recorded drive {(tt[-1] - tt[0]) / 1e6:.1f} s, {arc[-1]:.0f} m", flush=True)

# ---------------- CARLA proxy world ----------------
with zipfile.ZipFile(a.usdz) as z:
    xodr = z.read("map.xodr").decode(); rig = json.loads(z.read("rig_trajectories.json"))
t_sc = get_t_rig_enu_from_ecef(np.array(rig["T_world_base"]), xodr); t_sc_inv = np.linalg.inv(t_sc)
client = carla.Client("localhost", a.port); client.set_timeout(600.0)
try: world = client.get_world()
except RuntimeError as e: fail(f"no CARLA server on port {a.port} ({e})")
if not (a.keep_world and world.get_map().name.endswith("OpenDriveMap")):
    sk0 = socket.create_connection(("localhost", a.port), timeout=10)
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


SKYMAP = False; TORN_DOWN = False; LIVE_ACTORS = []


def teardown():
    global TORN_DOWN
    if TORN_DOWN: return
    TORN_DOWN = True
    steps = [("cvars", lambda: (console("r.EyeAdaptation.MethodOverride -1"), console("r.EyeAdaptation.LensAttenuation 0.78"), console("r.MotionBlur.Amount 0.5"))),
             ("sky light map", lambda: world.clear_sky_light_map() if SKYMAP else None),
             ("destroy actors", lambda: client.apply_batch([carla.command.DestroyActor(x.id) for x in LIVE_ACTORS if x.is_alive]) if LIVE_ACTORS else None),
             ("asynchronous mode", lambda: (world.tick(), setattr(settings, "synchronous_mode", False), setattr(settings, "fixed_delta_seconds", None), world.apply_settings(settings))),
             ("TM asynchronous", lambda: client.get_trafficmanager(a.tm_port).set_synchronous_mode(False))]
    for name, step in steps:
        try: step()
        except Exception as e: print(f"teardown: {name} failed ({e})", flush=True)


atexit.register(teardown)

a.sun_illum_json = None
if a.sun.startswith("auto:"):
    illum_json, calib_json = a.sun[5:].split(":")
    il = json.load(open(illum_json)); cb = json.load(open(calib_json))["fit"]; a.sun_illum_json = illum_json
    if not il.get("sun"): fail(f"{illum_json} has no sun; pass --sun az,alt,cloud")
    d_rig = np.array(il["sun"]["dir_rig"]); Rw = (t_sc @ np.array(il["rig_pose"]))[:3, :3]
    d_scn = Rw @ d_rig; d_carla = np.array([d_scn[0], -d_scn[1], d_scn[2]])
    bearing = np.degrees(np.arctan2(d_carla[1], d_carla[0]))
    az = ((bearing - cb["c"]) / cb["k"]) % 360.0; alt = float(np.degrees(np.arcsin(np.clip(d_carla[2], -1, 1))))
    cloud = float(np.clip(2.0 * (il["sky_lum_std"] - 20.0), 0.0, 80.0))
    print(f"sun auto: rig az {il['sun']['az_deg']:.1f} el {il['sun']['el_deg']:.1f} -> CARLA azimuth {az:.1f} altitude {alt:.1f} cloudiness {cloud:.0f}", flush=True)
    a.sun = f"{az:.2f},{alt:.2f},{cloud:.1f}"
az, alt, cloud = [float(x) for x in a.sun.split(",")]
world.set_weather(carla.WeatherParameters(cloudiness=cloud, sun_azimuth_angle=az, sun_altitude_angle=alt))
if a.skymap and a.sun_illum_json is not None and hasattr(world, "set_sky_light_map"):
    import skymap  # noqa: E402
    pano_rig, il_meta = skymap.load_probe(os.path.splitext(a.sun_illum_json)[0] + ".jpg", a.sun_illum_json)
    M_rig = skymap.rig_to_carla_matrix(rig_pose=t_sc @ np.array(il_meta["rig_pose"]))
    pano_carla = skymap.rig_pano_to_carla(pano_rig, M_rig)
    skymap.apply(world, pano_carla, a.skymap_intensity or skymap.DEFAULT_INTENSITY, 512); SKYMAP = True
    print(f"sky light map set from {a.sun_illum_json}", flush=True)
elif hasattr(world, "has_sky_light_map") and world.has_sky_light_map():
    world.clear_sky_light_map(); print("stale sky light map cleared", flush=True)
console("r.EyeAdaptation.MethodOverride 3"); console(f"r.EyeAdaptation.LensAttenuation {a.exposure}"); console("r.MotionBlur.Amount 0")
bpl = world.get_blueprint_library()


def cam_tf_at(t_us):
    """Recorded camera pose (CARLA transform, engine matrix) at a recorded time: used by the exposure calibration."""
    Tcam = rig_at(t_us) @ R2C
    return mat_to_carla_transform(t_sc @ Tcam @ np.linalg.inv(OPTICAL_BASIS)), Tcam


def cam_tf_to_engine(tf):
    """A CARLA camera transform (world) -> the engine's T_world_from_optical."""
    return t_sc_inv @ carla_transform_to_nurec(tf) @ OPTICAL_BASIS


def spawn_cam(kind, tf, attrs, attach_to=None):
    try: bp = bpl.find(kind)
    except (IndexError, KeyError, RuntimeError): fail(f"no blueprint {kind}: the CARLA server predates the hybrid sensors (rebuild the server from this tree)")
    for k, val in attrs.items():
        if not bp.has_attribute(k): fail(f"{kind} has no attribute '{k}': the CARLA server predates the hybrid sensors (rebuild the server from this tree)")
        bp.set_attribute(k, val)
    if bp.has_attribute("enable_dlss"): bp.set_attribute("enable_dlss", "false")
    q = queue.Queue(); cm = world.spawn_actor(bp, tf, attach_to=attach_to) if attach_to is not None else world.spawn_actor(bp, tf); cm.listen(q.put); return cm, q


def arr(img): return np.frombuffer(img.raw_data, dtype=np.uint8).reshape((img.height, img.width, 4)).copy()


def dist_arr(img): return np.frombuffer(img.raw_data, dtype=np.float32).reshape((img.height, img.width)).copy()


def capture(cams):
    fr = {}
    for k, (cm, q) in cams.items():
        try:
            img = q.get(timeout=30)
            while not q.empty(): img = q.get_nowait()
            fr[k] = img
        except queue.Empty: fr[k] = None
    return fr


def actor_ids_in(inst):
    syn = np.isin(inst[:, :, 2], SYN_TAGS)
    return frozenset((inst[:, :, 1].astype(np.int32) | (inst[:, :, 0].astype(np.int32) << 8))[syn].tolist())


def destroy_cams(cams):
    for cm, _ in cams.values(): cm.stop()
    world.tick()
    client.apply_batch([carla.command.DestroyActor(cm.id) for cm, _ in cams.values()]); world.tick()


# ---------------- exposure of the path-traced layer, measured on the recorded drive ----------------
if a.rt_exposure_auto:
    import exposure_calib
    ec, CALIB = exposure_calib.solve(world, spawn_cam, capture, arr, dist_arr, cam_tf_at, render_engine, rec_frame_t, RTLENS, a.catcher_tags, ec0=a.rt_exposure_comp,
                                     n_frames=a.calib_frames, tol=a.calib_tol, out_dir=a.out, fixed=False)
    a.rt_exposure_comp = ec; RTLENS = rt_lens_attributes()
print(f"rt capture: rt_lens ftheta ({a.rt_spp} spp, DLSS-RR, exposure_compensation {a.rt_exposure_comp:+.2f} EV{' solved' if a.rt_exposure_auto else ''})", flush=True)

# ---------------- the scenario's frame: recorded start pose on the lane centre, recorded end pose ----------------
tf_rig0 = mat_to_carla_transform(t_sc @ rig_at(tt[0]))
wp0 = cmap.get_waypoint(tf_rig0.location, project_to_road=True, lane_type=carla.LaneType.Driving)
if wp0 is None: fail("the recorded start pose projects onto no driving lane of the proxy world (wrong scene / usdz?)")
tf_end = mat_to_carla_transform(t_sc @ rig_at(tt[-1]))
v0 = np.linalg.norm(pos[min(5, len(pos) - 1)] - pos[0]) / max((tt[min(5, len(tt) - 1)] - tt[0]) / 1e6, 1e-3)
rig_h = a.rig_height if a.rig_height is not None else float(tf_rig0.location.z - wp0.transform.location.z)
print(f"ego start: road {wp0.road_id} lane {wp0.lane_id} ({wp0.transform.location}), recorded speed {v0 * 3.6:.0f} km/h, rig {rig_h:+.2f} m above the lane", flush=True)


def scenic_heading(yaw_deg): return float(-np.radians(yaw_deg + 90.0))           # CARLA yaw -> Scenic heading (0 = +y, counter-clockwise)


scenic_map = os.path.join(a.out, "map_scenic.xodr")
text, rep = scenic_xodr.sanitize(xodr); open(scenic_map, "w").write(text)
print("scenic map:", rep, flush=True)
params = dict(map=scenic_map, carla_map=None, timestep=1.0 / a.fps, render=0, timeout=120, use2DMap=True, snapToGroundDefault=True, weather=None,
              ego_x=float(wp0.transform.location.x), ego_y=float(-wp0.transform.location.y), ego_heading=scenic_heading(wp0.transform.rotation.yaw),
              clip_end_x=float(tf_end.location.x), clip_end_y=float(-tf_end.location.y), clip_length_m=float(arc[-1]), ego_speed_kmh=float(v0 * 3.6))
params.update(dict(parse_param(kv) for kv in a.param))

# ---------------- Scenic ----------------
import scenic  # noqa: E402
from scenic.simulators.carla.simulator import CarlaSimulator, CarlaSimulation  # noqa: E402
from scenic.domains.driving.simulators import DrivingSimulator  # noqa: E402
from scenic.core.simulators import Simulation, Simulator  # noqa: E402

random.seed(a.seed); np.random.seed(a.seed)
scenario = scenic.scenarioFromFile(a.scenario, params=params, mode2D=True)
scene, _ = scenario.generate(maxIterations=2000)
print(f"scenario {os.path.basename(a.scenario)} seed {a.seed}: {len(scene.objects)} objects", flush=True)
CAPTURED = []; WARMUP = {}


class HybridSimulator(CarlaSimulator):
    """CarlaSimulator on the proxy world that is already loaded (no load_world / generate_opendrive_world); synchronous mode
    stays on when it is destroyed (pass A and the engine pass follow)."""

    def __init__(self, client, world, tm_port, timestep):
        DrivingSimulator.__init__(self)
        self.client = client; self.world = world; self.timestep = timestep
        self.tm = client.get_trafficmanager(tm_port); self.tm.set_synchronous_mode(True)
        self.render = False; self.record = ""; self.scenario_number = 0

    def createSimulation(self, scene, *, timestep, **kwargs):
        self.scenario_number += 1
        return HybridSimulation(scene, self.client, self.tm, False, "", self.scenario_number, timestep=self.timestep, **kwargs)

    def destroy(self): Simulator.destroy(self)


class HybridSimulation(CarlaSimulation):
    """One Scenic simulation = pass B. The recorded camera is attached to the ego (rig origin rig_h above the road, on the
    ego's vertical axis, camera offset = the scene's rig_to_camera); each step ticks, captures colour + tag + distance
    and saves B_k.npz with the camera pose the frame was rendered from."""

    def setup(self):
        Simulation.setup(self)                     # spawn the scene's objects (CarlaSimulation.setup would also set a weather)
        self.world.tick()
        for obj in self.objects:
            if isinstance(obj.carlaActor, carla.Vehicle): obj.carlaActor.apply_control(carla.VehicleControl(manual_gear_shift=False))
        self.world.tick()
        ego = self.objects[0].carlaActor; LIVE_ACTORS.extend(o.carlaActor for o in self.objects)
        others = [o.carlaActor.id for o in self.objects[1:] if isinstance(o.carlaActor, (carla.Vehicle, carla.Walker))]
        # the camera on the ego: rig origin on the ego's vertical axis rig_h above the lane, camera = rig @ rig_to_camera
        wp = cmap.get_waypoint(ego.get_location(), project_to_road=True, lane_type=carla.LaneType.Driving)
        ego_h = float(ego.get_location().z - wp.transform.location.z) if wp is not None else 0.0
        T_rel = np.eye(4); T_rel[2, 3] = rig_h - ego_h
        rel = mat_to_carla_transform(T_rel @ R2C @ np.linalg.inv(OPTICAL_BASIS))
        aux = dict(RTLENS, show_only_actor_ids=",".join(str(i) for i in others), shadow_catcher_tags=a.catcher_tags)
        self.cams = {"rgb": spawn_cam("sensor.camera.rt_lens", rel, aux, attach_to=ego), "instance_segmentation": spawn_cam("sensor.camera.rt_lens_instance", rel, aux, attach_to=ego),
                     "depth": spawn_cam("sensor.camera.rt_lens_distance", rel, aux, attach_to=ego)}
        self.world.get_spectator().set_transform(self.cams["rgb"][0].get_transform())
        print(f"  camera on ego {ego.id} ({ego.type_id}): relative {rel}; actors shown {others}", flush=True)
        # warm up: the sensors deliver and the tag AOV's actor set is stable for 3 lit ticks (>= warmup-min ticks, which also
        # lets the spawned cars settle on the road)
        hist = []
        for n in range(1, 121):
            self.world.tick(); fr = capture(self.cams)
            lit = all(v is not None for v in fr.values()) and (arr(fr["rgb"])[:, :, :3].max(-1) > 8).mean() >= 0.02
            hist.append(actor_ids_in(arr(fr["instance_segmentation"])) if lit else None)
            if n >= a.warmup_min and len(hist) >= 3 and hist[-1] is not None and all(h == hist[-1] for h in hist[-3:]): break
        else: fail("the rt_lens sensors delivered no stable rendered frame in 120 warm-up ticks")
        WARMUP.update(ticks=n, actor_ids=sorted(hist[-1]))
        print(f"  sensors warmed up after {n} ticks, actor ids in view {sorted(hist[-1])}", flush=True)
        # initial speeds (Scenic's own CARLA interface rejects them): the clip starts mid-drive
        for obj in self.objects:
            if isinstance(obj.carlaActor, carla.Vehicle) and obj.speed:
                fv = obj.carlaActor.get_transform().get_forward_vector()
                obj.carlaActor.set_target_velocity(carla.Vector3D(fv.x * obj.speed, fv.y * obj.speed, 0.0))
        self.k = 0; self.t_start = time.time()
        self.spawned = [dict(id=int(o.carlaActor.id), bp=o.carlaActor.type_id, color=o.carlaActor.attributes.get("color", ""), role=getattr(o, "rolename", None) or "",
                             speed=float(o.speed or 0.0) * 3.6, ego=o is self.objects[0]) for o in self.objects]

    def step(self):
        self.current_frame = self.world.tick()
        fr = capture(self.cams)
        missing = [name for name, img in fr.items() if img is None]
        if missing: fail(f"frame {self.k}: no frame from sensor(s) {missing} within 30 s (see the server log)")
        tf = fr["rgb"].transform                        # the pose this frame was rendered from (sensor world transform at capture)
        Tcam = cam_tf_to_engine(tf)
        t_us, s_m, lat = project_on_drive((Tcam @ R2C_INV)[:3, 3])          # the rig, not the camera: the drive is the rig's path
        inst = arr(fr["instance_segmentation"])
        sem_ = inst[:, :, 2].copy(); inst_ = (inst[:, :, 1].astype(np.uint32) + inst[:, :, 0].astype(np.uint32) * 256)
        np.savez(f"{a.out}/B_{self.k:04d}.npz", rgb=arr(fr["rgb"])[:, :, 2::-1].copy(), sem=sem_, inst=inst_, dist=dist_arr(fr["depth"]), T_cam=Tcam, t_us=t_us,
                 cam_tf=np.array([tf.location.x, tf.location.y, tf.location.z, tf.rotation.pitch, tf.rotation.yaw, tf.rotation.roll]))
        ego = self.objects[0].carlaActor; v = ego.get_velocity()
        CAPTURED.append(dict(k=self.k, t_us=int(t_us), s_m=s_m, lateral_m=lat, ego_speed_kmh=float(np.hypot(v.x, v.y) * 3.6)))
        if self.k < 4 or self.k % 50 == 0:
            print(f"  B frame {self.k}: ego {CAPTURED[-1]['ego_speed_kmh']:.0f} km/h, {s_m:.0f} m along the drive, {lat:.2f} m off it, t+{(t_us - tt[0]) / 1e6:.2f} s, "
                  f"{int(np.isin(sem_, SYN_TAGS).sum())} actor px, ids {sorted(actor_ids_in(inst))}, {time.time() - self.t_start:.0f}s", flush=True)
        self.k += 1

    def destroy(self):
        if getattr(self, "cams", None):
            destroy_cams(self.cams); self.cams = None
        CarlaSimulation.destroy(self)


max_seconds = a.max_seconds if a.max_seconds > 0 else 2.0 * (tt[-1] - tt[0]) / 1e6 + 5.0
max_steps = a.frames if a.frames > 0 else int(max_seconds * a.fps)
simulator = HybridSimulator(client, world, a.tm_port, 1.0 / a.fps)
sim = simulator.simulate(scene, maxSteps=max_steps, maxIterations=1, verbosity=1)
simulator.destroy()
if sim is None: fail("Scenic rejected the simulation (see the messages above)")
n_frames = len(CAPTURED)
print(f"pass B: {n_frames} frames, terminated: {sim.result.terminationReason}", flush=True)
if n_frames == 0: fail("no frames captured")
spawned = getattr(sim, "spawned", [])


# ---------------- pass A: catchers alone over the captured camera poses ----------------
def cam_tf_of(k):
    x, y, z, p_, yw, r_ = np.load(f"{a.out}/B_{k:04d}.npz")["cam_tf"]
    return carla.Transform(carla.Location(float(x), float(y), float(z)), carla.Rotation(pitch=float(p_), yaw=float(yw), roll=float(r_)))


cams = {"catch": spawn_cam("sensor.camera.rt_lens", cam_tf_of(0), dict(RTLENS, show_only_tags=a.catcher_tags))}
for n in range(1, 61):
    world.tick(); fr = capture(cams)
    if fr["catch"] is not None and (arr(fr["catch"])[:, :, :3].max(-1) > 8).mean() >= 0.02 and n >= 3: break
else: fail("pass A: the catchers sensor delivered no lit frame in 60 ticks")
WARMUP["A_ticks"] = n; t_start = time.time()
for k in range(n_frames):
    tf = cam_tf_of(k); cams["catch"][0].set_transform(tf); world.tick(); fr = capture(cams)
    if fr["catch"] is None: fail(f"pass A frame {k}: no frame within 30 s")
    B = np.load(f"{a.out}/B_{k:04d}.npz")
    np.savez(f"{a.out}/A_{k:04d}.npz", rgb=arr(fr["catch"])[:, :, 2::-1].copy(), T_cam=B["T_cam"], t_us=int(B["t_us"]))
    if k % 50 == 0: print(f"  A frame {k}/{n_frames} {time.time() - t_start:.0f}s", flush=True)
destroy_cams(cams)

# ---------------- engine pass ----------------
t_start = time.time()
for k in range(n_frames):
    B = np.load(f"{a.out}/B_{k:04d}.npz")
    rgb, dist, opa = render_engine(B["T_cam"], int(B["t_us"]))
    np.savez(f"{a.out}/E_{k:04d}.npz", color=rgb, distance=dist, opacity=opa)
    if k % 50 == 0: print(f"  engine frame {k}/{n_frames} {time.time() - t_start:.0f}s", flush=True)

json.dump(dict(scene=sid, frames=int(n_frames), fps=a.fps, frame_t=[f["t_us"] for f in CAPTURED], cars=[sp for sp in spawned if not sp["ego"]], ego=next((sp for sp in spawned if sp["ego"]), None),
               walkers=[], sun=a.sun, w=a.w, h=a.h, ftheta=FT, furniture=a.furniture, capture_mode="rt-scenic", skymap=SKYMAP,
               rt=dict(spp=a.rt_spp, exposure_comp=a.rt_exposure_comp, exposure_mode="auto" if a.rt_exposure_auto else "fixed", exposure_calib=CALIB),
               catcher_tags=a.catcher_tags, scenic=dict(scenario=os.path.abspath(a.scenario), seed=a.seed, params={k: v for k, v in params.items() if k not in ("map",)},
               termination=str(sim.result.terminationReason), rig_height=rig_h, map_report=rep), ego_track=CAPTURED, warmup_ticks=WARMUP),
          open(f"{a.out}/meta.json", "w"))
teardown()
print("done", flush=True)
