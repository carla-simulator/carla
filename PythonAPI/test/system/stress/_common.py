"""Shared bits for the stress scripts: args, client, cleanup, result line."""

import argparse
import json
import sys
import time
from pathlib import Path

import carla


def parser(doc):
    ap = argparse.ArgumentParser(description=doc)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=2000)
    ap.add_argument("--tm-port", type=int, default=8000)
    ap.add_argument("--server-pid", type=int, help="server pid, to sample its memory")
    return ap


def client(args, timeout=60.0):
    c = carla.Client(args.host, args.port)
    c.set_timeout(timeout)
    return c


def rss_mb(pid):
    if not pid:
        return None
    try:
        for line in Path(f"/proc/{pid}/status").read_text().splitlines():
            if line.startswith("VmRSS:"):
                return int(line.split()[1]) // 1024
    except OSError:
        pass
    return None


def safe_vehicle_bps(world):
    bps = [b for b in world.get_blueprint_library().filter("vehicle.*")
           if b.has_attribute("base_type")
           and b.get_attribute("base_type").as_str().lower() == "car"]
    if not bps:
        bps = [b for b in world.get_blueprint_library().filter("vehicle.*")
               if int(b.get_attribute("number_of_wheels")) == 4]
    return sorted(bps, key=lambda b: b.id)


def set_sync(world, on, delta=0.05):
    s = world.get_settings()
    s.synchronous_mode = on
    s.fixed_delta_seconds = delta if on else None
    world.apply_settings(s)


def destroy_all(c, patterns=("sensor.*", "controller.*", "walker.*", "vehicle.*")):
    world = c.get_world()
    for a in world.get_actors().filter("controller.ai.walker"):
        try:
            a.stop()
        except RuntimeError:
            pass
    ids = []
    for p in patterns:
        ids += [a.id for a in world.get_actors().filter(p)]
    if ids:
        c.apply_batch_sync([carla.command.DestroyActor(i) for i in ids], False)
    return len(ids)


def percentile(xs, q):
    if not xs:
        return None
    xs = sorted(xs)
    return xs[min(len(xs) - 1, int(q * len(xs)))]


def result(**metrics):
    """The harness reads the last line starting with RESULT."""
    print("RESULT " + json.dumps(metrics), flush=True)
    sys.exit(0)


def now():
    return time.perf_counter()
