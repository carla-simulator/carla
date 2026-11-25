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
# Inverted AI numbers its lights independently of CARLA's actor ids, and the
# actor ids' order is not stable across builds (a table built from sorted ids
# was off by two on UE 5.8 Town10HD, so every IAI car saw another junction's
# light). Lights are matched along the lane instead: an IAI light belongs to the
# nearest CARLA stop waypoint on its lane (or the line between two lanes),
# facing the same way, just before it. Measured on Town01-07 and Town10HD, IAI
# lights sit 0-14 m past CARLA's stop waypoints, within 1.8 m of the lane centre.
#
# One side owns the lights and the other follows it every step:
#   carla  CARLA's light cycle runs; its states are sent to Inverted AI.
#   iai    Inverted AI's server runs the lights; CARLA's lights are frozen and
#          set to the states the IAI agents acted on.
# A light only one side has is shown as off on the other side where possible
# (an IAI light with no CARLA light is sent as `none`; a CARLA light with no IAI
# light is switched off in iai mode). Every step is checked.

TL_LATERAL_M = 2.5           # an IAI light must be this close to the stop waypoint's lane centre
TL_ALONG_M = (-2.0, 20.0)    # and this far behind/past the stop waypoint, along the lane
TL_YAW_DEG = 45.0            # and face the same way (curved approaches differ by ~30 degrees)


def lane_distance(light, stop):
    """Distance from a stop waypoint's transform to an IAI light on the same
    lane and heading just past it, or None if the light is not there."""
    fwd = stop.get_forward_vector()
    dx, dy = light.center.x - stop.location.x, light.center.y - stop.location.y
    along, lateral = dx * fwd.x + dy * fwd.y, -dx * fwd.y + dy * fwd.x
    dyaw = (math.degrees(light.orientation) - stop.rotation.yaw + 180.0) % 360.0 - 180.0
    if abs(lateral) <= TL_LATERAL_M and TL_ALONG_M[0] <= along <= TL_ALONG_M[1] and abs(dyaw) <= TL_YAW_DEG:
        return math.hypot(dx, dy)
    return None


def match_lights(world, iai_lights):
    """CARLA light id -> the IAI light ids on its lanes, plus the IAI lights and
    the CARLA lights with no partner."""
    stops = [(tl.id, wp.transform) for tl in world.get_actors().filter("traffic.traffic_light*")
             for wp in tl.get_stop_waypoints()]
    mapping, lone_iai = {}, []
    for light in iai_lights:
        found = [(d, cid) for cid, tf in stops for d in [lane_distance(light, tf)] if d is not None]
        if found:
            mapping.setdefault(min(found)[1], set()).add(str(light.actor_id))
        else:
            lone_iai.append(str(light.actor_id))
    # an IAI light id can come once per signal head, so ids are kept once
    mapping = {cid: sorted(ids) for cid, ids in mapping.items()}
    paired = {i for ids in mapping.values() for i in ids}
    lone_iai = sorted(set(lone_iai) - paired)
    lone_carla = sorted({cid for cid, _ in stops} - set(mapping))
    return mapping, lone_iai, lone_carla


RESTRICTIVE = [carla.TrafficLightState.Red, carla.TrafficLightState.Yellow,
               carla.TrafficLightState.Green, carla.TrafficLightState.Off]


def group_state(values):
    """One CARLA state for the IAI lights of one CARLA light. They differ only
    where IAI has per-lane lights (e.g. a free turn lane on Town06), which a
    single CARLA light cannot show: take the majority, ties to the most
    restrictive."""
    return max(set(values), key=lambda v: (values.count(v), -RESTRICTIVE.index(v)))


class LightSync:  # pylint: disable=too-many-instance-attributes
    """Keeps CARLA's and Inverted AI's traffic lights identical (see above)."""

    def __init__(self, world, location, iai, mode):
        from invertedai.common import TrafficLightState as S  # pylint: disable=import-outside-toplevel,import-error
        self.world, self.mode = world, mode
        self.to_iai = {carla.TrafficLightState.Red: S.red, carla.TrafficLightState.Yellow: S.yellow,
                       carla.TrafficLightState.Green: S.green}
        self.to_carla = {S.red: carla.TrafficLightState.Red, S.yellow: carla.TrafficLightState.Yellow,
                         S.green: carla.TrafficLightState.Green}
        self.none_iai = S.none
        info = iai.location_info(location=location)
        lights = [a for a in (info.static_actors or []) if str(getattr(a, "agent_type", "")).endswith("traffic_light")]
        self.tls = {tl.id: tl for tl in world.get_actors().filter("traffic.traffic_light*")}
        self.counts = {"checks": 0, "mismatches": 0, "split": 0}
        self.frozen, self.applied, self.lone_iai, self.lone_carla = False, {}, [], []
        if not lights:
            self.mapping = None
            if mode == "iai":
                sys.exit(f"FAIL {location} has no Inverted AI traffic lights; use --traffic-lights carla")
            print(f"WARN  {location} has no Inverted AI traffic lights: NPCs will not see CARLA's lights")
            return
        self.mapping, self.lone_iai, self.lone_carla = match_lights(world, lights)
        if self.lone_iai:
            print(f"WARN  Inverted AI lights with no CARLA light: {self.lone_iai}; "
                  + ("sent as off" if mode == "carla" else "CARLA cannot show them"))
        if self.lone_carla:
            print(f"WARN  CARLA lights with no Inverted AI light: {self.lone_carla}; "
                  + ("Inverted AI agents ignore them" if mode == "carla" else "switched off"))

    @property
    def mismatches(self):
        """Steps where the two sides did not show the same states."""
        return self.counts["mismatches"]

    def summary(self):
        """One line: the mode, the pairs, the unpaired lights and the check results."""
        if self.mapping is None:
            return "traffic lights: not synced (no Inverted AI lights)"
        out = (f"traffic lights ({self.mode}): {len(self.mapping)} CARLA <-> "
               f"{sum(len(v) for v in self.mapping.values())} Inverted AI")
        if self.lone_iai or self.lone_carla:
            out += f" ({len(self.lone_iai)} IAI and {len(self.lone_carla)} CARLA unpaired)"
        out += f", {self.counts['mismatches']} mismatches in {self.counts['checks']} checks"
        if self.counts["split"]:
            out += (f", {self.counts['split']} steps where IAI per-lane lights of one CARLA light differed"
                    " (CARLA showed the majority)")
        return out

    def from_carla(self):
        """carla mode: IAI states for every IAI light, from one read of CARLA's lights."""
        if self.mapping is None:
            return None
        out = {iid: self.to_iai.get(self.tls[cid].state, self.none_iai)
               for cid, ids in self.mapping.items() for iid in ids}
        out.update({iid: self.none_iai for iid in self.lone_iai})  # no CARLA light there: off
        return out

    def check_echo(self, sent, seen):
        """carla mode: the states the server says the agents saw must be the ones sent."""
        if self.mapping is None:
            return
        self.counts["checks"] += 1
        seen = {str(k): v for k, v in (seen or {}).items()}  # the SDK returns int ids
        self.counts["mismatches"] += any(seen.get(i) != v for i, v in sent.items())

    def freeze(self):
        """iai mode: stop CARLA's light cycle so only Inverted AI changes the lights."""
        for tl in self.tls.values():
            tl.freeze(True)
        self.frozen = True

    def apply_iai(self, states):
        """iai mode: set every CARLA light to its IAI lights' state, and the CARLA
        lights IAI does not have to off. Shows from the next tick, where
        check_carla verifies it."""
        states = {str(k): v for k, v in (states or {}).items()}  # the SDK returns int ids
        self.applied = {cid: carla.TrafficLightState.Off for cid in self.lone_carla}
        split = False
        for cid, ids in self.mapping.items():
            vals = [self.to_carla.get(states.get(i), carla.TrafficLightState.Off) for i in ids]
            if any(i not in states for i in ids):
                self.counts["mismatches"] += 1  # the server left out a light it has
            split |= len(set(vals)) != 1
            self.applied[cid] = group_state(vals)
        self.counts["split"] += split
        for cid, value in self.applied.items():
            self.tls[cid].set_state(value)

    def check_carla(self):
        """iai mode, after a tick: CARLA's lights must show what was applied."""
        self.counts["checks"] += 1
        self.counts["mismatches"] += any(self.tls[c].state != v for c, v in self.applied.items())

    def release(self):
        """Give the lights back to CARLA's own cycle."""
        if self.frozen:
            for tl in self.tls.values():
                tl.freeze(False)
            self.world.reset_all_traffic_lights()
            self.frozen = False


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
    if not lights:
        print("WARN  this Inverted AI map has no traffic lights; NPCs ignore CARLA's lights")
        return
    mapping, lone_iai, lone_carla = match_lights(world, lights)
    print(f"      traffic lights matched along the lanes: {len(mapping)} CARLA lights <-> "
          f"{sum(len(v) for v in mapping.values())} Inverted AI lights")
    if lone_iai or lone_carla:
        print(f"WARN  unpaired: Inverted AI {lone_iai}, CARLA {lone_carla}")


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
    """The co-simulation loop. Each step: tick CARLA, then one large_drive whose
    light states are the ones CARLA shows (carla mode: read after the tick and
    sent; iai mode: applied before the tick and stepped by the server), then
    teleport the NPCs and, in iai mode, set CARLA's lights to the new states.
    `sim` holds location, seed, lights, actors, states, props, recur, light_recur and stop."""
    cmap = world.get_map()
    lights = sim["lights"]
    actors, states, recur = sim["actors"], sim["states"], sim["recur"]
    steps = None if args.hold else args.steps
    n, api_time, moved = 0, 0.0, set()
    start_xy = {a.id: (s0.center.x, s0.center.y) for a, s0 in zip(actors, states)}
    while not sim["stop"]["flag"] and (steps is None or n < steps):
        world.tick()
        if lights.mode == "iai":
            lights.check_carla()
            sent = None
        else:
            sent = lights.from_carla()
        t0 = time.time()
        resp = iai.large_drive(location=sim["location"], agent_states=states, agent_properties=sim["props"],
                               recurrent_states=recur, traffic_lights_states=sent,
                               light_recurrent_states=sim["light_recur"],
                               single_call_agent_limit=args.capacity,
                               api_model_version=args.api_model, random_seed=sim["seed"])
        api_time += time.time() - t0
        if lights.mode == "iai":
            # The server steps its lights at the start of the call and the agents
            # react to them, so they are only known now. Show them with the
            # positions the agents reacted from (the call's input); the new
            # positions follow on the next step. Each frame then pairs positions
            # with the lights acted on, as in carla mode.
            shown = states
            sim["light_recur"] = resp.light_recurrent_states
            lights.apply_iai(resp.traffic_lights_states)
        else:
            lights.check_echo(sent, resp.traffic_lights_states)
        states, recur = resp.agent_states, resp.recurrent_states
        if lights.mode != "iai":
            shown = states
        client.apply_batch([carla.command.ApplyTransform(a.id, to_carla(st, cmap))
                            for a, st in zip(actors, shown)])
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
    print(f"[iai] {lights.summary()}")


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
    lights = None
    signal.signal(signal.SIGINT, lambda *_: sim["stop"].update(flag=True))
    try:
        lights = sim["lights"] = LightSync(world, sim["location"], iai, args.traffic_lights)
        world.tick()
        if lights.mode == "iai":
            lights.freeze()  # from here on only Inverted AI changes CARLA's lights
            history = None   # the server picks the initial light states
        else:
            sent = lights.from_carla()
            history = [sent] if sent else None

        center = tuple(args.center)
        regions = iai.get_regions_default(location=sim["location"], total_num_agents=args.number,
                                          area_shape=(args.size / 2, args.size / 2),
                                          map_center=center, random_seed=sim["seed"])
        t0 = time.time()
        resp = iai.large_initialize(location=sim["location"], regions=regions,
                                    traffic_light_state_history=history, random_seed=sim["seed"])
        print(f"[iai] large_initialize: {len(resp.agent_states)} agents around {center} "
              f"({args.size} m square) in {time.time() - t0:.1f} s, seed {sim['seed']}")

        sim["states"], sim["props"], sim["recur"] = spawn_npcs(world, resp, sim["actors"])
        if not sim["actors"]:
            sys.exit("FAIL no Inverted AI agent could be spawned (all overlapped?); try another --center or --seed")
        print(f"[iai] spawned {len(sim['actors'])}/{len(resp.agent_states)} as role_name={ROLE}")
        sim["light_recur"] = None
        if lights.mode == "iai":
            # the agents were placed for these lights; CARLA shows them from the first frame
            sim["light_recur"] = resp.light_recurrent_states
            lights.apply_iai(resp.traffic_lights_states)

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
        if lights is not None:
            lights.release()
        world.apply_settings(previous)
        print(f"[iai] world settings restored (synchronous={previous.synchronous_mode})")
    if lights.mismatches:
        sys.exit(f"FAIL {lights.summary()}")


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
    r.add_argument("--traffic-lights", choices=("carla", "iai"), default="carla",
                   help="who runs the lights: CARLA's cycle, sent to Inverted AI (default), or Inverted AI's "
                        "server, applied to CARLA's frozen lights")
    args = p.parse_args()
    {"ping": cmd_ping, "run": cmd_run, "status": cmd_status, "clean": cmd_clean}[args.cmd](args)


if __name__ == "__main__":
    main()
