#!/usr/bin/env python3
"""Show-only render mode check for the scene-capture cameras.

Spawns a vehicle in front of a fixed camera pose and captures, in the same
tick:

  ref        plain rgb + semantic (everything)
  cars       rgb + semantic + depth with show_only_tags=Car and
             shadow_catcher_tags=Roads,Sidewalks,RoadLines
  catch      rgb with show_only_tags=Roads,Sidewalks,RoadLines (catchers only)
  carsonly   rgb with show_only_tags=Car and no catchers

then verifies numerically that the show-only images contain only the
selected labels, that the car is present, that empty pixels carry the Sky
label (the semantic camera labels everything without geometry as Sky) and
the far depth, and that the cast-shadow ratio cars/catch dips under the car and
stays ~1 elsewhere. Writes showonly_*.png plus a side-by-side sheet.

usage: showonly_test.py [--host localhost] [--port 3100] [--out DIR]
                        [--map Town10HD_Opt] [--w 1280 --h 720] [--exposure 0.44]
"""
import argparse
import os
import queue
import socket
import sys
import time

import msgpack
import numpy as np
from PIL import Image, ImageDraw

import carla

ap = argparse.ArgumentParser()
ap.add_argument("--host", default="localhost")
ap.add_argument("--port", type=int, default=3100)
_HERE = os.path.abspath(__file__)
for _ in range(8):  # tests/hybrid/nurec/nvidia/examples/PythonAPI/<repo>/<CARLA_SOURCE>
    _HERE = os.path.dirname(_HERE)
ap.add_argument("--out", default=os.path.join(_HERE, ".omc", "captures"))
ap.add_argument("--map", default="Town10HD_Opt")
ap.add_argument("--w", type=int, default=1280)
ap.add_argument("--h", type=int, default=720)
ap.add_argument("--exposure", type=float, default=0.44)
ap.add_argument("--warmup", type=int, default=25)
ap.add_argument("--vehicle", default="vehicle.dodge.charger")
ap.add_argument("--fisheye", action="store_true",
                help="also exercise sensor.camera.rgb_fisheye (two 1280x720 fisheyes crash the 2026-09-07 "
                     "build in FPixelReader::WritePixelsToBuffer with or without show-only, so off by default)")
a = ap.parse_args()
os.makedirs(a.out, exist_ok=True)

LABELS = {0: "None", 1: "Roads", 2: "Sidewalks", 3: "Buildings", 4: "Walls", 5: "Fences", 6: "Poles",
          7: "TrafficLight", 8: "TrafficSigns", 9: "Vegetation", 10: "Terrain", 11: "Sky", 12: "Pedestrians",
          13: "Rider", 14: "Car", 15: "Truck", 16: "Bus", 17: "Train", 18: "Motorcycle", 19: "Bicycle",
          20: "Static", 21: "Dynamic", 22: "Other", 23: "Water", 24: "RoadLines", 25: "Ground", 26: "Bridge",
          27: "RailTrack", 28: "GuardRail", 29: "Rock"}


def console(cmd):
    sk = socket.create_connection((a.host, a.port), timeout=10)
    sk.sendall(msgpack.packb([0, 1, "console_command", [[False], cmd]], use_bin_type=True))
    sk.settimeout(10)
    sk.recv(1 << 16)
    sk.close()


client = carla.Client(a.host, a.port)
client.set_timeout(120.0)
world = client.get_world()
if a.map not in world.get_map().name:
    print(f"loading {a.map} ...", flush=True)
    world = client.load_world(a.map)
    time.sleep(5)
settings = world.get_settings()
settings.synchronous_mode = True
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)
world.tick()

# Clear noon, high sun so the car casts a compact shadow onto the road.
weather = carla.WeatherParameters.ClearNoon
weather.sun_altitude_angle = 55.0
weather.sun_azimuth_angle = 200.0
weather.cloudiness = 0.0
world.set_weather(weather)
# Same manual exposure on every camera so the ratio image is meaningful.
console("r.EyeAdaptation.MethodOverride 3")
console(f"r.EyeAdaptation.LensAttenuation {a.exposure}")

bpl = world.get_blueprint_library()
spawn = world.get_map().get_spawn_points()[0]
vbp = bpl.find(a.vehicle)
if vbp.has_attribute("color"):
    vbp.set_attribute("color", "200,30,30")
vehicle = world.spawn_actor(vbp, spawn)
vehicle.set_simulate_physics(False)
actors = [vehicle]

# Camera 7 m behind and 2.8 m above the car, looking slightly down.
fwd = spawn.get_forward_vector()
cam_loc = spawn.location - fwd * 7.0 + carla.Location(z=2.8)
cam_rot = carla.Rotation(pitch=-14.0, yaw=spawn.rotation.yaw, roll=0.0)
cam_tf = carla.Transform(cam_loc, cam_rot)


def spawn_cam(kind, **attrs):
    bp = bpl.find(kind)
    for k, v in (("image_size_x", str(a.w)), ("image_size_y", str(a.h)), ("fov", "90")):
        bp.set_attribute(k, v)
    if bp.has_attribute("enable_dlss"):
        bp.set_attribute("enable_dlss", "false")
    for k, v in attrs.items():
        if not bp.has_attribute(k):
            print(f"FAIL: {kind} has no attribute {k}", flush=True)
            sys.exit(2)
        bp.set_attribute(k, v)
    q = queue.Queue()
    cam = world.spawn_actor(bp, cam_tf)
    cam.listen(q.put)
    actors.append(cam)
    return cam, q


CATCHERS = "Roads,Sidewalks,RoadLines"  # Town10 lane markings are separate meshes in road cut-outs
CARS = dict(show_only_tags="Car", shadow_catcher_tags=CATCHERS)
cams = {
    "rgb_ref": spawn_cam("sensor.camera.rgb"),
    "sem_ref": spawn_cam("sensor.camera.semantic_segmentation"),
    "rgb_cars": spawn_cam("sensor.camera.rgb", **CARS),
    "sem_cars": spawn_cam("sensor.camera.semantic_segmentation", **CARS),
    "depth_cars": spawn_cam("sensor.camera.depth", **CARS),
    "rgb_catch": spawn_cam("sensor.camera.rgb", show_only_tags=CATCHERS),
    "rgb_carsonly": spawn_cam("sensor.camera.rgb", show_only_tags="Car"),
}
if a.fisheye:
    # Wide-angle (cubemap) family: same attributes, all six faces filtered.
    cams["fish_ref"] = spawn_cam("sensor.camera.rgb_fisheye", fov="150")
    cams["fish_cars"] = spawn_cam("sensor.camera.rgb_fisheye", fov="150", **CARS)


def arr(img):
    return np.frombuffer(img.raw_data, dtype=np.uint8).reshape((img.height, img.width, 4))


def capture():
    fr = {}
    for k, (cam, q) in cams.items():
        img = q.get(timeout=60)
        while not q.empty():
            img = q.get_nowait()
        fr[k] = img
    return fr


try:
    t0 = time.time()
    for i in range(a.warmup):
        world.tick()
        capture()
    world.tick()
    fr = capture()
    frames = {k: v.frame for k, v in fr.items()}
    print(f"captured after {a.warmup} warm-up ticks in {time.time() - t0:.1f} s; frames {sorted(set(frames.values()))}", flush=True)

    rgb = {k: arr(fr[k])[:, :, :3][:, :, ::-1].copy() for k in cams if k.startswith(("rgb_", "fish_"))}
    sem_ref = arr(fr["sem_ref"])[:, :, 2].copy()
    sem_cars = arr(fr["sem_cars"])[:, :, 2].copy()
    d = arr(fr["depth_cars"]).astype(np.float64)
    depth = (d[:, :, 2] + d[:, :, 1] * 256 + d[:, :, 0] * 65536) / 16777215.0 * 1000.0

    ok = True

    def check(cond, msg):
        global ok
        print(("PASS " if cond else "FAIL ") + msg, flush=True)
        ok = ok and cond

    ref_labels = {int(v): int(c) for v, c in zip(*np.unique(sem_ref, return_counts=True))}
    cars_labels = {int(v): int(c) for v, c in zip(*np.unique(sem_cars, return_counts=True))}
    print("ref labels: " + ", ".join(f"{LABELS.get(k, k)}={v}" for k, v in sorted(ref_labels.items())))
    print("show-only labels: " + ", ".join(f"{LABELS.get(k, k)}={v}" for k, v in sorted(cars_labels.items())))
    allowed = {0, 1, 2, 11, 14, 24}
    extra = {LABELS.get(k, k): v for k, v in cars_labels.items() if k not in allowed}
    check(not extra, f"show-only semantic image holds only Sky/Roads/Sidewalks/RoadLines/Car (extra: {extra})")

    car_ref = sem_ref == 14
    car_so = sem_cars == 14
    inter = np.logical_and(car_ref, car_so).sum()
    union = np.logical_or(car_ref, car_so).sum()
    check(car_so.sum() > 2000, f"car present in show-only image: {int(car_so.sum())} px (ref {int(car_ref.sum())} px)")
    check(union > 0 and inter / union > 0.9, f"car mask IoU ref vs show-only = {inter / max(union, 1):.3f}")

    clutter = np.isin(sem_ref, [3, 4, 5, 6, 7, 8, 9, 10, 20, 21, 22, 24, 25, 28])
    clutter_remaining = np.isin(sem_cars[clutter], list(set(range(1, 30)) - {1, 2, 11, 14, 24})).sum()
    check(clutter_remaining == 0,
          f"buildings/vegetation/poles pixels of the reference ({int(clutter.sum())}) carry no such label in show-only ({int(clutter_remaining)})")

    empty = np.isin(sem_cars, [0, 11])
    check(empty.sum() > 0 and np.all(depth[empty] > 990.0),
          f"empty pixels ({int(empty.sum())}, label Sky) read far depth: min {depth[empty].min() if empty.any() else -1:.1f} m")
    car_depth = depth[car_so]
    print(f"car depth: min {car_depth.min():.2f} m, median {np.median(car_depth):.2f} m")

    # What the colour capture shows where nothing is rendered.
    for k in ("rgb_cars", "rgb_carsonly"):
        m = rgb[k][empty].mean(axis=0)
        print(f"{k} empty-pixel mean colour RGB = {m.round(1)} (ref there: {rgb['rgb_ref'][empty].mean(axis=0).round(1)})")
    lum_carsonly = rgb["rgb_carsonly"].astype(np.float32).mean(axis=2)
    road_in_carsonly = lum_carsonly[np.isin(sem_cars, [1, 2, 24])]
    print(f"cars-only image, luminance where the catchers would be: mean {road_in_carsonly.mean():.1f}")

    # Cast-shadow ratio on the catchers.
    catch_mask = np.isin(sem_cars, [1, 2, 24])
    num = rgb["rgb_cars"].astype(np.float32).mean(axis=2)
    den = rgb["rgb_catch"].astype(np.float32).mean(axis=2)
    ratio = np.ones_like(num)
    valid = catch_mask & (den > 8)
    ratio[valid] = num[valid] / den[valid]
    ratio = np.clip(ratio, 0, 2)
    # Region near the car footprint (dilated bbox) vs far away.
    ys, xs = np.nonzero(car_so)
    y0, y1, x0, x1 = ys.min(), ys.max(), xs.min(), xs.max()
    near = np.zeros_like(valid)
    near[max(0, y0 - 40):min(a.h, y1 + 120), max(0, x0 - 160):min(a.w, x1 + 160)] = True
    near &= valid
    far = valid & ~near
    shadow_px = int((ratio[near] < 0.75).sum())
    check(shadow_px > 500, f"shadow under the car: {shadow_px} catcher px with ratio < 0.75 near the car (min ratio {ratio[near].min():.2f})")
    far_mean, far_std = float(ratio[far].mean()), float(ratio[far].std())
    check(abs(far_mean - 1.0) < 0.03 and far_std < 0.05,
          f"ratio away from the car is ~1: mean {far_mean:.3f} std {far_std:.3f} over {int(far.sum())} px")

    # Fisheye family: the show-only image must drop most of the scene (the
    # buildings become sky) while keeping the car region unchanged.
    if a.fisheye:
        fdiff = np.abs(rgb["fish_cars"].astype(np.int16) - rgb["fish_ref"].astype(np.int16)).max(axis=2)
        changed = float((fdiff > 24).mean())
        check(0.2 < changed < 0.95, f"rgb_fisheye show-only differs from its reference on {changed * 100:.1f} % of the pixels")

    # Outputs.
    def save(name, im):
        p = os.path.join(a.out, f"showonly_{name}.png")
        Image.fromarray(im).save(p)
        return p

    palette = np.zeros((256, 3), np.uint8)
    rng = np.random.RandomState(3)
    palette[1:30] = rng.randint(40, 255, (29, 3))
    palette[1] = (128, 64, 128); palette[2] = (244, 35, 232); palette[14] = (0, 0, 142); palette[3] = (70, 70, 70)
    ratio_img = np.clip(ratio * 255, 0, 255).astype(np.uint8)
    ratio_rgb = np.stack([ratio_img] * 3, axis=2)
    depth_img = np.clip(depth / 60.0 * 255, 0, 255).astype(np.uint8)
    panels = [
        ("reference rgb", rgb["rgb_ref"]),
        ("show_only=Car + catchers Roads,Sidewalks,RoadLines", rgb["rgb_cars"]),
        ("show_only=Roads,Sidewalks,RoadLines", rgb["rgb_catch"]),
        ("show_only=Car, no catchers", rgb["rgb_carsonly"]),
        ("semantic, show-only", palette[sem_cars]),
        ("depth, show-only (0-60 m)", np.stack([depth_img] * 3, axis=2)),
        ("shadow ratio cars/catchers", ratio_rgb),
        ("semantic, reference", palette[sem_ref]),
    ]
    if a.fisheye:
        panels += [("rgb_fisheye 150deg, reference", rgb["fish_ref"]),
                   ("rgb_fisheye 150deg, show_only=Car + catchers", rgb["fish_cars"])]
    for name, im in (("ref", rgb["rgb_ref"]), ("cars", rgb["rgb_cars"]), ("catch", rgb["rgb_catch"]),
                     ("carsonly", rgb["rgb_carsonly"]), ("sem", palette[sem_cars]), ("ratio", ratio_rgb),
                     ("depth", np.stack([depth_img] * 3, axis=2)),
                     *(((("fisheye_ref", rgb["fish_ref"]), ("fisheye_cars", rgb["fish_cars"])) if a.fisheye else ()))):
        save(name, im)
    tw, th = a.w // 2, a.h // 2
    rows = (len(panels) + 3) // 4
    sheet = Image.new("RGB", (tw * 4, th * rows), (0, 0, 0))
    dr = ImageDraw.Draw(sheet)
    for i, (label, im) in enumerate(panels):
        tile = Image.fromarray(im).resize((tw, th))
        x, y = (i % 4) * tw, (i // 4) * th
        sheet.paste(tile, (x, y))
        dr.rectangle([x, y, x + 8 + 7 * len(label), y + 18], fill=(0, 0, 0))
        dr.text((x + 4, y + 3), label, fill=(255, 255, 0))
    p = save("sheet", np.asarray(sheet))
    print(f"sheet: {p}")
    print("RESULT: " + ("OK" if ok else "FAILED"), flush=True)
finally:
    for act in actors:
        try:
            if hasattr(act, "stop"):
                act.stop()
            act.destroy()
        except Exception:
            pass
    settings.synchronous_mode = False
    settings.fixed_delta_seconds = None
    world.apply_settings(settings)
sys.exit(0 if ok else 1)
