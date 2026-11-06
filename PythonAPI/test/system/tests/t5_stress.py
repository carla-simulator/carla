"""T5 — stress. Each test gets a fresh server; the script reports metrics, the verdict is here."""

import json

from harness.core import test, check, Fail, SYSTEM_DIR, PYTHON, has_traceback


def stress(ctx, script, *args, timeout):
    cmd = [PYTHON, SYSTEM_DIR / "stress" / script, "--port", ctx.port,
           "--server-pid", ctx.server.pid, *args]
    rc, out = ctx.sh(cmd, cwd=SYSTEM_DIR / "stress", timeout=timeout, ok_rc=None)
    lines = [l for l in out.splitlines() if l.startswith("RESULT ")]
    if not lines:
        raise Fail(f"{script} produced no RESULT (rc={rc}):\n{out[-2000:]}")
    check(not has_traceback(out), f"{script} raised:\n{out[-2000:]}")
    return json.loads(lines[-1][len("RESULT "):])


@test("T5.churn", restart=True)
def churn(ctx):
    m = stress(ctx, "churn.py", "--count", "300", "--cycles", "30", timeout=3600)
    check(m["max_leaked"] == 0, f"vehicles left after destroy: {m['max_leaked']}")
    check(m["rss_growth"] is not None and m["rss_growth"] <= 1.05,
          f"server memory grew {m['rss_growth']:.3f}x from cycle 2 to 30")
    return m


@test("T5.max-traffic", restart=True)
def max_traffic(ctx):
    m = stress(ctx, "max_traffic.py", "--vehicles", "250", "--walkers", "200", "--minutes", "10",
               timeout=1800)
    check(m["tick_p99_s"] < 1.0, f"p99 tick {m['tick_p99_s']:.2f}s >= 1 s")
    return m


@test("T5.sensor-flood", restart=True)
def sensor_flood(ctx):
    m = stress(ctx, "sensor_flood.py", "--cameras", "16", "--lidars", "4", "--ticks", "300",
               timeout=1800)
    check(m["missing"] == 0, f"{m['missing']} sensor frames never arrived")
    return m


@test("T5.multi-client", restart=True)
def multi_client(ctx):
    m = stress(ctx, "multi_client.py", "--clients", "10", "--minutes", "5", timeout=1200)
    check(m["clients"] == 10, f"only {m['clients']} clients reported back")
    check(m["errors"] == 0, f"{m['errors']} RPC errors: {m['error_samples'][:3]}")
    return m


@test("T5.map-reload", restart=True)
def map_reload(ctx):
    m = stress(ctx, "map_reload.py", "--loads", "20", timeout=3600)
    check(m["wrong_map"] == 0, f"{m['wrong_map']} loads ended on the wrong map")
    check(m["max_load_s"] < 120, f"slowest load took {m['max_load_s']} s")
    check(m["rss_growth"] is not None and m["rss_growth"] <= 1.10,
          f"server memory grew {m['rss_growth']:.3f}x from load 2 to 20")
    return m


@test("T5.determinism", restart=True)
def determinism(ctx):
    m = stress(ctx, "determinism.py", "--vehicles", "30", "--ticks", "600", timeout=1800)
    check(m["mean_travel_m"] > 20, f"vehicles barely moved ({m['mean_travel_m']:.1f} m); "
                                   "comparison would be vacuous")
    check(m["max_diff_m"] < 0.01, f"runs diverge: max {m['max_diff_m']:.3f} m "
                                  f"(first at tick {m['first_divergent_tick']})")
    return m


@test("T5.client-kill", restart=True)
def client_kill(ctx):
    m = stress(ctx, "client_kill.py", timeout=600)
    check(m.get("recovered"), f"no recovery: {m}")
    check(m["recovered_s"] < 30, f"recovery took {m['recovered_s']:.1f} s")
    check(m["tm_takeover_drives"], f"a new Traffic Manager on the dead one's port does not "
                                   f"drive: {m.get('tm_error') or 'vehicle did not move'}")
    return m


@test("T5.soak", restart=True)
def soak(ctx):
    minutes = ctx.opts.soak_minutes
    m = stress(ctx, "soak.py", "--minutes", minutes, "--vehicles", "100", "--rec", ctx.out / "soak.log",
               timeout=int(minutes * 60 * 1.5) + 900)
    check(m["recorder_frames"] > 0, "recorder file unreadable after the soak")
    if m["rss_slope_mb_per_10min"] is not None:
        check(m["rss_slope_mb_per_10min"] < 50,
              f"server memory rising {m['rss_slope_mb_per_10min']:.0f} MB per 10 min")
    return m
