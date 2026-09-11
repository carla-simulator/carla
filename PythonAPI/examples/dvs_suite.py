#!/usr/bin/env python3
"""DVS test suite: F4 sync mode, F5 attribute effects, F7 resolution/FOV, F8 lifecycle.

Run against a live CARLA UE5 server. Each test prints PASS/FAIL; exit code is
the number of failures.
"""
import argparse
import time

import numpy as np
import carla

DVS_DTYPE = np.dtype([
    ("x", np.uint16), ("y", np.uint16), ("t", np.int64), ("pol", np.bool_)])


def decode(data):
    return np.frombuffer(data.raw_data, dtype=DVS_DTYPE)


def spawn_rig(world, attrs=None, size=(640, 480), fov=None):
    bp_lib = world.get_blueprint_library()
    dvs_bp = bp_lib.find("sensor.camera.dvs")
    dvs_bp.set_attribute("image_size_x", str(size[0]))
    dvs_bp.set_attribute("image_size_y", str(size[1]))
    if fov is not None:
        dvs_bp.set_attribute("fov", str(fov))
    for k, v in (attrs or {}).items():
        dvs_bp.set_attribute(k, str(v))
    veh_bp = bp_lib.filter("vehicle.*")[0]
    spawn = world.get_map().get_spawn_points()[0]
    vehicle = world.try_spawn_actor(veh_bp, spawn)
    assert vehicle is not None, "vehicle spawn failed"
    dvs = world.spawn_actor(
        dvs_bp, carla.Transform(carla.Location(x=1.5, z=2.0)), attach_to=vehicle)
    return vehicle, dvs


def collect_sync(world, dvs, vehicle, ticks, throttle=0.4):
    packets = []
    dvs.listen(lambda d: packets.append((d.frame, d.timestamp, decode(d).copy())))
    for _ in range(ticks):
        vehicle.apply_control(carla.VehicleControl(throttle=throttle))
        world.tick()
        time.sleep(0.02)  # let the stream deliver
    time.sleep(0.5)
    dvs.stop()
    return packets


def test_f4_sync(client, world):
    """One packet per tick, monotone timestamps, delta consistent with fixed dt."""
    print("== F4 sync mode ==")
    vehicle, dvs = spawn_rig(world)
    settings = world.get_settings()
    original = carla.WorldSettings(
        settings.synchronous_mode, settings.no_rendering_mode,
        settings.fixed_delta_seconds or 0.0)
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)
    try:
        # warmup ticks so autopilot-free vehicle settles and stream attaches
        for _ in range(5):
            world.tick()
        packets = collect_sync(world, dvs, vehicle, 40)
    finally:
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = 0.0
        world.apply_settings(settings)
        dvs.destroy()
        vehicle.destroy()

    frames = [p[0] for p in packets]
    stamps = [p[1] for p in packets]
    fails = []
    # one packet per tick: frames strictly increasing, no duplicates
    if len(frames) != len(set(frames)):
        fails.append("duplicate packets for the same frame")
    if len(packets) < 35:
        fails.append(f"only {len(packets)} packets for 40 ticks")
    deltas = np.diff(stamps)
    if not np.allclose(deltas, 0.05, atol=1e-3):
        bad = deltas[~np.isclose(deltas, 0.05, atol=1e-3)]
        # allow gaps that are exact multiples of dt (dropped-empty packets)
        nonmult = [d for d in bad if abs(d / 0.05 - round(d / 0.05)) > 0.02]
        if nonmult:
            fails.append(f"timestamp deltas not multiple of dt: {nonmult[:5]}")
    # event timestamps monotone across packets
    all_t = np.concatenate([p[2]["t"] for p in packets if len(p[2])])
    if not (np.diff(all_t) >= 0).all():
        fails.append("event timestamps not monotone across packets")
    print(f"  packets={len(packets)} ticks=40 stamp_delta_median="
          f"{np.median(deltas):.4f}")
    print("  F4:", "FAIL: " + "; ".join(fails) if fails else "PASS")
    return not fails


def run_count(client, world, attrs, ticks=30):
    vehicle, dvs = spawn_rig(world, attrs=attrs)
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)
    try:
        for _ in range(5):
            world.tick()
        packets = collect_sync(world, dvs, vehicle, ticks)
    finally:
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = 0.0
        world.apply_settings(settings)
        dvs.destroy()
        vehicle.destroy()
    events = sum(len(p[2]) for p in packets)
    return events, packets


def test_f5_attributes(client, world):
    """Attribute effects go the expected direction."""
    print("== F5 attribute effects ==")
    fails = []

    counts = {}
    for th in (0.1, 0.3, 0.7):
        n, _ = run_count(client, world,
                         {"positive_threshold": th, "negative_threshold": th})
        counts[th] = n
        print(f"  threshold={th}: {n} events")
    if not (counts[0.1] > counts[0.3] > counts[0.7]):
        fails.append(f"threshold not monotone: {counts}")

    n_free, _ = run_count(client, world, {"refractory_period_ns": 0})
    n_refr, _ = run_count(client, world, {"refractory_period_ns": 10_000_000})
    print(f"  refractory 0: {n_free} events, 10ms: {n_refr} events")
    if not n_refr < n_free:
        fails.append(f"refractory period did not reduce events ({n_free} -> {n_refr})")

    n_log, p_log = run_count(client, world, {"use_log": True})
    n_lin, p_lin = run_count(client, world, {"use_log": False})
    print(f"  use_log=true: {n_log} events, false: {n_lin} events")
    rel = abs(n_log - n_lin) / max(n_log, n_lin, 1)
    if rel < 0.02:
        fails.append(f"use_log had no measurable effect (rel diff {rel:.3f})")

    print("  F5:", "FAIL: " + "; ".join(fails) if fails else "PASS")
    return not fails


def test_f7_resolution(client, world):
    """1920x1080 fov=120: coords in bounds, both dims exercised."""
    print("== F7 resolution/FOV ==")
    vehicle, dvs = spawn_rig(world, size=(1920, 1080), fov=120)
    packets = []
    dvs.listen(lambda d: packets.append((decode(d).copy(), d.width, d.height)))
    vehicle.set_autopilot(True)
    deadline = time.time() + 20
    while len(packets) < 20 and time.time() < deadline:
        time.sleep(0.2)
    dvs.stop()
    dvs.destroy()
    vehicle.destroy()
    fails = []
    if not packets:
        fails.append("no packets")
    else:
        ev = np.concatenate([p[0] for p in packets if len(p[0])])
        w, h = packets[0][1], packets[0][2]
        if (w, h) != (1920, 1080):
            fails.append(f"unexpected size {w}x{h}")
        if ev["x"].max() >= w or ev["y"].max() >= h:
            fails.append(f"OOB events: max=({ev['x'].max()},{ev['y'].max()})")
        # events should span most of the sensor, not a squashed band
        if ev["x"].max() < w * 0.8 or ev["y"].max() < h * 0.6:
            fails.append(f"events confined: max=({ev['x'].max()},{ev['y'].max()})")
        print(f"  packets={len(packets)} events={len(ev)} "
              f"span=({ev['x'].max()},{ev['y'].max()}) of {w}x{h}")
    print("  F7:", "FAIL: " + "; ".join(fails) if fails else "PASS")
    return not fails


def test_f8_lifecycle(client, world):
    """stop/listen cycles, destroy-while-streaming, respawn loop x10."""
    print("== F8 lifecycle ==")
    fails = []
    bp_lib = world.get_blueprint_library()
    veh_bp = bp_lib.filter("vehicle.*")[0]
    spawn = world.get_map().get_spawn_points()[0]
    vehicle = world.try_spawn_actor(veh_bp, spawn)
    assert vehicle
    vehicle.set_autopilot(True)

    # stop -> listen again on the same actor
    dvs_bp = bp_lib.find("sensor.camera.dvs")
    dvs = world.spawn_actor(
        dvs_bp, carla.Transform(carla.Location(x=1.5, z=2.0)), attach_to=vehicle)
    got = []
    dvs.listen(lambda d: got.append(1))
    time.sleep(3)
    dvs.stop()
    n1 = len(got)
    got.clear()
    dvs.listen(lambda d: got.append(1))
    time.sleep(3)
    dvs.stop()
    n2 = len(got)
    print(f"  first listen: {n1} packets, re-listen: {n2} packets")
    if n1 == 0 or n2 == 0:
        fails.append(f"stop/re-listen broken ({n1}, {n2})")

    # destroy while streaming
    dvs.listen(lambda d: got.append(1))
    time.sleep(1)
    dvs.destroy()

    # respawn loop
    for i in range(10):
        d = world.spawn_actor(
            dvs_bp, carla.Transform(carla.Location(x=1.5, z=2.0)),
            attach_to=vehicle)
        cnt = []
        d.listen(lambda x: cnt.append(1))
        time.sleep(0.8)
        d.destroy()
    vehicle.destroy()

    # server still alive?
    try:
        client.get_server_version()
        world.get_snapshot()
    except Exception as exc:
        fails.append(f"server unresponsive after lifecycle churn: {exc}")

    print("  F8:", "FAIL: " + "; ".join(fails) if fails else "PASS")
    return not fails


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=3000)
    ap.add_argument("--only", nargs="*", default=None,
                    help="subset like f4 f5 f7 f8")
    args = ap.parse_args()
    client = carla.Client("localhost", args.port)
    client.set_timeout(30.0)
    world = client.get_world()
    print("server:", client.get_server_version())

    tests = {"f4": test_f4_sync, "f5": test_f5_attributes,
             "f7": test_f7_resolution, "f8": test_f8_lifecycle}
    run = args.only or list(tests)
    failures = 0
    for name in run:
        try:
            ok = tests[name](client, world)
        except Exception as exc:
            print(f"  {name.upper()}: ERROR {type(exc).__name__}: {exc}")
            ok = False
            # restore async mode if a sync test blew up mid-way
            s = world.get_settings()
            if s.synchronous_mode:
                s.synchronous_mode = False
                s.fixed_delta_seconds = 0.0
                world.apply_settings(s)
        failures += 0 if ok else 1
    print(f"== suite done: {len(run) - failures}/{len(run)} passed ==")
    return failures


if __name__ == "__main__":
    raise SystemExit(main())
