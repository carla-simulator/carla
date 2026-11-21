"""T3 — the python-api skill scripts, each judged by querying the world afterwards."""

import json
import math
import re
import time

import carla
import numpy as np

from harness.core import test, check, Fail, Skip, SKILLS, PYTHON, REPO, has_traceback
from harness import world as W

SCRIPT = {
    "vehicles": "spawn-vehicles/scripts/vehicles.py",
    "walkers": "spawn-walkers/scripts/walkers.py",
    "weather": "set-weather/scripts/weather.py",
    "settings": "set-world-settings/scripts/world_settings.py",
    "load_map": "load-map/scripts/load_map.py",
    "lights_tl": "control-traffic-lights/scripts/traffic_lights.py",
    "traffic": "control-traffic/scripts/traffic.py",
    "control": "control-vehicle/scripts/vehicle_control.py",
    "navigate": "navigate-to/scripts/navigate.py",
    "sensors": "create-sensor/scripts/sensors.py",
    "read_sensor": "read-sensor/scripts/read_sensor.py",
    "bbox": "bounding-boxes/scripts/bounding_boxes.py",
    "record": "record-simulation/scripts/record.py",
    "query": "query-recording/scripts/query.py",
    "replay": "replay-recording/scripts/replay.py",
    "telemetry": "telemetry/scripts/telemetry.py",
    "spectator": "control-spectator/scripts/spectator.py",
    "waypoints": "map-waypoints/scripts/waypoints.py",
    "world_data": "world-data/scripts/world_data.py",
    "draw": "debug-draw/scripts/debug_draw.py",
    "navmesh": "debug-navmesh/scripts/navmesh.py",
    "env_objects": "toggle-env-objects/scripts/toggle_env.py",
    "lights": "control-lights/scripts/lights.py",
}


def skill(ctx, name, *args, timeout=180, ok_rc=(0,), env=None):
    rc, out = ctx.sh([PYTHON, SKILLS / "python-api" / SCRIPT[name], *args],
                     timeout=timeout, ok_rc=ok_rc, env=env)
    check(not has_traceback(out), f"{name} {' '.join(args[:2])} printed a traceback:\n{out[-1500:]}")
    return out


def skill_bg(ctx, name, *args):
    return ctx.bg([PYTHON, SKILLS / "python-api" / SCRIPT[name], *args])


def xyz(loc):
    return f"{loc.x:.2f},{loc.y:.2f},{loc.z:.2f}"


def spawn_point(world, i=0):
    return world.get_map().get_spawn_points()[i]


def ego(ctx, at=None):
    """Spawn the hero with the skill and return the actor."""
    args = ["ego"] + (["--at", xyz(at)] if at else [])
    out = skill(ctx, "vehicles", *args)
    m = re.search(r"spawned ego id=(\d+)", out)
    check(m, f"ego spawn printed no id:\n{out}")
    world = ctx.world()
    world.wait_for_tick(5.0)
    hero = world.get_actor(int(m.group(1)))
    check(hero is not None and hero.attributes.get("role_name") == "hero", "hero not found in world")
    return hero


def held_traffic(ctx, n, seed=7):
    """Start `vehicles.py spawn --hold` (sync, TM) and wait until it holds."""
    p = skill_bg(ctx, "vehicles", "spawn", "--count", n, "--seed", seed, "--safe", "--hold")
    p.wait_for("holding", 180)
    return p


def mean_speed(world, samples=10, gap=1.0, role="autopilot"):
    vals = []
    for _ in range(samples):
        vs = [W.speed(a) for a in world.get_actors().filter("vehicle.*")
              if a.attributes.get("role_name") == role]
        if vs:
            vals.append(sum(vs) / len(vs))
        time.sleep(gap)
    return sum(vals) / len(vals) if vals else 0.0


# ---------------------------------------------------------------------------

@test("T3.check-env")
def check_env(ctx):
    blocked = []
    dirs = sorted(SKILLS.glob("python-api/*/scripts/check_env.sh"))
    for f in dirs:
        rc, out = ctx.sh(["bash", f], ok_rc=None, timeout=120)
        if rc != 0:
            blocked.append(f"{f.parts[-3]}: " + "; ".join(l.strip() for l in out.splitlines()
                                                         if l.strip().startswith("FAIL")))
    check(not blocked, "check_env BLOCKED:\n" + "\n".join(blocked))
    return {"skills": len(dirs)}


@test("T3.world-data")
def world_data(ctx):
    sp = spawn_point(ctx.world())
    for args in (["actors", "--all", "--limit", "20"], ["snapshot"],
                 ["ground", "--at", xyz(sp.location + carla.Location(z=30))],
                 ["raycast", "--from", xyz(sp.location + carla.Location(z=20)),
                  "--to", xyz(sp.location - carla.Location(z=5))],
                 ["ros-topics"]):
        out = skill(ctx, "world_data", *args)
        check(out.strip(), f"world_data {args[0]} printed nothing")


@test("T3.load-map")
def load_map(ctx):
    out = skill(ctx, "load_map", "list")
    check("Town03_Opt" in out, "Town03_Opt not in the map list")
    before = ctx.world().id
    # The script's default 60 s RPC timeout is too short for some loads on an
    # uncooked -game server; give it room and report the time as a metric.
    env = ctx.env(CARLA_TIMEOUT=600)
    t = time.time()
    skill(ctx, "load_map", "load", "--map", "Town03_Opt", timeout=700, env=env)
    load_s = time.time() - t
    ctx.client().set_timeout(120.0)
    w = ctx.world()
    check(w.get_map().name.endswith("Town03_Opt"), f"map is {w.get_map().name}")
    check(w.id != before, "world id did not change after load")
    skill(ctx, "load_map", "load", "--map", "Town10HD_Opt", timeout=700, env=env)
    check(ctx.world().get_map().name.endswith("Town10HD_Opt"), "could not load Town10HD_Opt back")
    if load_s > 60:
        ctx.notes.append(f"Town03_Opt took {load_s:.0f}s to load: over the skill's default "
                         "60 s CARLA_TIMEOUT")
    return {"town03_load_s": round(load_s, 1)}


@test("T3.set-world-settings")
def world_settings(ctx):
    skill(ctx, "settings", "sync", "--fps", "20")
    s = ctx.world().get_settings()
    check(s.synchronous_mode and abs((s.fixed_delta_seconds or 0) - 0.05) < 1e-6,
          f"after sync --fps 20: sync={s.synchronous_mode} delta={s.fixed_delta_seconds}")
    out = skill(ctx, "settings", "show")
    check("0.05" in out, "show does not report the 0.05 s delta")
    skill(ctx, "settings", "async")
    s = ctx.world().get_settings()
    check(not s.synchronous_mode, "still synchronous after async")


@test("T3.set-weather")
def weather(ctx):
    out = skill(ctx, "weather", "list-presets")
    check("HardRainSunset" in out, "HardRainSunset missing from presets")
    skill(ctx, "weather", "preset", "HardRainSunset")
    w, ref = ctx.world().get_weather(), carla.WeatherParameters.HardRainSunset
    check(abs(w.precipitation - ref.precipitation) < 0.5 and abs(w.sun_altitude_angle - ref.sun_altitude_angle) < 0.5,
          f"preset not applied: precipitation={w.precipitation} sun={w.sun_altitude_angle}")
    skill(ctx, "weather", "set", "--fog-density", "40", "--sun-altitude-angle", "-10")
    w = ctx.world().get_weather()
    check(abs(w.fog_density - 40) < 0.5 and abs(w.sun_altitude_angle + 10) < 0.5,
          f"set not applied: fog={w.fog_density} sun={w.sun_altitude_angle}")


@test("T3.spawn-vehicles")
def spawn_vehicles(ctx):
    p = held_traffic(ctx, 40)
    world = ctx.world()
    m = re.search(r"spawned (\d+) vehicles", p.output())
    spawned = int(m.group(1)) if m else -1
    n = W.count(world, "vehicle.*", role="autopilot")
    check(n == spawned and n >= 35, f"script says {spawned} spawned, world has {n} (want >= 35)")
    time.sleep(10)
    v = mean_speed(ctx.world(), samples=5)
    rc, out = ctx.stop_bg(p)
    check(v > 1.0, f"autopilot vehicles barely move: mean speed {v:.2f} m/s")
    time.sleep(2)
    left = W.count(ctx.world(), "vehicle.*")
    check(left == 0, f"{left} vehicles remain after releasing --hold")
    return {"spawned": n, "mean_speed_ms": round(v, 2)}


@test("T3.spawn-walkers")
def spawn_walkers(ctx):
    p = skill_bg(ctx, "walkers", "spawn", "--count", "30", "--seed", "3", "--hold")
    p.wait_for("holding", 240)
    world = ctx.world()
    walkers = list(world.get_actors().filter("walker.pedestrian.*"))
    ctrls = W.count(world, "controller.ai.walker")
    check(len(walkers) >= 25 and ctrls == len(walkers),
          f"{len(walkers)} walkers / {ctrls} controllers (want >= 25, equal)")
    start = {a.id: a.get_location() for a in walkers}
    time.sleep(15)
    world = ctx.world()
    moved = [world.get_actor(i).get_location().distance(l) for i, l in start.items()
             if world.get_actor(i) is not None]
    disp = sum(moved) / max(len(moved), 1)
    ctx.stop_bg(p)
    check(disp > 2.0, f"walkers barely move: mean displacement {disp:.2f} m in 15 s")
    time.sleep(2)
    left = W.count(ctx.world(), "walker.*") + W.count(ctx.world(), "controller.*")
    check(left == 0, f"{left} walkers/controllers remain after releasing --hold")
    return {"walkers": len(walkers), "mean_displacement_m": round(disp, 2)}


@test("T3.control-traffic")
def control_traffic(ctx):
    p = held_traffic(ctx, 40, seed=11)
    time.sleep(10)
    before = mean_speed(ctx.world(), samples=15)
    skill(ctx, "traffic", "global", "--speed-diff", "70")
    time.sleep(8)
    after = mean_speed(ctx.world(), samples=15)
    ctx.stop_bg(p)
    check(before > 1.0, f"traffic not moving before the change ({before:.2f} m/s)")
    check(after < 0.75 * before, f"speed-diff 70% had no effect: {before:.2f} -> {after:.2f} m/s")
    return {"before_ms": round(before, 2), "after_ms": round(after, 2)}


@test("T3.control-vehicle")
def control_vehicle(ctx):
    hero = ego(ctx, spawn_point(ctx.world(), 0).location)
    skill(ctx, "control", "control", "--throttle", "1", "--hold", "3")
    v_fast = W.speed(hero)
    skill(ctx, "control", "stop")
    time.sleep(5)
    v_stop = W.speed(hero)
    skill(ctx, "vehicles", "destroy")
    check(v_fast > 5.0, f"full throttle for 3 s only reached {v_fast:.2f} m/s")
    check(v_stop < 0.5, f"still moving at {v_stop:.2f} m/s after stop")
    return {"speed_after_throttle_ms": round(v_fast, 2), "speed_after_stop_ms": round(v_stop, 2)}


@test("T3.navigate-to")
def navigate_to(ctx):
    world = ctx.world()
    sp = spawn_point(world, 0)
    wp = world.get_map().get_waypoint(sp.location)
    nxt = wp.next(120.0)
    check(nxt, "no waypoint 120 m ahead of spawn point 0")
    goal = nxt[0].transform.location
    hero = ego(ctx, sp.location)
    skill(ctx, "navigate", "go", "--to", xyz(goal), "--agent", "basic", "--seconds", "90",
          timeout=600)
    d = hero.get_location().distance(goal)
    skill(ctx, "vehicles", "destroy")
    check(d < 5.0, f"ended {d:.1f} m from the goal")
    return {"final_distance_m": round(d, 2)}


@test("T3.telemetry")
def telemetry(ctx):
    hero = ego(ctx)
    out = skill(ctx, "telemetry", "show", "--role", "hero")
    check(str(hero.id) in out, "telemetry show does not mention the hero id")
    out = skill(ctx, "telemetry", "watch", "--role", "hero", "--seconds", "3", timeout=120)
    check(len(out.splitlines()) >= 3, "telemetry watch printed fewer than 3 lines")
    skill(ctx, "vehicles", "destroy")


@test("T3.control-spectator")
def control_spectator(ctx):
    hero = ego(ctx)
    skill(ctx, "spectator", "look", "--role", "hero", "--view", "chase")
    world = ctx.world()
    world.wait_for_tick(5.0)
    d = world.get_spectator().get_location().distance(hero.get_location())
    skill(ctx, "spectator", "follow", "--role", "hero", "--seconds", "3", timeout=120)
    skill(ctx, "vehicles", "destroy")
    check(d < 15.0, f"spectator is {d:.1f} m from the hero after look --view chase")
    return {"distance_m": round(d, 2)}


def _sensor(ctx, typ, *extra):
    out = skill(ctx, "sensors", "spawn", "--type", typ, *extra)
    m = re.search(r"id=(\d+)", out)
    check(m, f"sensor spawn printed no id:\n{out}")
    return int(m.group(1))


@test("T3.create-read-sensor")
def create_read_sensor(ctx):
    ego(ctx)
    cam = _sensor(ctx, "sensor.camera.rgb", "--attach-to", "hero", "--x", "1.5", "--z", "2.4",
                  "--attr", "image_size_x=800", "--attr", "image_size_y=600")
    lidar = _sensor(ctx, "sensor.lidar.ray_cast", "--attach-to", "hero", "--z", "2.5")
    imu = _sensor(ctx, "sensor.other.imu", "--attach-to", "hero")
    hung = []
    for sid, sub in ((cam, "rgb"), (lidar, "lidar"), (imu, "imu")):
        # Allow a timeout so a save that finished but never exits is reported as
        # that, and the frames it wrote are still checked.
        rc, out = ctx.sh([PYTHON, SKILLS / "python-api" / SCRIPT["read_sensor"], "save",
                          "--id", sid, "--out", ctx.out / sub, "--frames", "10"],
                         timeout=120, allow_timeout=True)
        check(not has_traceback(out), f"read_sensor save ({sub}) raised:\n{out[-1500:]}")
        if ctx.last_timed_out:
            check("saved" in out, f"read_sensor save ({sub}) did not finish in 120 s:\n{out[-1500:]}")
            hung.append(sub)
    from PIL import Image
    pngs = sorted((ctx.out / "rgb").glob("*.png"))
    stds = [float(np.asarray(Image.open(p).convert("L")).std()) for p in pngs]
    plys = sorted((ctx.out / "lidar").glob("*.ply"))
    pts = 0
    if plys:
        head = plys[-1].read_bytes()[:400].decode(errors="replace")
        m = re.search(r"element vertex (\d+)", head)
        pts = int(m.group(1)) if m else 0
    jl = ctx.out / "imu" / "data.jsonl"
    rows = len(jl.read_text().splitlines()) if jl.exists() else 0
    skill(ctx, "sensors", "destroy")
    skill(ctx, "vehicles", "destroy")
    check(len(pngs) == 10, f"{len(pngs)} PNGs, want 10")
    check(stds and min(stds) > 5, f"blank camera frame (pixel std {stds})")
    check(len(plys) == 10 and pts > 1000, f"{len(plys)} PLYs, last has {pts} points")
    check(rows == 10, f"imu data.jsonl has {rows} rows, want 10")
    check(not hung, f"read_sensor save finished but the process never exited ({', '.join(hung)}); "
                    "killed after 120 s")
    return {"min_pixel_std": round(min(stds), 1), "lidar_points": pts}


@test("T3.bounding-boxes")
def bounding_boxes(ctx):
    world = ctx.world()
    sp = spawn_point(world, 0)
    skill(ctx, "vehicles", "line", "--at", xyz(sp.location), "--count", "5", "--gap", "10",
          "--no-autopilot", "--safe")
    fwd = sp.get_forward_vector()
    loc = sp.location - fwd * 12 + carla.Location(z=4)
    cam = _sensor(ctx, "sensor.camera.rgb", "--x", f"{loc.x:.2f}", "--y", f"{loc.y:.2f}",
                  "--z", f"{loc.z:.2f}", "--yaw", f"{sp.rotation.yaw:.1f}", "--pitch", "-12",
                  "--attr", "image_size_x=1280", "--attr", "image_size_y=720")
    out_png = ctx.out / "boxes.png"
    skill(ctx, "bbox", "project", "--camera", str(cam), "--out", out_png, timeout=120)
    skill(ctx, "bbox", "list", "--limit", "5")
    boxes = json.loads(out_png.with_suffix(".json").read_text())
    skill(ctx, "sensors", "destroy")
    skill(ctx, "vehicles", "destroy")
    check(len(boxes) >= 3, f"only {len(boxes)} boxes projected for a row of 5 vehicles")
    bad = [b for b in boxes if b["bbox"][2] < 0 or b["bbox"][3] < 0
           or b["bbox"][0] > 1280 or b["bbox"][1] > 720]
    check(not bad, f"{len(bad)} boxes fall fully outside the image")
    return {"boxes": len(boxes)}


@test("T3.control-traffic-lights")
def traffic_lights(ctx):
    world = ctx.world()
    tls = list(world.get_actors().filter("traffic.traffic_light"))
    check(tls, "no traffic lights on this map")
    tl = tls[0]
    skill(ctx, "lights_tl", "list")
    try:
        for state, want in (("red", carla.TrafficLightState.Red),
                            ("green", carla.TrafficLightState.Green)):
            skill(ctx, "lights_tl", "set", "--state", state, "--id", str(tl.id))
            world.wait_for_tick(5.0)
            check(tl.get_state() == want, f"after set {state}: {tl.get_state()}")
        skill(ctx, "lights_tl", "freeze", "on", "--state", "green")
        world.wait_for_tick(5.0)
        frozen = [t for t in world.get_actors().filter("traffic.traffic_light") if not t.is_frozen()]
        check(not frozen, f"{len(frozen)} lights not frozen after freeze on")
        skill(ctx, "lights_tl", "freeze", "off")
        world.wait_for_tick(5.0)
        check(not tl.is_frozen(), "light still frozen after freeze off")
    finally:
        skill(ctx, "lights_tl", "reset", ok_rc=None)


@test("T3.control-lights")
def control_lights(ctx):
    world = ctx.world()
    lm = world.get_lightmanager()
    street = lm.get_all_lights(carla.LightGroup.Street)
    if not street:
        raise Skip("no street lights on this map")
    initial = lm.is_active(street)
    try:
        # Start from all-on so "off" has something to do (daytime maps start all-off).
        lm.turn_on(street)
        for _ in range(3):
            world.wait_for_tick(5.0)
        skill(ctx, "lights", "off", "--group", "street")
        world = ctx.world()
        lm = world.get_lightmanager()
        street = lm.get_all_lights(carla.LightGroup.Street)
        on = sum(lm.is_active(street))
        check(on == 0, f"{on}/{len(street)} street lights still on after off")
        skill(ctx, "lights", "on", "--group", "street")
        world = ctx.world()
        lm = world.get_lightmanager()
        street = lm.get_all_lights(carla.LightGroup.Street)
        on = sum(lm.is_active(street))
        check(on == len(street), f"only {on}/{len(street)} street lights on after on")
        skill(ctx, "lights", "list", "--group", "street")
    finally:
        lm.set_active(street, initial)
    return {"street_lights": len(street)}


@test("T3.toggle-env-objects")
def toggle_env(ctx):
    skill(ctx, "env_objects", "labels")
    n = len(ctx.world().get_environment_objects(carla.CityObjectLabel.Buildings))
    check(n > 0, "no Buildings environment objects")
    out = skill(ctx, "env_objects", "disable", "--label", "Buildings", "--limit", "5", "--dry-run")
    skill(ctx, "env_objects", "disable", "--label", "Buildings", "--limit", "5")
    skill(ctx, "env_objects", "enable", "--label", "Buildings", "--limit", "5")
    check("5" in out, f"dry-run did not report 5 objects:\n{out}")
    return {"buildings": n}


@test("T3.map-waypoints")
def map_waypoints(ctx):
    sp = spawn_point(ctx.world(), 0)
    for args in (["summary"], ["topology"], ["junctions", "--arms", "3"],
                 ["waypoint", "--at", xyz(sp.location)],
                 ["navigate", "--at", xyz(sp.location), "--steps", "5"],
                 ["landmarks", "--limit", "10"]):
        out = skill(ctx, "waypoints", *args, timeout=300)
        check(len(out.strip().splitlines()) >= 1, f"waypoints {args[0]} printed nothing")


@test("T3.debug-draw")
def debug_draw(ctx):
    p = spawn_point(ctx.world(), 0).location
    a, b = xyz(p + carla.Location(z=2)), xyz(p + carla.Location(x=5, z=2))
    for args in (["point", "--at", a], ["line", "--from", a, "--to", b],
                 ["arrow", "--from", a, "--to", b], ["box", "--center", a, "--extent", "1,1,1"],
                 ["text", "--at", a, "--text", "system-test"]):
        skill(ctx, "draw", *args, "--life", "5", "--color", "0,255,0")


@test("T3.debug-navmesh")
def debug_navmesh(ctx):
    out = skill(ctx, "navmesh", "validate", "--count", "300", ok_rc=None, timeout=300)
    check("PASS" in out and "FAIL" not in out, f"navmesh validate:\n{out[-1500:]}")
    skill(ctx, "navmesh", "sample", "--count", "200", "--life", "5", timeout=300)


@test("T3.record-query-replay")
def record_query_replay(ctx):
    name = "system_test_rec.log"
    p = held_traffic(ctx, 30, seed=5)
    skill(ctx, "record", "clip", "--file", name, "--seconds", "20", timeout=600)
    ctx.stop_bg(p)
    time.sleep(2)
    info = skill(ctx, "query", "info", "--file", name)
    m = re.search(r"Frames:\s*(\d+)", info)
    frames = int(m.group(1)) if m else 0
    check(frames > 300, f"recording has {frames} frames (want > 300 for 20 s at 20 Hz)")
    skill(ctx, "query", "collisions", "--file", name, "--type1", "a", "--type2", "a")
    skill(ctx, "query", "blocked", "--file", name, "--min-time", "5", "--min-distance", "1")
    skill(ctx, "replay", "play", "--file", name, "--duration", "10")
    time.sleep(5)
    replayed = W.count(ctx.world(), "vehicle.*")
    skill(ctx, "replay", "stop")
    time.sleep(3)
    leftover = W.dynamic_ids(ctx.world())
    if leftover:
        W.destroy(ctx.client(), leftover)
    check(25 <= replayed <= 35, f"replay shows {replayed} vehicles, recorded 30")
    check(not leftover, f"{len(leftover)} replayed actors remain after replay stop")
    return {"frames": frames, "replayed_vehicles": replayed,
            "left_after_stop": len(leftover)}


@test("T3.audit-docs")
def audit_docs(ctx):
    rc, out = ctx.sh([PYTHON, SKILLS / "docs" / "audit-carla-docs" / "scripts" / "audit_docs.py",
                      "--root", REPO, "--port", str(ctx.port)], ok_rc=None, timeout=600)
    check(rc == 0 and not has_traceback(out), f"audit_docs rc={rc}:\n{out[-1500:]}")
    (ctx.out / "audit.md").write_text(out)
    return {"report_lines": len(out.splitlines())}


@test("T3.sensors-hold", xfail="sensors.py declares --hold but never reads it")
def sensors_hold(ctx):
    p = ctx.bg([PYTHON, SKILLS / "python-api" / SCRIPT["sensors"], "spawn", "--type",
                "sensor.other.imu", "--hold"])
    time.sleep(10)
    still = p.running()
    ctx.stop_bg(p)
    skill(ctx, "sensors", "destroy")
    check(still, "sensors.py spawn --hold exited on its own instead of holding")
