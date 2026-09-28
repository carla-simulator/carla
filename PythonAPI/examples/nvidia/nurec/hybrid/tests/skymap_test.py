"""Exercise world.set_sky_light_map / clear_sky_light_map on a live server.

  python skymap_test.py --port 3300 --out /path/captures [--probe hyb_illum_7c2cf6cd.jpg]

Spawns a car, looks at it from behind (so the car's left is the image's left), captures:
  a) default sky,  b) synthetic map (blue top / orange bottom, white patch on the ego's LEFT),
  c) the real illum_probe panorama rotated into the CARLA frame,  d) after clear_sky_light_map.
Reports car-pixel colour per state, the left/right asymmetry induced by the patch, and |d - a|.
"""
import argparse, os, sys, time, queue

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import skymap  # noqa: E402
import carla  # noqa: E402

ap = argparse.ArgumentParser()
ap.add_argument("--host", default="localhost"); ap.add_argument("--port", type=int, default=3300)
ap.add_argument("--out", default="/home/german/Projects/CARLA_SOURCE/.omc/captures")
ap.add_argument("--probe", default="/home/german/Projects/CARLA_SOURCE/.omc/captures/hyb_illum_7c2cf6cd.jpg")
ap.add_argument("--intensity", type=float, default=skymap.DEFAULT_INTENSITY)
ap.add_argument("--synthetic-intensity", type=float, default=skymap.DEFAULT_INTENSITY)
ap.add_argument("--map", default="Town10HD_Opt"); ap.add_argument("--settle", type=int, default=40)
ap.add_argument("--car", default="vehicle.dodge.charger"); ap.add_argument("--color", default="20,20,20")
a = ap.parse_args()
os.makedirs(a.out, exist_ok=True)

client = carla.Client(a.host, a.port); client.set_timeout(120.0)
world = client.get_world()
if a.map and a.map not in world.get_map().name:
    world = client.load_world(a.map)
settings = world.get_settings(); settings.synchronous_mode = True; settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)
world.set_weather(carla.WeatherParameters(sun_altitude_angle=45.0, sun_azimuth_angle=60.0, cloudiness=10.0))

bp = world.get_blueprint_library()
car_bp = bp.find(a.car)
if car_bp.has_attribute("color"):
    car_bp.set_attribute("color", a.color)
sp = world.get_map().get_spawn_points()[0]
car = world.spawn_actor(car_bp, sp)
car.set_simulate_physics(False)

W, H = 1280, 720
fwd, right, up = skymap.carla_rotation_rows(sp.rotation.yaw)
cam_loc = np.array([sp.location.x, sp.location.y, sp.location.z]) - 7.0 * fwd + 2.4 * up
cam_tf = carla.Transform(carla.Location(*map(float, cam_loc)), carla.Rotation(pitch=-12.0, yaw=sp.rotation.yaw, roll=0.0))


def make_cam(kind):
    b = bp.find(kind); b.set_attribute("image_size_x", str(W)); b.set_attribute("image_size_y", str(H)); b.set_attribute("fov", "70")
    s = world.spawn_actor(b, cam_tf); q = queue.Queue(); s.listen(q.put); return s, q


rgb, qrgb = make_cam("sensor.camera.rgb")
seg, qseg = make_cam("sensor.camera.semantic_segmentation")


def grab(n):
    for _ in range(n):
        world.tick()
        while not qrgb.empty(): qrgb.get()
        while not qseg.empty(): qseg.get()
    world.tick()
    im = qrgb.get(timeout=30.0); sm = qseg.get(timeout=30.0)
    img = np.frombuffer(im.raw_data, np.uint8).reshape(H, W, 4)[..., :3][..., ::-1].copy()
    tag = np.frombuffer(sm.raw_data, np.uint8).reshape(H, W, 4)[..., 2].copy()
    return img, tag


VEHICLE_TAGS = {14, 15, 16, 17, 18, 19}


def stats(img, tag):
    m = np.isin(tag, list(VEHICLE_TAGS))
    if m.sum() < 500:
        return dict(px=int(m.sum()))
    ys, xs = np.nonzero(m); cx = xs.mean()
    lum = img.mean(-1)
    return dict(px=int(m.sum()), rgb=[round(float(v), 1) for v in img[m].mean(0)],
                left=round(float(lum[m & (np.arange(W)[None, :] < cx)].mean()), 1),
                right=round(float(lum[m & (np.arange(W)[None, :] >= cx)].mean()), 1))


results = {}
try:
    img_a, tag_a = grab(a.settle); results["a_default"] = stats(img_a, tag_a)
    Image.fromarray(img_a).save(os.path.join(a.out, "skymap_a_default.png"))
    img_a2, tag_a2 = grab(a.settle); results["a2_default_again"] = stats(img_a2, tag_a2)
    m_aa = None
    m_aa = np.isin(tag_a, list(VEHICLE_TAGS)) & np.isin(tag_a2, list(VEHICLE_TAGS))
    results["drift_a2_vs_a_car"] = round(float(np.abs(img_a.astype(float) - img_a2.astype(float))[m_aa].mean()), 2)

    M = skymap.rig_to_carla_matrix(ego_transform=sp)
    # Orientation control: the same patch on the ego's RIGHT. Left-minus-right car luminance must flip sign.
    syn_r = np.roll(skymap.synthetic_test_map(512, 256, side="right"), 256, axis=1)
    skymap.apply(world, skymap.rig_pano_to_carla(syn_r, M), a.synthetic_intensity)
    img_e, tag_e = grab(a.settle); results["e_synthetic_right"] = stats(img_e, tag_e)
    Image.fromarray(img_e).save(os.path.join(a.out, "skymap_e_synthetic_right.png"))

    t0 = time.time()
    syn = skymap.synthetic_test_map(512, 256, side="left")
    # synthetic_test_map is authored in the CARLA layout for an ego with yaw 0 (patch on its left). A
    # CARLA column u (azimuth phi toward +Y right) is rig azimuth -phi, i.e. probe column u + W/2, so
    # rolling by half a turn turns it into a rig-frame map; rotating that with THIS ego's transform puts
    # the patch on this car's left whatever its yaw.
    syn_rig = np.roll(syn, syn.shape[1] // 2, axis=1)
    syn_c = skymap.rig_pano_to_carla(syn_rig, M)
    skymap.apply(world, syn_c, a.synthetic_intensity)
    results["set_call_s"] = round(time.time() - t0, 3)
    img_b, tag_b = grab(a.settle); results["b_synthetic"] = stats(img_b, tag_b)
    Image.fromarray(img_b).save(os.path.join(a.out, "skymap_b_synthetic.png"))
    Image.fromarray((np.clip(syn_c, 0, 1) ** (1 / 2.2) * 255).astype(np.uint8)).save(os.path.join(a.out, "skymap_b_map.png"))
    results["has_map_after_set"] = bool(world.has_sky_light_map())

    # weather change must not revert the map
    world.set_weather(carla.WeatherParameters(sun_altitude_angle=45.0, sun_azimuth_angle=60.0, cloudiness=10.0))
    img_b2, tag_b2 = grab(4); results["b_after_set_weather"] = stats(img_b2, tag_b2)

    if a.probe and os.path.exists(a.probe):
        pano, meta = skymap.load_probe(a.probe, a.probe[:-4] + ".json" if os.path.exists(a.probe[:-4] + ".json") else None)
        pano_c = skymap.rig_pano_to_carla(pano, M)
        skymap.apply(world, pano_c, a.intensity)
        img_c, tag_c = grab(a.settle); results["c_probe"] = stats(img_c, tag_c)
        Image.fromarray(img_c).save(os.path.join(a.out, "skymap_c_probe.png"))
        Image.fromarray((np.clip(pano_c, 0, 1) ** (1 / 2.2) * 255).astype(np.uint8)).save(os.path.join(a.out, "skymap_c_map.png"))

    world.clear_sky_light_map()
    results["has_map_after_clear"] = bool(world.has_sky_light_map())
    img_d, tag_d = grab(a.settle); results["d_cleared"] = stats(img_d, tag_d)
    Image.fromarray(img_d).save(os.path.join(a.out, "skymap_d_cleared.png"))
    m = np.isin(tag_a, list(VEHICLE_TAGS)) & np.isin(tag_d, list(VEHICLE_TAGS))
    results["abs_diff_d_vs_a_car"] = round(float(np.abs(img_a[m].astype(float) - img_d[m].astype(float)).mean()), 2)
    results["abs_diff_b_vs_a_car"] = round(float(np.abs(img_a[m].astype(float) - img_b[m].astype(float)).mean()), 2)

    sheet = np.concatenate([np.concatenate([img_a, img_b], 1), np.concatenate([locals().get("img_c", img_a), img_d], 1)], 0)
    Image.fromarray(sheet).resize((W, H)).save(os.path.join(a.out, "skymap_sheet.png"))
finally:
    for s in (rgb, seg):
        s.stop(); s.destroy()
    car.destroy()
    settings.synchronous_mode = False; world.apply_settings(settings)

import json
print(json.dumps(results, indent=1))
