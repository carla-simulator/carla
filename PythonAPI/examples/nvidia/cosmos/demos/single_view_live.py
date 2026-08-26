#!/usr/bin/env python3
"""Capture a single-camera Cosmos clip from a TM-driven ego vehicle.

Spawns a hero plus background traffic, captures ``--frames`` frames with the
selected rig (RGB, depth, segmentation controls and the ClipGT scene package)
and writes a Clip to disk.  With ``--capture-only`` it stops there; job
submission arrives with the Phase 2/3 server work.

Example (CARLA server on :2100)::

    python single_view_live.py --port 2100 --frames 60 --vehicles 10 \
        --out ./clips --capture-only
"""

import argparse
import logging
import random
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "client"))

import carla  # noqa: E402

from carla_cosmos import BUILTIN_CONTRACTS, Capture, Rig  # noqa: E402

log = logging.getLogger("single_view_live")


def parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=2000)
    ap.add_argument("--tm-port", type=int, default=8000)
    ap.add_argument("--town", default=None, help="load this town first (default: keep the current one)")
    ap.add_argument("--frames", type=int, default=60)
    ap.add_argument("--fps", type=int, default=30)
    ap.add_argument("--vehicles", type=int, default=10)
    ap.add_argument("--walkers", type=int, default=0)
    ap.add_argument("--rig", default="single", help="single | nvidia_av7 | path to a rig YAML")
    ap.add_argument("--edge", action="store_true", help="also write the masked-Canny edge control")
    ap.add_argument("--backend", default="cosmos3-nano", choices=sorted(BUILTIN_CONTRACTS))
    ap.add_argument("--out", default="./clips")
    ap.add_argument("--clip-id", default=None)
    ap.add_argument("--seed", type=int, default=7)
    ap.add_argument("--capture-only", action="store_true", help="capture the clip and exit (no submission)")
    ap.add_argument("--endpoint", default=None, help="Cosmos server URL (Phase 2+)")
    ap.add_argument("--token", default=None)
    ap.add_argument("-v", "--verbose", action="store_true")
    return ap.parse_args()


def load_rig(name: str) -> Rig:
    if name == "single":
        return Rig.single()
    if name == "nvidia_av7":
        return Rig.nvidia_av7()
    return Rig.load(name)


def spawn_traffic(world: carla.World, tm: carla.TrafficManager, n_vehicles: int,
                  n_walkers: int, seed: int) -> tuple[carla.Vehicle, list[carla.Actor]]:
    """Spawn the hero and background traffic; returns (hero, all actors)."""
    rng = random.Random(seed)
    bps = world.get_blueprint_library()
    spawns = world.get_map().get_spawn_points()
    rng.shuffle(spawns)
    actors: list[carla.Actor] = []

    hero_bps = list(bps.filter("vehicle.lincoln.mkz*"))
    if not hero_bps:  # blueprint set differs per content branch (Phase 0 finding)
        hero_bps = [b for b in bps.filter("vehicle.*")
                    if b.has_attribute("base_type") and b.get_attribute("base_type").as_str() == "car"]
    hero_bp = hero_bps[0]
    hero_bp.set_attribute("role_name", "hero")
    hero = world.spawn_actor(hero_bp, spawns[0])
    actors.append(hero)
    hero.set_autopilot(True, tm.get_port())

    car_bps = []
    for b in bps.filter("vehicle.*"):
        try:  # some blueprints raise on attribute access (Phase 0 finding)
            if b.has_attribute("number_of_wheels") and int(b.get_attribute("number_of_wheels")) >= 2:
                car_bps.append(b)
        except RuntimeError:
            continue
    for sp in spawns[1:1 + n_vehicles]:
        v = world.try_spawn_actor(rng.choice(car_bps), sp)
        if v is not None:
            actors.append(v)
            v.set_autopilot(True, tm.get_port())

    walker_bps = list(bps.filter("walker.pedestrian.*"))
    controllers = []
    ctrl_bp = bps.find("controller.ai.walker")
    for _ in range(n_walkers):
        loc = world.get_random_location_from_navigation()
        if loc is None:
            continue
        loc.z += 1.0  # navmesh points need a lift to spawn (Phase 0 finding)
        w = world.try_spawn_actor(rng.choice(walker_bps), carla.Transform(loc))
        if w is not None:
            actors.append(w)
    world.tick()
    for w in [a for a in actors if a.type_id.startswith("walker.")]:
        c = world.spawn_actor(ctrl_bp, carla.Transform(), attach_to=w)
        controllers.append(c)
    world.tick()
    for c in controllers:
        c.start()
        c.go_to_location(world.get_random_location_from_navigation())
        c.set_max_speed(1.4)
    actors += controllers
    return hero, actors


def main() -> int:
    args = parse_args()
    logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO,
                        format="%(asctime)s %(levelname)-7s %(name)s: %(message)s")
    random.seed(args.seed)

    client = carla.Client(args.host, args.port)
    client.set_timeout(120.0)
    world = client.get_world()
    if args.town and args.town.split("/")[-1] not in world.get_map().name:
        log.info("loading %s ...", args.town)
        world = client.load_world(args.town)
        time.sleep(5.0)
    log.info("map: %s", world.get_map().name)

    tm = client.get_trafficmanager(args.tm_port)
    tm.set_random_device_seed(args.seed)
    contract = BUILTIN_CONTRACTS[args.backend]
    rig = load_rig(args.rig)
    clip_id = args.clip_id or f"carla_{world.get_map().name.split('/')[-1].lower()}_{int(time.time())}"

    settings = world.get_settings()
    actors: list[carla.Actor] = []
    try:
        # spawn under sync mode so the settle phase is deterministic
        s = world.get_settings()
        s.synchronous_mode = True
        s.fixed_delta_seconds = 1.0 / args.fps
        world.apply_settings(s)
        tm.set_synchronous_mode(True)

        hero, actors = spawn_traffic(world, tm, args.vehicles, args.walkers, args.seed)
        log.info("spawned hero %d (%s) + %d actors", hero.id, hero.type_id, len(actors) - 1)
        for _ in range(2 * args.fps):  # let traffic get moving
            world.tick()

        cap = Capture(world, hero, rig, contract, frames=args.frames, fps=args.fps, edge=args.edge)
        t0 = time.time()
        clip = cap.run(args.out, clip_id, seed=args.seed,
                       carla_version=client.get_server_version(),
                       progress=lambda i, n: log.info("frame %d/%d", i, n) if i % args.fps == 0 or i == n else None)
        log.info("captured %s in %.1fs -> %s", clip_id, time.time() - t0, clip.path)

        problems = clip.validate()
        if problems:
            for p in problems:
                log.error("clip validation: %s", p)
            return 1
        log.info("clip validated: %d videos, scene package at %s",
                 len(clip.manifest.videos), clip.scene_dir)

        if args.capture_only:
            return 0
        raise NotImplementedError(
            "Job submission needs the carla-cosmos server (Phase 2/3). "
            "The clip on disk is complete; once the server ships, run: "
            f"single_view_live.py --endpoint <url> --token <token> --backend {args.backend}")
    finally:
        tm.set_synchronous_mode(False)
        world.apply_settings(settings)
        for a in actors:
            try:
                if a.type_id.startswith("controller."):
                    a.stop()
            except RuntimeError:
                pass
        client.apply_batch([carla.command.DestroyActor(a) for a in actors])


if __name__ == "__main__":
    sys.exit(main())
