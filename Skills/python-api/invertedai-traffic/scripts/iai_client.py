#!/usr/bin/env python
"""Compact Inverted AI co-simulation client for CARLA (invertedai-traffic skill).

    iai_client.py ping                 prove the key and the map name, touch nothing
    iai_client.py run  [-n N] ...      spawn N Inverted AI-driven vehicles and drive them
    iai_client.py status               count this skill's NPCs and report the clock
    iai_client.py clean                destroy leftover NPCs, give the clock back

Run it with the IAI venv's interpreter ($IAI_PY from env.sh). The key is read
from IAI_API_KEY, which env.sh loads from the git-ignored .env.

The full example (walkers, ego hooks, IAI log and GIF, video) is
PythonAPI/examples/invertedai_traffic.py; run_example.sh wraps it. This client
is the small loop: initialize, spawn, then each 0.1 s step tick CARLA, send
the traffic-light states, call large_drive, teleport the NPCs.
"""

import argparse
import logging
import math
import os
import random
import signal
import sys
import time

import carla

ROLE = "iai_npc"
STEP = 0.1  # the only step length Inverted AI supports


def connect(args):
    """Client and world for --host/--port."""
    client = carla.Client(args.host, args.port)
    client.set_timeout(args.timeout)
    return client, client.get_world()


def location_of(world, override):
    """carla:<Town> for the loaded map. CARLA on UE 5 ships Town10HD_Opt;
    Inverted AI names it after the original town."""
    if override:
        return override
    name = world.get_map().name.split("/")[-1]
    if name.endswith("_Opt"):
        name = name[:-4]
    return "carla:" + name


def quiet_academic_notice(record):
    """Logging filter: drop the SDK's warning that an academic key is used. It
    fires on every authenticated call and is not an error."""
    return "Commercial access denied" not in record.getMessage()


def load_iai():
    """Import the SDK and register the key from IAI_API_KEY, or exit with a FAIL."""
    try:
        import invertedai as iai  # pylint: disable=import-outside-toplevel
    except ImportError as e:
        sys.exit(f"FAIL invertedai not importable in {sys.executable} ({e}); run scripts/install_iai.sh")
    key = os.environ.get("IAI_API_KEY")
    if not key:
        sys.exit("FAIL IAI_API_KEY is not set: put IAI_API_KEY=<key> in the repo-root .env (see SKILL.md)")
    logging.getLogger("invertedai.utils").addFilter(quiet_academic_notice)
    iai.add_apikey(key)
    return iai


# --- traffic lights --------------------------------------------------------
# Inverted AI's CARLA maps number lights differently from the actor ids. The
# shipped example derives the mapping from sorted actor ids, and that is only
# valid on Town10HD (each CARLA light maps to two IAI light ids).

def tl_mapping(world, location):
    """CARLA light actor id -> its two IAI light ids, or None off Town10HD."""
    if location != "carla:Town10HD":
        return None
    ids = sorted(tl.id for tl in world.get_actors().filter("traffic.traffic_light*"))
    return {cid: (str(4364 + i), str(5364 + i)) for i, cid in enumerate(ids)}


def tl_states(world, mapping):
    """IAI traffic-light states mirroring CARLA's lights, or None without a mapping."""
    if mapping is None:
        return None
    from invertedai.common import TrafficLightState  # pylint: disable=import-outside-toplevel,import-error
    conv = {carla.TrafficLightState.Red: TrafficLightState.red,
            carla.TrafficLightState.Yellow: TrafficLightState.yellow,
            carla.TrafficLightState.Green: TrafficLightState.green}
    out = {}
    for tl in world.get_actors().filter("traffic.traffic_light*"):
        for iid in mapping.get(tl.id, ()):
            # the SDK's "off" is `none` (older examples used `.Off`, which does not exist)
            out[iid] = conv.get(tl.state, TrafficLightState.none)
    return out


# --- geometry --------------------------------------------------------------

def to_carla(state, cmap, z_lift=0.1):
    """IAI and CARLA share x/y in metres; IAI yaw is radians. Take z from the
    road under the agent so hilly maps do not bury or float the NPCs."""
    loc = carla.Location(state.center.x, state.center.y, 0.0)
    wp = cmap.get_waypoint(loc, project_to_road=True)
    loc.z = (wp.transform.location.z if wp else 0.0) + z_lift
    return carla.Transform(loc, carla.Rotation(yaw=math.degrees(state.orientation)))


# --- commands --------------------------------------------------------------

def cmd_ping(args):
    """Prove the key and the map name with one location_info call."""
    iai = load_iai()
    _, world = connect(args)
    loc = location_of(world, args.location)
    t0 = time.time()
    r = iai.location_info(location=loc)
    lights = [a for a in (r.static_actors or []) if str(getattr(a, "agent_type", "")).endswith("traffic_light")]
    print(f"PASS  location_info({loc}) in {time.time() - t0:.2f} s")
    print(f"      map version {r.version}, max agents per call {r.max_agent_number}")
    print(f"      map centre {r.map_center}, fov {r.map_fov} m, static traffic lights {len(lights)}")
    if tl_mapping(world, loc) is None:
        print("WARN  traffic-light sync is only implemented for Town10HD; elsewhere NPCs ignore CARLA's lights")


def own_npcs(world):
    """Vehicles this skill spawned (role_name=iai_npc)."""
    return [a for a in world.get_actors().filter("vehicle.*") if a.attributes.get("role_name") == ROLE]


def cmd_status(args):
    """Count this skill's NPCs and report the clock."""
    _, world = connect(args)
    s = world.get_settings()
    npcs = own_npcs(world)
    print(f"map {world.get_map().name.split('/')[-1]}; {len(npcs)} {ROLE} vehicles; "
          f"synchronous={s.synchronous_mode} fixed_delta={s.fixed_delta_seconds}")


def cmd_clean(args):
    """Destroy leftover NPCs and give the clock back."""
    client, world = connect(args)
    npcs = own_npcs(world)
    client.apply_batch_sync([carla.command.DestroyActor(a.id) for a in npcs])
    s = world.get_settings()
    s.synchronous_mode, s.fixed_delta_seconds = False, None
    world.apply_settings(s)
    print(f"destroyed {len(npcs)} {ROLE} vehicles; world asynchronous")


def spawn_npcs(world, resp, actors):
    """Spawn a CARLA car for each car agent of an initialize response. Spawned
    actors are appended to `actors` as they appear, so the caller can destroy
    a partial set. Returns the states, properties and recurrent states of the
    agents that did spawn, in the same order."""
    cmap = world.get_map()
    bps = [b for b in world.get_blueprint_library().filter("vehicle.*")
           if b.has_attribute("base_type") and b.get_attribute("base_type").as_str() == "car"]
    if not bps:
        sys.exit("FAIL no blueprint with base_type=car on this server")
    states, props, recur = [], [], []
    for st, pr, rs in zip(resp.agent_states, resp.agent_properties, resp.recurrent_states):
        if str(getattr(pr, "agent_type", "car")) not in ("car", "AgentType.car"):
            continue  # this client only drives cars
        bp = random.choice(bps)
        bp.set_attribute("role_name", ROLE)
        if bp.has_attribute("color"):
            bp.set_attribute("color", random.choice(bp.get_attribute("color").recommended_values))
        actor = world.try_spawn_actor(bp, to_carla(st, cmap, z_lift=0.3))
        if actor is None:
            continue
        actor.set_simulate_physics(False)
        ext = actor.bounding_box.extent
        pr.length, pr.width, pr.rear_axis_offset = 2 * ext.x, 2 * ext.y, 2 * ext.x / 3
        actors.append(actor)
        states.append(st)
        props.append(pr)
        recur.append(rs)
    return states, props, recur


def drive(iai, client, world, sim, args):
    """The co-simulation loop: tick CARLA, send the lights, large_drive, teleport.
    `sim` holds location, seed, mapping, actors, states, props, recur and stop."""
    cmap = world.get_map()
    actors, states, recur = sim["actors"], sim["states"], sim["recur"]
    steps = None if args.hold else args.steps
    n, api_time, moved = 0, 0.0, set()
    start_xy = {a.id: (s0.center.x, s0.center.y) for a, s0 in zip(actors, states)}
    while not sim["stop"]["flag"] and (steps is None or n < steps):
        world.tick()
        t0 = time.time()
        resp = iai.large_drive(location=sim["location"], agent_states=states, agent_properties=sim["props"],
                               recurrent_states=recur, traffic_lights_states=tl_states(world, sim["mapping"]),
                               single_call_agent_limit=args.capacity,
                               api_model_version=args.api_model, random_seed=sim["seed"])
        api_time += time.time() - t0
        states, recur = resp.agent_states, resp.recurrent_states
        client.apply_batch([carla.command.ApplyTransform(a.id, to_carla(st, cmap))
                            for a, st in zip(actors, states)])
        for a, st in zip(actors, states):
            x0, y0 = start_xy[a.id]
            if math.hypot(st.center.x - x0, st.center.y - y0) > 2.0:
                moved.add(a.id)
        n += 1
        if n % 50 == 0:
            mean_v = sum(st.speed for st in states) / len(states)
            print(f"[iai] step {n}: mean speed {mean_v * 3.6:.1f} km/h, "
                  f"large_drive {1000 * api_time / n:.0f} ms/step")
    mean_v = sum(st.speed for st in states) / len(states)
    print(f"[iai] done: {n} steps ({n * STEP:.0f} s sim), {len(moved)}/{len(actors)} vehicles "
          f"moved > 2 m, final mean speed {mean_v * 3.6:.1f} km/h, "
          f"large_drive {1000 * api_time / max(n, 1):.0f} ms/step")


def cmd_run(args):
    """Initialize N agents, spawn them and drive them; always clean up."""
    iai = load_iai()
    client, world = connect(args)
    sim = {"location": location_of(world, args.location),
           "seed": args.seed if args.seed is not None else int(time.time()),
           "actors": [], "stop": {"flag": False}}
    random.seed(sim["seed"])

    previous = world.get_settings()
    if previous.synchronous_mode:
        print("WARN  world already synchronous: another client may own the clock; "
              "this client ticks it anyway (Inverted AI needs a fixed 0.1 s step)")
    settings = world.get_settings()
    settings.synchronous_mode, settings.fixed_delta_seconds = True, STEP
    world.apply_settings(settings)

    recording = False
    signal.signal(signal.SIGINT, lambda *_: sim["stop"].update(flag=True))
    try:
        sim["mapping"] = tl_mapping(world, sim["location"])
        if sim["mapping"] is None:
            print(f"WARN  no traffic-light mapping for {sim['location']}: NPCs will not see CARLA's lights")
        world.tick()
        lights = tl_states(world, sim["mapping"])

        center = tuple(args.center)
        regions = iai.get_regions_default(location=sim["location"], total_num_agents=args.number,
                                          area_shape=(args.size / 2, args.size / 2),
                                          map_center=center, random_seed=sim["seed"])
        t0 = time.time()
        resp = iai.large_initialize(location=sim["location"], regions=regions,
                                    traffic_light_state_history=[lights] if lights else None,
                                    random_seed=sim["seed"])
        print(f"[iai] large_initialize: {len(resp.agent_states)} agents around {center} "
              f"({args.size} m square) in {time.time() - t0:.1f} s, seed {sim['seed']}")

        sim["states"], sim["props"], sim["recur"] = spawn_npcs(world, resp, sim["actors"])
        if not sim["actors"]:
            sys.exit("FAIL no Inverted AI agent could be spawned (all overlapped?); try another --center or --seed")
        print(f"[iai] spawned {len(sim['actors'])}/{len(resp.agent_states)} as role_name={ROLE}")

        if args.record:
            path = os.path.abspath(args.record)
            client.start_recorder(path)
            recording = True
            print(f"[iai] recording to {path}")

        drive(iai, client, world, sim, args)
    finally:
        if recording:
            client.stop_recorder()
        if sim["actors"] and not args.keep:
            client.apply_batch_sync([carla.command.DestroyActor(a.id) for a in sim["actors"]])
            print(f"[iai] destroyed {len(sim['actors'])} NPCs")
        world.apply_settings(previous)
        print(f"[iai] world settings restored (synchronous={previous.synchronous_mode})")


def main():
    """Parse the command line and dispatch."""
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--host", default=os.environ.get("CARLA_HOST", "127.0.0.1"))
    p.add_argument("--port", type=int, default=int(os.environ.get("CARLA_PORT", "2000")))
    p.add_argument("--timeout", type=float, default=30.0)
    p.add_argument("--location", help="Inverted AI location, e.g. carla:Town10HD (default: from the loaded map)")
    sub = p.add_subparsers(dest="cmd", required=True)
    sub.add_parser("ping")
    sub.add_parser("status")
    sub.add_parser("clean")
    r = sub.add_parser("run")
    r.add_argument("-n", "--number", type=int, default=30, help="vehicles to request")
    r.add_argument("--center", type=float, nargs=2, default=(-50.0, 20.0), metavar=("X", "Y"),
                   help="centre of the populated square, CARLA metres (default: Town10HD centre)")
    r.add_argument("--size", type=float, default=250.0, help="side of the populated square, m")
    r.add_argument("--steps", type=int, default=300, help="0.1 s steps to drive (default 300 = 30 s)")
    r.add_argument("--hold", action="store_true", help="drive until Ctrl+C")
    r.add_argument("--seed", type=int)
    r.add_argument("--record", metavar="FILE", help="CARLA recorder .log path (server-side)")
    r.add_argument("--keep", action="store_true", help="leave the NPCs in the world (frozen) on exit")
    r.add_argument("--capacity", type=int, default=100, help="agents per large_drive sub-call")
    r.add_argument("--api-model", default="nBu1", help="Inverted AI drive model version")
    args = p.parse_args()
    {"ping": cmd_ping, "run": cmd_run, "status": cmd_status, "clean": cmd_clean}[args.cmd](args)


if __name__ == "__main__":
    main()
