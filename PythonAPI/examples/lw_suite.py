#!/usr/bin/env python3
"""Light/weather suite: L6 L7 L8 L9 L10, W2 W3, PW1 PW2.

Run against a live CARLA UE5 server on Town10HD_Opt. Each test prints
PASS/FAIL; exit code = number of failures. L10 reloads the world — always last.
"""
import argparse
import time

import numpy as np
from PIL import Image
import carla

OUT = "."


def snap(world, tag, loc, rot):
    bp = world.get_blueprint_library().find("sensor.camera.rgb")
    bp.set_attribute("image_size_x", "960")
    bp.set_attribute("image_size_y", "540")
    cam = world.spawn_actor(bp, carla.Transform(loc, rot))
    holder = []
    cam.listen(lambda img: holder.append(img) if not holder else None)
    deadline = time.time() + 10
    while not holder and time.time() < deadline:
        time.sleep(0.1)
    cam.stop()
    cam.destroy()
    assert holder, f"no frame for {tag}"
    img = holder[0]
    arr = np.frombuffer(img.raw_data, dtype=np.uint8).reshape(
        img.height, img.width, 4)[:, :, [2, 1, 0]].copy()
    Image.fromarray(arr).save(f"{OUT}/lw_{tag}.png")
    return arr


def cam_pose(lm):
    lights = lm.get_all_lights(carla.LightGroup.Street)
    ref = lights[0].location
    loc = carla.Location(x=ref.x + 15.0, y=ref.y, z=ref.z + 2.0)
    rot = carla.Rotation(pitch=-8, yaw=180)
    return loc, rot


def test_l6_state_roundtrip(client, world):
    """set_intensity / set_color / set_light_group round-trip through the API."""
    print("== L6 intensity/color/group round-trip ==")
    lm = world.get_lightmanager()
    lights = lm.get_all_lights(carla.LightGroup.Street)[:5]
    fails = []

    lm.set_intensity(lights, 12345.0)
    time.sleep(1)
    got = [l.intensity for l in lm.get_all_lights(carla.LightGroup.Street)[:5]]
    if not all(abs(v - 12345.0) < 1.0 for v in got):
        fails.append(f"intensity round-trip: {got}")

    target = carla.Color(255, 40, 10)
    lm.set_color(lights, target)
    time.sleep(1)
    got = [l.color for l in lm.get_all_lights(carla.LightGroup.Street)[:5]]
    if not all(c.r == 255 and c.g == 40 and c.b == 10 for c in got):
        fails.append(f"color round-trip: {[(c.r, c.g, c.b) for c in got]}")

    ids = {l.id for l in lights}
    lm.set_light_group(lights, carla.LightGroup.Building)
    time.sleep(1)
    moved = {l.id for l in lm.get_all_lights(carla.LightGroup.Building)} & ids
    if len(moved) != len(ids):
        fails.append(f"group round-trip: {len(moved)}/{len(ids)} moved")
    # restore
    back = [l for l in lm.get_all_lights(carla.LightGroup.Building)
            if l.id in ids]
    lm.set_light_group(back, carla.LightGroup.Street)
    lm.set_intensity(back, 0.0)  # 0 = authored default? leave visible sanity to L2 rerun

    print("  L6:", "FAIL: " + "; ".join(fails) if fails else "PASS")
    return not fails


def street_on_count(lm):
    return sum(1 for l in lm.get_all_lights(carla.LightGroup.Street) if l.is_on)


def test_l7_day_night_cycle_off(client, world):
    """set_day_night_cycle(False): weather stops driving lights; manual works."""
    print("== L7 day/night cycle off ==")
    lm = world.get_lightmanager()
    fails = []
    world.set_weather(carla.WeatherParameters.ClearNoon)
    time.sleep(2)
    lm.set_day_night_cycle(False)
    time.sleep(1)
    base = street_on_count(lm)
    world.set_weather(carla.WeatherParameters.ClearNight)
    time.sleep(3)
    after_night = street_on_count(lm)
    print(f"  cycle off: day on={base}, after ClearNight on={after_night}")
    if after_night != base:
        fails.append(f"weather still drives lights with cycle off "
                     f"({base} -> {after_night})")
    # manual control must still work
    lights = lm.get_all_lights(carla.LightGroup.Street)
    lm.turn_on(lights)
    time.sleep(1)
    n_on = street_on_count(lm)
    if n_on < len(lights):
        fails.append(f"manual turn_on with cycle off: {n_on}/{len(lights)}")
    lm.set_day_night_cycle(True)
    world.set_weather(carla.WeatherParameters.ClearNoon)
    time.sleep(2)
    print("  L7:", "FAIL: " + "; ".join(fails) if fails else "PASS")
    return not fails


def test_l8_vehicle_lights(client, world):
    """Headlight beams illuminate the road at night."""
    print("== L8 vehicle lights at night ==")
    fails = []
    lm = world.get_lightmanager()
    world.set_weather(carla.WeatherParameters.ClearNight)
    time.sleep(2)
    # kill street lights so headlights dominate the measurement
    lights = lm.get_all_lights(carla.LightGroup.Street)
    lm.set_day_night_cycle(False)
    lm.turn_off(lights)
    time.sleep(1)

    bp_lib = world.get_blueprint_library()
    veh_bp = bp_lib.filter("vehicle.*")[0]
    spawn = world.get_map().get_spawn_points()[0]
    vehicle = world.try_spawn_actor(veh_bp, spawn)
    assert vehicle, "vehicle spawn failed"
    time.sleep(1)

    tf = vehicle.get_transform()
    fwd = tf.get_forward_vector()
    loc = carla.Location(tf.location.x - 6 * fwd.x, tf.location.y - 6 * fwd.y,
                         tf.location.z + 3.0)
    rot = carla.Rotation(pitch=-12, yaw=tf.rotation.yaw)

    off_img = snap(world, "l8_beams_off", loc, rot)
    st = carla.VehicleLightState(
        carla.VehicleLightState.LowBeam | carla.VehicleLightState.HighBeam)
    vehicle.set_light_state(st)
    time.sleep(2)
    on_img = snap(world, "l8_beams_on", loc, rot)
    vehicle.destroy()
    lm.set_day_night_cycle(True)
    world.set_weather(carla.WeatherParameters.ClearNoon)
    time.sleep(1)

    mean_off = float(off_img.mean())
    mean_on = float(on_img.mean())
    print(f"  scene mean brightness: beams off={mean_off:.2f} on={mean_on:.2f}")
    if mean_on <= mean_off * 1.05:
        fails.append(f"beams do not illuminate ({mean_off:.2f} -> {mean_on:.2f})")
    print("  L8:", "FAIL: " + "; ".join(fails) if fails else "PASS")
    return not fails


def test_l9_recorder(client, world):
    """Record light changes, replay, states restored."""
    print("== L9 recorder/replayer ==")
    fails = []
    lm = world.get_lightmanager()
    lights = lm.get_all_lights(carla.LightGroup.Street)
    world.set_weather(carla.WeatherParameters.ClearNoon)
    lm.set_day_night_cycle(False)
    lm.turn_off(lights)
    time.sleep(1)

    rec = f"{OUT}/l9_lights.rec"
    client.start_recorder(rec, True)
    time.sleep(1)
    lm.turn_on(lights)          # recorded event: all on
    time.sleep(2)
    client.stop_recorder()

    lm.turn_off(lights)         # diverge from recording
    time.sleep(1)
    n_before = street_on_count(lm)

    client.replay_file(rec, 0.0, 0.0, 0)
    time.sleep(4)
    n_after = street_on_count(lm)
    client.stop_replayer(False)
    time.sleep(2)
    print(f"  on before replay={n_before}, after replay={n_after}")
    if n_after <= n_before:
        fails.append(f"replay did not restore light-on states "
                     f"({n_before} -> {n_after})")
    lm2 = client.get_world().get_lightmanager()
    lm2.set_day_night_cycle(True)
    client.get_world().set_weather(carla.WeatherParameters.ClearNoon)
    print("  L9:", "FAIL: " + "; ".join(fails) if fails else "PASS")
    return not fails


def test_w2_param_sweep(client, world):
    """Each individual parameter visibly changes the scene + round-trips."""
    print("== W2 parameter sweep ==")
    fails = []
    lm = world.get_lightmanager()
    loc, rot = cam_pose(lm)
    base_wp = carla.WeatherParameters.ClearNoon
    world.set_weather(base_wp)
    time.sleep(2)
    base = snap(world, "w2_base", loc, rot).astype(np.int16)

    sweeps = {
        "cloudiness": 100.0,
        "precipitation": 100.0,
        "precipitation_deposits": 100.0,
        "wind_intensity": 100.0,
        "fog_density": 80.0,
        "fog_distance": 5.0,
        "fog_falloff": 2.0,
        "wetness": 100.0,
        "scattering_intensity": 8.0,
        "mie_scattering_scale": 0.5,
        "dust_storm": 80.0,
    }
    for name, value in sweeps.items():
        wp = carla.WeatherParameters.ClearNoon
        setattr(wp, name, value)
        world.set_weather(wp)
        time.sleep(2.5)
        back = getattr(world.get_weather(), name)
        img = snap(world, f"w2_{name}", loc, rot).astype(np.int16)
        diff = float(np.abs(img - base).mean())
        rt_ok = abs(back - value) < 0.5
        vis_ok = diff > 1.0
        print(f"  {name}={value}: readback={back:.2f} imgdiff={diff:.2f}"
              f"{'' if rt_ok else ' RT-FAIL'}{'' if vis_ok else ' VIS-FAIL'}")
        if not rt_ok:
            fails.append(f"{name} readback {back} != {value}")
        if not vis_ok:
            fails.append(f"{name} no visible effect (diff {diff:.2f})")
    world.set_weather(base_wp)
    print("  W2:", "FAIL: " + "; ".join(fails) if fails else "PASS")
    return not fails


def test_w3_all_presets(client, world):
    """Every preset renders sanely (no black screen, no collapsed sky)."""
    print("== W3 all presets ==")
    fails = []
    lm = world.get_lightmanager()
    loc, rot = cam_pose(lm)
    presets = [n for n in dir(carla.WeatherParameters)
               if n[0].isupper() and isinstance(
                   getattr(carla.WeatherParameters, n), carla.WeatherParameters)]
    print("  presets:", len(presets))
    for name in presets:
        world.set_weather(getattr(carla.WeatherParameters, name))
        time.sleep(2.5)
        img = snap(world, f"w3_{name}", loc, rot)
        mean = float(img.mean())
        floor = 0.5 if "Night" in name else 3.0
        status = "ok" if mean > floor else "BLACK?"
        print(f"  {name}: mean={mean:.2f} {status}")
        if mean <= floor:
            fails.append(f"{name} renders near-black (mean {mean:.2f})")
    world.set_weather(carla.WeatherParameters.ClearNoon)
    time.sleep(1)
    print("  W3:", "FAIL: " + "; ".join(fails) if fails else "PASS")
    return not fails


def test_pw1_broadcast_cost(client, world):
    """set_weather RPC latency with all lights + a second client connected."""
    print("== PW1 broadcast cost ==")
    second = carla.Client("localhost", 3000)
    second.set_timeout(10.0)
    second.get_world().get_lightmanager()  # force registration as light client
    lat = []
    for i in range(10):
        wp = (carla.WeatherParameters.ClearNight if i % 2 else
              carla.WeatherParameters.ClearNoon)
        t0 = time.perf_counter()
        world.set_weather(wp)
        lat.append((time.perf_counter() - t0) * 1000)
    world.set_weather(carla.WeatherParameters.ClearNoon)
    print(f"  set_weather latency ms: median={np.median(lat):.1f} "
          f"max={max(lat):.1f}")
    ok = np.median(lat) < 500
    print("  PW1:", "PASS" if ok else f"FAIL: median {np.median(lat):.1f} ms")
    return ok


def test_pw2_rapid_changes(client, world):
    """100 alternating day/night: no drift, is_on consistent at the end."""
    print("== PW2 rapid weather changes ==")
    lm = world.get_lightmanager()
    n_total = len(lm.get_all_lights(carla.LightGroup.Street))
    for i in range(100):
        wp = (carla.WeatherParameters.ClearNight if i % 2 == 0 else
              carla.WeatherParameters.ClearNoon)
        world.set_weather(wp)
        time.sleep(0.05)
    # ended on ClearNoon (i=99 odd) -> lights off expected
    time.sleep(3)
    n_on = street_on_count(lm)
    n_now = len(lm.get_all_lights(carla.LightGroup.Street))
    print(f"  after 100 flips: lights={n_now} (was {n_total}), on={n_on} "
          f"(expect 0, weather=ClearNoon)")
    fails = []
    if n_now != n_total:
        fails.append(f"light count drift {n_total} -> {n_now}")
    if n_on != 0:
        fails.append(f"{n_on} lights stuck on after final ClearNoon")
    print("  PW2:", "FAIL: " + "; ".join(fails) if fails else "PASS")
    return not fails


def test_l10_map_reload(client, world):
    """Registration and is_on survive load_world. Runs LAST."""
    print("== L10 map reload ==")
    fails = []
    lm = world.get_lightmanager()
    n_before = len(lm.get_all_lights(carla.LightGroup.Street))
    world2 = client.load_world("Town10HD_Opt")
    time.sleep(10)
    lm2 = world2.get_lightmanager()
    n_after = len(lm2.get_all_lights(carla.LightGroup.Street))
    print(f"  street lights before reload={n_before} after={n_after}")
    if n_after != n_before:
        fails.append(f"light count changed across reload {n_before}->{n_after}")
    world2.set_weather(carla.WeatherParameters.ClearNight)
    time.sleep(3)
    n_on = sum(1 for l in lm2.get_all_lights(carla.LightGroup.Street) if l.is_on)
    print(f"  after reload + ClearNight: on={n_on}")
    if n_on != n_after:
        fails.append(f"day/night broken after reload ({n_on}/{n_after})")
    world2.set_weather(carla.WeatherParameters.ClearNoon)
    print("  L10:", "FAIL: " + "; ".join(fails) if fails else "PASS")
    return not fails


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=3000)
    ap.add_argument("--only", nargs="*", default=None)
    args = ap.parse_args()
    client = carla.Client("localhost", args.port)
    client.set_timeout(60.0)
    world = client.get_world()
    print("server:", client.get_server_version(), "| map:", world.get_map().name)

    tests = {"l6": test_l6_state_roundtrip, "l7": test_l7_day_night_cycle_off,
             "l8": test_l8_vehicle_lights, "l9": test_l9_recorder,
             "w2": test_w2_param_sweep, "w3": test_w3_all_presets,
             "pw1": test_pw1_broadcast_cost, "pw2": test_pw2_rapid_changes,
             "l10": test_l10_map_reload}
    run = args.only or ["l6", "l7", "l8", "w2", "w3", "pw1", "pw2", "l9", "l10"]
    failures = 0
    for name in run:
        try:
            ok = tests[name](client, world)
        except Exception as exc:
            print(f"  {name.upper()}: ERROR {type(exc).__name__}: {exc}")
            ok = False
        failures += 0 if ok else 1
        world = client.get_world()  # refresh in case a test reloaded
    print(f"== suite done: {len(run) - failures}/{len(run)} passed ==")
    return failures


if __name__ == "__main__":
    raise SystemExit(main())
