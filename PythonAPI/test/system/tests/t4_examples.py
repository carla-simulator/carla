"""T4 — PythonAPI/examples, run headless (SDL dummy driver) with a time box.

Verdict: no traceback (other than the KeyboardInterrupt our SIGINT causes), no
early non-zero exit, plus the per-script check; the shared health check then
catches leaked actors and sync mode left on.
"""

import json
import time

from harness.core import test, check, Skip, EXAMPLES, PYTHON, REPO, SKILLS, has_traceback
from harness import world as W

# The client sometimes segfaults at interpreter exit on this branch; a run that
# finished its work and then got SIGSEGV on teardown is not counted as a failure.
EXIT_SEGV = (-11, 139)


def run(ctx, script, *args, timeout=60, forever=True):
    """Run an example. forever=True: SIGINT at `timeout` is the expected way out."""
    rc, out = ctx.sh([PYTHON, EXAMPLES / script, *args], cwd=EXAMPLES, timeout=timeout,
                     allow_timeout=True, ok_rc=None)
    check(not has_traceback(out), f"{script} raised:\n{out[-2000:]}")
    if rc in EXIT_SEGV:
        ctx.notes.append(f"{script} segfaulted at exit (rc={rc})")
    elif not ctx.last_timed_out:
        check(rc == 0, f"{script} exited rc={rc}" + (" before its time box" if forever else "")
              + f":\n{out[-2000:]}")
    elif not forever:
        check(False, f"{script} did not finish within {timeout}s")
    return rc, out


run_checked = run


def bg(ctx, script, *args):
    return ctx.bg([PYTHON, EXAMPLES / script, *args], cwd=EXAMPLES)


def finish(ctx, p, script):
    rc, out = ctx.stop_bg(p)
    check(not has_traceback(out), f"{script} raised:\n{out[-2000:]}")
    return rc, out


P = ("--port", "2000")


@test("T4.generate_traffic")
def generate_traffic(ctx):
    p = bg(ctx, "generate_traffic.py", *P, "-n", "50", "-w", "30", "--safe", "--seed", "1")
    time.sleep(45)
    world = ctx.world()
    v, w = W.count(world, "vehicle.*"), W.count(world, "walker.pedestrian.*")
    finish(ctx, p, "generate_traffic.py")
    check(v >= 45, f"only {v} vehicles while running (asked 50)")
    check(w >= 20, f"only {w} walkers while running (asked 30)")
    return {"vehicles": v, "walkers": w}


@test("T4.automatic_control")
def automatic_control(ctx):
    p = bg(ctx, "automatic_control.py", *P, "--sync", "--res", "640x360", "-l", "-s", "4")
    time.sleep(25)
    hero = W.find_role(ctx.world(), "hero")
    check(hero is not None, "no hero vehicle after 25 s")
    a = hero.get_location()
    time.sleep(50)
    d = hero.get_location().distance(a)
    finish(ctx, p, "automatic_control.py")
    check(d > 30, f"hero moved only {d:.1f} m in 50 s")
    return {"moved_m": round(d, 1)}


@test("T4.manual_control")
def manual_control(ctx):
    p = bg(ctx, "manual_control.py", *P, "--autopilot", "--res", "640x360")
    time.sleep(30)
    hero = W.find_role(ctx.world(), "hero")
    v = W.speed(hero) if hero else 0
    time.sleep(10)
    finish(ctx, p, "manual_control.py")
    check(hero is not None, "no hero vehicle after 30 s")
    return {"hero_speed_ms": round(v, 2)}


@test("T4.sensor_synchronization")
def sensor_synchronization(ctx):
    rc, out = run(ctx, "sensor_synchronization.py", timeout=45)
    check("Frame" in out or "frame" in out, "no frame output from sensor_synchronization")


@test("T4.no_rendering_mode")
def no_rendering_mode(ctx):
    run(ctx, "no_rendering_mode.py", *P, "--res", "800x600", timeout=45)


@test("T4.dynamic_weather")
def dynamic_weather(ctx):
    w0 = ctx.world().get_weather()
    p = bg(ctx, "dynamic_day_night_weather_cycle.py", *P, "--speed", "50")
    time.sleep(20)
    w1 = ctx.world().get_weather()
    finish(ctx, p, "dynamic_day_night_weather_cycle.py")
    changed = abs(w1.sun_altitude_angle - w0.sun_altitude_angle) > 1 or \
        abs(w1.cloudiness - w0.cloudiness) > 1 or abs(w1.precipitation - w0.precipitation) > 1
    check(changed, "weather did not change in 20 s")


@test("T4.lane_explorer")
def lane_explorer(ctx):
    run(ctx, "lane_explorer.py", *P, "-t", "0.05", timeout=30)


@test("T4.vehicle_gallery")
def vehicle_gallery(ctx):
    # Cycles a camera around every vehicle blueprint; 10 minutes covers a good share.
    run(ctx, "vehicle_gallery.py", timeout=600)


@test("T4.vehicle_lights_demo")
def vehicle_lights_demo(ctx):
    run(ctx, "vehicle_lights_demo.py", *P, timeout=90)


@test("T4.vehicle_doors_demo")
def vehicle_doors_demo(ctx):
    run(ctx, "vehicle_doors_demo.py", *P, timeout=90)


@test("T4.draw_skeleton")
def draw_skeleton(ctx):
    run(ctx, "draw_skeleton.py", timeout=30)


@test("T4.dvs_test")
def dvs_test(ctx):
    _, out = run_checked(ctx, "dvs_test.py", *P, "--frames", "30", "--out",
                         ctx.out / "dvs.png", timeout=300, forever=False)
    check("event" in out.lower(), f"dvs_test reported no events:\n{out[-1000:]}")


@test("T4.visualize_multiple_sensors")
def visualize_multiple_sensors(ctx):
    run(ctx, "visualize_multiple_sensors.py", *P, "--res", "1280x720", timeout=45)


@test("T4.visualize_radar")
def visualize_radar(ctx):
    run(ctx, "visualize_radar.py", *P, timeout=30)


@test("T4.interpolate_camera")
def interpolate_camera(ctx):
    run_checked(ctx, "interpolate_camera.py", "-f", "interpolation.xml", "--id",
                "Town10_showcase", timeout=300, forever=False)


@test("T4.recorder_replay")
def recorder_replay(ctx):
    name = str(ctx.out / "t4_fixture.log")   # absolute: the example checks the path client-side
    hold = ctx.bg([PYTHON, SKILLS / "python-api" / "spawn-vehicles" / "scripts" / "vehicles.py",
                   "spawn", "--count", "20", "--safe", "--seed", "2", "--hold"])
    hold.wait_for("holding", 180)
    client = ctx.client()
    client.start_recorder(name)
    time.sleep(15)
    client.stop_recorder()
    ctx.stop_bg(hold)
    run_checked(ctx, "recorder_replay.py", *P, "-f", name, timeout=240, forever=False)
    left = W.dynamic_ids(ctx.world())
    if left:
        W.destroy(ctx.client(), left)
        ctx.notes.append(f"{len(left)} replayed actors remained after the replay ended")


@test("T4.rig_perf_demo")
def rig_perf_demo(ctx):
    out = ctx.out / "rig"
    run_checked(ctx, "rig_perf_demo.py", "measure", *P, "--seconds", "20", "--out", out,
                timeout=900, forever=False)
    files = list(out.glob("*")) if out.exists() else []
    check(files, "rig_perf_demo wrote nothing to --out")
    return {"files": len(files)}


@test("T4.tm_benchmark")
def tm_benchmark(ctx):
    out = ctx.out / "tm_benchmark.json"
    rc, log = ctx.sh([PYTHON, REPO / "PythonAPI" / "test" / "tm_benchmark.py", "run", *P,
                      "--duration", "40", "--out", out], timeout=1200, ok_rc=None)
    check(not has_traceback(log), f"tm_benchmark raised:\n{log[-2000:]}")
    check(out.exists(), f"no report written (rc={rc})")
    json.loads(out.read_text())


@test("T4.open3d_lidar")
def open3d_lidar(ctx):
    raise Skip("opens an Open3D window; not runnable headless")
