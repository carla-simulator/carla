#!/usr/bin/env python3
"""Explore a CARLA map's road network: topology, junctions, waypoints, navigation.

Commands:

    summary                              a rundown of the whole map (stats + prose)
    topology  [--draw] [--life 120]      list / draw the road topology (segments)
    junctions [--arms N] [--draw]        enumerate junctions (id, centre, arms, place)
    waypoint  --at X,Y,Z                 nearest driving waypoint + lane facts
    navigate  --at X,Y,Z [--dist 2 --steps 20] [--draw]   walk next() from a point
    road      --id R                     one road: length, neighbours, lane sections and their lanes
    lane      --at X,Y,Z | --road R --lane L [--section S] [--draw]   one lane and its connections
    junction  --id J [--draw]            one junction: adjacent roads, entry and exit lanes

Design note for natural language: this skill turns the map into STRUCTURED data
(junction list with centre / arm-count / distance-to-map-centre / bearing, lane
stats, extent). The agent then resolves a phrase like "the 4-way junction in the
middle" by filtering that list (arms == 4, smallest distance-to-centre) — same
pattern as the weather skill. `--draw` overlays the result via world.debug so you
can confirm the modelled map matches the visible one.

Connection comes from the environment (see env.sh): CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import math
import os
from collections import defaultdict

import carla  # provided by the active interpreter; check_env.sh verifies this

# Sampling step (m) for map-wide stats. Coarse enough to stay fast on large maps
# (Town12), fine enough for road/lane counts and extent.
STAT_STEP = 3.0


def _world():
    client = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                          int(os.environ.get("CARLA_PORT", "2000")))
    client.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return client.get_world()


def _bearing(dx: float, dy: float) -> str:
    """8-way label in the map's XY frame (not true compass — see reference)."""
    if abs(dx) < 1e-6 and abs(dy) < 1e-6:
        return "centre"
    ang = math.degrees(math.atan2(dy, dx)) % 360
    return ["E", "NE", "N", "NW", "W", "SW", "S", "SE"][int((ang + 22.5) // 45) % 8]


def _arms(junction: carla.Junction) -> int:
    """Number of streets meeting at a junction, by approach direction.

    NOT the count of distinct entry road ids. `get_waypoints()` returns one
    (entry, exit) pair per connecting lane *inside* the junction, and those
    entry waypoints belong to the junction's internal roads -- counting their
    road ids counts internal connections, which reports a T-junction as
    four-armed (measured on Town10HD_Opt junction 841: 4 internal roads,
    3 actual arms).

    So each entry is walked backwards until it leaves the junction, and the
    resulting approach headings are clustered: two lanes of the same street
    share a heading, opposite sides of one street differ by 180 degrees and are
    two arms.
    """
    headings = []
    for entry, _ in junction.get_waypoints(carla.LaneType.Driving):
        outside = _step_out(entry)
        if outside is None:
            continue
        yaw = outside.transform.rotation.yaw % 360.0
        if not any(min(abs(yaw - h), 360.0 - abs(yaw - h)) < 30.0
                   for h in headings):
            headings.append(yaw)
    return len(headings)


def _step_out(entry: carla.Waypoint, back: float = 4.0, limit: int = 12):
    """Walk previous() from a junction entry until outside the junction."""
    current = entry
    for _ in range(limit):
        previous = current.previous(back)
        if not previous:
            return None
        current = previous[0]
        if not current.is_junction:
            return current
    return None


def _junctions(world):
    """Dedup junctions across the topology; return list of dicts with geometry."""
    m = world.get_map()
    seen = {}
    for wp, _ in m.get_topology():
        if wp.is_junction:
            j = wp.get_junction()
            seen[j.id] = j
    out = []
    for jid, j in seen.items():
        c = j.bounding_box.location
        e = j.bounding_box.extent
        out.append({"id": jid, "x": c.x, "y": c.y, "z": c.z,
                    "size_x": e.x * 2, "size_y": e.y * 2, "arms": _arms(j), "junction": j})
    return out


def _extent(world):
    xs, ys = [], []
    for wp in world.get_map().generate_waypoints(STAT_STEP):
        loc = wp.transform.location
        xs.append(loc.x); ys.append(loc.y)
    return xs, ys


def cmd_summary(args):
    world = _world()
    m = world.get_map()
    wps = m.generate_waypoints(STAT_STEP)
    roads = defaultdict(set)          # road_id -> set(lane_id)
    for wp in wps:
        roads[wp.road_id].add(wp.lane_id)
    n_roads = len(roads)
    n_lanes = sum(len(v) for v in roads.values())
    lane_counts = [len(v) for v in roads.values()]
    modal = max(set(lane_counts), key=lane_counts.count) if lane_counts else 0
    length_km = len(wps) * STAT_STEP / 1000.0   # ~ one wp per lane per STAT_STEP m

    js = _junctions(world)
    arm_hist = defaultdict(int)
    for j in js:
        arm_hist[j["arms"]] += 1
    n4 = arm_hist.get(4, 0); n3 = arm_hist.get(3, 0)

    xs = [wp.transform.location.x for wp in wps]
    ys = [wp.transform.location.y for wp in wps]
    w = (max(xs) - min(xs)) if xs else 0
    h = (max(ys) - min(ys)) if ys else 0
    area_km2 = (w / 1000.0) * (h / 1000.0) or 1e-9
    jdens = len(js) / area_km2
    density = "dense, city-like" if jdens > 25 else "moderate" if jdens > 8 else "sparse / rural"

    crosswalks = len(m.get_crosswalks())

    print(f"MAP: {m.name}")
    print(f"  extent        : {w:.0f} x {h:.0f} m  (~{area_km2:.2f} km^2)")
    print(f"  roads / lanes : {n_roads} roads, {n_lanes} lanes; most roads have {modal} lane(s)")
    print(f"  lane length   : ~{length_km:.1f} km of driving lane (approx)")
    print(f"  junctions     : {len(js)} total — {n4} four-way, {n3} three-way, "
          f"{len(js)-n4-n3} other")
    print(f"  junction dens.: {jdens:.1f} per km^2 -> {density}")
    print(f"  crosswalks    : {crosswalks}")
    print()
    # A one-paragraph rundown the agent can relay verbatim.
    print("RUNDOWN:")
    print(f"  {m.name} is a {density} map spanning roughly {w:.0f}x{h:.0f} m "
          f"(~{area_km2:.2f} km^2), with {n_roads} roads carrying {n_lanes} lanes — "
          f"most roads are {modal}-lane. It has {len(js)} junctions "
          f"({n4} four-way, {n3} three-way), about {length_km:.1f} km of drivable "
          f"lane, and {crosswalks} crosswalks.")


def cmd_topology(args):
    world = _world()
    topo = world.get_map().get_topology()
    print(f"topology: {len(topo)} directed segments (connected waypoint pairs)")
    if args.draw:
        dbg = world.debug
        for a, b in topo:
            la, lb = a.transform.location, b.transform.location
            dbg.draw_arrow(carla.Location(la.x, la.y, la.z + 0.3),
                           carla.Location(lb.x, lb.y, lb.z + 0.3),
                           0.1, 0.2, carla.Color(0, 255, 0), args.life)
        print(f"drew {len(topo)} topology arrows (life {args.life}s) — compare with the rendered roads")


def cmd_junctions(args):
    world = _world()
    js = _junctions(world)
    if not js:
        print("no junctions on this map"); return
    xs, ys = _extent(world)
    cx, cy = (min(xs) + max(xs)) / 2, (min(ys) + max(ys)) / 2
    for j in js:
        j["dist"] = math.hypot(j["x"] - cx, j["y"] - cy)
        j["place"] = _bearing(j["x"] - cx, j["y"] - cy)
    shown = [j for j in js if args.arms is None or j["arms"] == args.arms]
    shown.sort(key=lambda j: j["dist"])
    print(f"map centre ~({cx:.0f},{cy:.0f}); {len(shown)} junction(s)"
          + (f" with {args.arms} arms" if args.arms is not None else "") + ", nearest-centre first:")
    for j in shown:
        print(f"  id={j['id']:4d}  arms={j['arms']}  centre=({j['x']:.0f},{j['y']:.0f})  "
              f"size={j['size_x']:.0f}x{j['size_y']:.0f}m  {j['dist']:.0f}m {j['place']} of centre")
    if args.draw:
        dbg = world.debug
        for j in shown:
            box = carla.BoundingBox(carla.Location(j["x"], j["y"], j["z"] + 0.3),
                                    carla.Vector3D(j["size_x"] / 2, j["size_y"] / 2, 2.0))
            dbg.draw_box(box, carla.Rotation(), 0.2, carla.Color(255, 128, 0), args.life)
            dbg.draw_string(carla.Location(j["x"], j["y"], j["z"] + 3),
                            f"J{j['id']}", False, carla.Color(255, 255, 0), args.life)
        print(f"drew {len(shown)} junction box(es) + labels (life {args.life}s)")


def cmd_waypoint(args):
    world = _world()
    x, y, z = (float(v) for v in args.at.split(","))
    wp = world.get_map().get_waypoint(carla.Location(x, y, z),
                                      project_to_road=True, lane_type=carla.LaneType.Driving)
    if wp is None:
        print("no driving lane near that point"); return
    print(f"nearest driving waypoint to ({x},{y},{z}):")
    print(f"  road_id={wp.road_id} section_id={wp.section_id} lane_id={wp.lane_id} s={wp.s:.1f}")
    print(f"  is_junction={wp.is_junction}" + (f" (junction {wp.junction_id})" if wp.is_junction else ""))
    print(f"  lane_type={wp.lane_type} lane_width={wp.lane_width:.2f}m lane_change={wp.lane_change}")
    print(f"  location=({wp.transform.location.x:.1f},{wp.transform.location.y:.1f},"
          f"{wp.transform.location.z:.1f}) yaw={wp.transform.rotation.yaw:.0f}")
    # get_left/right_waypoint() replace the deprecated get_left/right_lane(); they return any lane type.
    for side, w in (("left", wp.get_left_waypoint()), ("right", wp.get_right_waypoint())):
        print(f"  {side} lane : " + (f"{w.lane_type} (lane {w.lane_id})" if w else "none"))
    print(f"  lane object: {_lane_str(wp.get_lane())} (map-waypoints lane --road {wp.road_id} --section {wp.section_id} --lane {wp.lane_id})")


def cmd_navigate(args):
    world = _world()
    x, y, z = (float(v) for v in args.at.split(","))
    wp = world.get_map().get_waypoint(carla.Location(x, y, z))
    if wp is None:
        print("no lane near that point"); return
    path = [wp]
    for _ in range(args.steps):
        nxt = path[-1].next(args.dist)
        if not nxt:
            break
        path.append(nxt[0])   # take the first branch at forks
    print(f"navigated {len(path)-1} steps of {args.dist}m from ({x},{y},{z}):")
    for i, w in enumerate(path):
        l = w.transform.location
        print(f"  {i:3d}  road={w.road_id} lane={w.lane_id} s={w.s:6.1f}  "
              f"({l.x:.1f},{l.y:.1f}) {'[junction]' if w.is_junction else ''}")
    if args.draw:
        dbg = world.debug
        for a, b in zip(path, path[1:]):
            la, lb = a.transform.location, b.transform.location
            dbg.draw_arrow(carla.Location(la.x, la.y, la.z + 0.3),
                           carla.Location(lb.x, lb.y, lb.z + 0.3),
                           0.15, 0.25, carla.Color(0, 128, 255), args.life)
        print(f"drew the {len(path)-1}-step path (life {args.life}s)")


def _lane_str(lane):
    return f"{lane.road_id}/{lane.section_id}/{lane.id}" if lane else "none"


def cmd_road(args):
    m = _world().get_map()
    road = m.get_road(args.id)
    if road is None:
        raise SystemExit(f"no road {args.id} on {m.name}")
    j = f" in junction {road.junction_id}" if road.is_junction else ""
    print(f"road {road.id} '{road.name}': {road.length:.1f} m{j}, {'right' if road.is_rht else 'left'}-hand traffic")
    print(f"  previous roads: {[r.id for r in road.get_previous_road()] or 'none'}")
    print(f"  next roads    : {[r.id for r in road.get_next_road()] or 'none'}")
    t = road.get_transform_at(0.0).location
    print(f"  starts at ({t.x:.1f},{t.y:.1f},{t.z:.1f})")
    for sec in road.get_sections():
        lanes = sorted(sec.get_lanes(carla.LaneType.Any), key=lambda l: l.id)
        mid = sec.s_start + sec.length / 2
        desc = ", ".join(f"{l.id}:{str(l.type).split('.')[-1]} {l.get_width(mid):.1f}m" for l in lanes if l.id != 0)
        print(f"  section {sec.id}: s={sec.s_start:.1f}..{sec.s_start + sec.length:.1f}  {desc}")


def _lane_from_args(m, args):
    if args.at:
        x, y, z = (float(v) for v in args.at.split(","))
        wp = m.get_waypoint(carla.Location(x, y, z), project_to_road=True, lane_type=carla.LaneType.Any)
        if wp is None:
            raise SystemExit("no lane near that point")
        return wp.get_lane()
    if args.road is None or args.lane is None:
        raise SystemExit("lane needs --at X,Y,Z, or --road R --lane L [--section S]")
    lane = m.get_lane(args.road, args.section, args.lane)
    if lane is None:
        raise SystemExit(f"no lane {args.road}/{args.section}/{args.lane}")
    return lane


def cmd_lane(args):
    m = _world().get_map()
    lane = _lane_from_args(m, args)
    mid = lane.s_start + lane.length / 2
    print(f"lane {_lane_str(lane)} (road/section/lane): {str(lane.type).split('.')[-1]}, {lane.length:.1f} m from s={lane.s_start:.1f}")
    print(f"  width {lane.get_width(lane.s_start):.2f} m at start, {lane.get_width(mid):.2f} m in the middle")
    print(f"  straight={lane.is_straight}  travels with the road's s: {lane.is_positive_direction}")
    print(f"  left: {_lane_str(lane.get_left_lane())}   right: {_lane_str(lane.get_right_lane())}")
    print(f"  previous: {[_lane_str(l) for l in lane.get_previous_lane()] or 'none'}")
    print(f"  next    : {[_lane_str(l) for l in lane.get_next_lane()] or 'none'}")
    if args.draw:
        wps = lane.get_waypoints(2.0)
        dbg = _world().debug
        for a, b in zip(wps, wps[1:]):
            dbg.draw_line(a.transform.location + carla.Location(z=0.3), b.transform.location + carla.Location(z=0.3),
                          0.15, carla.Color(0, 200, 255), args.life)
        print(f"drew the lane centre, {len(wps)} waypoints (life {args.life}s)")


def cmd_junction(args):
    world = _world()
    j = world.get_map().get_junction_by_id(args.id)
    if j is None:
        raise SystemExit(f"no junction {args.id}")
    lt = carla.LaneType.Driving
    entries, exits = j.get_entry_waypoints(lt), j.get_exit_waypoints(lt)
    c = j.bounding_box.location
    print(f"junction {j.id} at ({c.x:.0f},{c.y:.0f}): {len(j.get_connecting_roads())} connecting roads inside")
    print(f"  adjacent roads: {sorted(r.id for r in j.get_adjacent_roads())}")
    print(f"  entry lanes ({len(entries)}): {[_lane_str(l) for l in j.get_entry_lanes(lt)]}")
    print(f"  exit lanes  ({len(exits)}): {[_lane_str(l) for l in j.get_exit_lanes(lt)]}")
    if args.draw:
        dbg = world.debug
        for wps, col, tag in ((entries, carla.Color(0, 255, 0), "in"), (exits, carla.Color(255, 0, 0), "out")):
            for w in wps:
                loc = w.transform.location + carla.Location(z=0.5)
                dbg.draw_arrow(loc, loc + w.transform.get_forward_vector() * 3, 0.15, 0.3, col, args.life)
                dbg.draw_string(loc + carla.Location(z=1), f"{tag} {w.road_id}/{w.lane_id}", False, col, args.life)
        print(f"drew {len(entries)} entries (green) and {len(exits)} exits (red) (life {args.life}s)")


def cmd_landmarks(args):
    world = _world()
    m = world.get_map()
    if args.near:
        x, y, z = (float(v) for v in args.near.split(","))
        wp = m.get_waypoint(carla.Location(x, y, z))
        if wp is None:
            print("no lane near that point"); return
        marks = wp.get_landmarks(args.distance)          # landmarks ahead in-lane
        scope = f"within {args.distance} m ahead of ({x:.0f},{y:.0f})"
    else:
        marks = m.get_all_landmarks()                    # every OpenDRIVE signal
        scope = "on the whole map"
    print(f"{len(marks)} landmark(s) {scope}:")
    for lm in marks[:args.limit]:
        val = f" value={lm.value}{lm.unit}" if lm.value else ""
        loc = lm.transform.location
        print(f"  {lm.name or '(unnamed)':22s} type={lm.type:>4s} sub={lm.sub_type or '-':>4s}"
              f"{val}  road={lm.road_id} ({loc.x:.0f},{loc.y:.0f})")
    if len(marks) > args.limit:
        print(f"  ... and {len(marks) - args.limit} more (raise --limit)")
    print("  note: 'type' is the OpenDRIVE signal code (e.g. 274 = speed limit); "
          "speed limits carry value+unit.")


def main() -> None:
    p = argparse.ArgumentParser(description="Explore a CARLA map's road network.")
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("summary", help="rundown of the whole map").set_defaults(func=cmd_summary)

    pt = sub.add_parser("topology", help="list/draw road topology")
    pt.add_argument("--draw", action="store_true"); pt.add_argument("--life", type=float, default=120.0)
    pt.set_defaults(func=cmd_topology)

    pj = sub.add_parser("junctions", help="enumerate junctions (basis for NL resolution)")
    pj.add_argument("--arms", type=int, help="filter to junctions with this many arms (e.g. 4)")
    pj.add_argument("--draw", action="store_true"); pj.add_argument("--life", type=float, default=120.0)
    pj.set_defaults(func=cmd_junctions)

    pw = sub.add_parser("waypoint", help="nearest driving waypoint + lane facts")
    pw.add_argument("--at", required=True, help="X,Y,Z world location")
    pw.set_defaults(func=cmd_waypoint)

    pn = sub.add_parser("navigate", help="walk next() from a point")
    pn.add_argument("--at", required=True, help="X,Y,Z world location")
    pn.add_argument("--dist", type=float, default=2.0, help="step distance in m")
    pn.add_argument("--steps", type=int, default=20, help="number of steps")
    pn.add_argument("--draw", action="store_true"); pn.add_argument("--life", type=float, default=120.0)
    pn.set_defaults(func=cmd_navigate)

    pr = sub.add_parser("road", help="one OpenDRIVE road: length, neighbours, sections and lanes")
    pr.add_argument("--id", type=int, required=True)
    pr.set_defaults(func=cmd_road)

    pl = sub.add_parser("lane", help="one lane: width, neighbours, what it connects to")
    pl.add_argument("--at", help="X,Y,Z world location (the lane under it)")
    pl.add_argument("--road", type=int); pl.add_argument("--section", type=int, default=0); pl.add_argument("--lane", type=int)
    pl.add_argument("--draw", action="store_true"); pl.add_argument("--life", type=float, default=120.0)
    pl.set_defaults(func=cmd_lane)

    pjj = sub.add_parser("junction", help="one junction: adjacent roads, entry and exit lanes")
    pjj.add_argument("--id", type=int, required=True)
    pjj.add_argument("--draw", action="store_true"); pjj.add_argument("--life", type=float, default=120.0)
    pjj.set_defaults(func=cmd_junction)

    pk = sub.add_parser("landmarks", help="OpenDRIVE signals: speed limits, signs")
    pk.add_argument("--near", help="X,Y,Z — landmarks ahead in that lane (else whole map)")
    pk.add_argument("--distance", type=float, default=100.0, help="look-ahead when --near (m)")
    pk.add_argument("--limit", type=int, default=25)
    pk.set_defaults(func=cmd_landmarks)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
