#!/usr/bin/env python3
"""Export live CARLA traffic-light states keyed by Lanelet2 relation ID.

This process is deliberately a non-ticking CARLA client. It matches each live
traffic-light actor to the generated map's regulatory element by stop-line
center, then atomically writes a small JSON snapshot for the ROS-side adapter.
"""

import argparse
import json
import math
import os
import signal
import sys
import time
import xml.etree.ElementTree as ET

try:
    import carla
except ImportError:
    sys.exit(
        "carla_traffic_light_state_source.py: cannot 'import carla'. Install "
        "the CARLA Python wheel (PythonAPI/carla/dist) into this environment "
        "first."
    )


# Wire-contract color codes shared with autoware_traffic_light_state_publisher.py
# (a ROS 2 node that cannot import this module). Both files must stay in sync;
# these also match autoware_perception_msgs.msg.TrafficLightElement's
# RED/AMBER/GREEN/UNKNOWN values.
TL_UNKNOWN = 0
TL_RED = 1
TL_AMBER = 2
TL_GREEN = 3

COLORS = {
    carla.TrafficLightState.Red: TL_RED,
    carla.TrafficLightState.Yellow: TL_AMBER,
    carla.TrafficLightState.Green: TL_GREEN,
    carla.TrafficLightState.Off: TL_UNKNOWN,
    carla.TrafficLightState.Unknown: TL_UNKNOWN,
}

MAX_CONSECUTIVE_ERRORS = 10
ERROR_BACKOFF_SEC = 0.5
# Safety-net refresh cadence even when nothing changed. Keep this well below
# autoware_traffic_light_state_publisher.py's --max-age (default 1.0s): that
# default assumes max_age = 5 * WRITE_INTERVAL_SEC, so a real tick stall (not
# just a slow debounce cycle) is what trips the stale-UNKNOWN fail-safe.
# Changing one constant means checking the other.
WRITE_INTERVAL_SEC = 0.2


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=2000)
    parser.add_argument("--map", required=True, dest="map_path")
    parser.add_argument("--output", required=True)
    parser.add_argument("--max-match-distance", type=float, default=1.0)
    parser.add_argument("--timeout", type=float, default=10.0)
    return parser.parse_args()


def map_stop_lines(path):
    """Parse the generated lanelet2 map's traffic-light regulatory elements.

    Returns (stop_lines, opendrive_id_to_relation):
      stop_lines: relation_id -> stop-line centroid (x, y), used by the
        distance-based fallback matcher.
      opendrive_id_to_relation: CARLA opendrive id (str) -> relation_id, read
        from each relation's `carla_opendrive_id` tag. Maps generated before
        that tag existed simply produce an empty dict here.
    """
    root = ET.parse(path).getroot()
    nodes = {}
    for node in root.findall("node"):
        tags = {tag.get("k"): tag.get("v") for tag in node.findall("tag")}
        if "local_x" in tags and "local_y" in tags:
            nodes[node.get("id")] = (float(tags["local_x"]), float(tags["local_y"]))
    ways = {way.get("id"): way for way in root.findall("way")}
    stop_lines = {}
    opendrive_id_to_relation = {}
    for relation in root.findall("relation"):
        tags = {tag.get("k"): tag.get("v") for tag in relation.findall("tag")}
        if tags.get("type") != "regulatory_element" or tags.get("subtype") != "traffic_light":
            continue
        relation_id = int(relation.get("id"))
        if "carla_opendrive_id" in tags:
            opendrive_id_to_relation[tags["carla_opendrive_id"]] = relation_id
        stop_ref = next(
            (member.get("ref") for member in relation.findall("member")
             if member.get("role") == "ref_line"),
            None,
        )
        if stop_ref not in ways:
            continue
        points = [nodes[nd.get("ref")] for nd in ways[stop_ref].findall("nd")
                  if nd.get("ref") in nodes]
        if points:
            stop_lines[relation_id] = (
                sum(point[0] for point in points) / len(points),
                sum(point[1] for point in points) / len(points),
            )
    return stop_lines, opendrive_id_to_relation


def actor_stop_center(actor):
    # CARLA's y-axis is inverted relative to the Autoware map frame this value
    # is compared against (see carla_common.carla_xyz_to_map: y := -y). Do not
    # simplify this negation away.
    points = [
        (waypoint.transform.location.x, -waypoint.transform.location.y)
        for waypoint in actor.get_stop_waypoints()
    ]
    if not points:
        return None
    return (
        sum(point[0] for point in points) / len(points),
        sum(point[1] for point in points) / len(points),
    )


def match_actors(actors, stop_lines, opendrive_id_to_relation, max_distance):
    matches = {}
    used_relations = set()
    remaining = []
    id_match_count = 0

    for actor in actors:
        relation_id = opendrive_id_to_relation.get(str(actor.get_opendrive_id()))
        if relation_id is None or relation_id in used_relations:
            remaining.append(actor)
            continue
        matches[actor.id] = (actor, relation_id)
        used_relations.add(relation_id)
        id_match_count += 1
        print(
            f"CARLA light {actor.get_opendrive_id()} -> Lanelet2 relation {relation_id} "
            f"(opendrive id match)",
            flush=True,
        )

    distance_match_count = 0
    for actor in remaining:
        center = actor_stop_center(actor)
        if center is None:
            continue
        candidates = [
            (math.hypot(center[0] - point[0], center[1] - point[1]), relation_id)
            for relation_id, point in stop_lines.items()
            if relation_id not in used_relations
        ]
        if not candidates:
            continue
        distance, relation_id = min(candidates)
        if distance > max_distance:
            raise RuntimeError(
                f"CARLA light {actor.get_opendrive_id()} has no Lanelet2 stop line within "
                f"{max_distance:.2f} m (nearest relation {relation_id}: {distance:.3f} m)"
            )
        matches[actor.id] = (actor, relation_id)
        used_relations.add(relation_id)
        distance_match_count += 1
        print(
            f"CARLA light {actor.get_opendrive_id()} -> Lanelet2 relation {relation_id} "
            f"({distance:.3f} m, distance fallback)",
            flush=True,
        )

    print(
        f"traffic-light matching summary: {id_match_count} by opendrive id, "
        f"{distance_match_count} by distance fallback",
        flush=True,
    )
    return matches


def write_snapshot(path, matches, last_write, dead_actor_ids):
    """Write the snapshot if its content changed or the safety-net interval elapsed.

    A relation is never dropped from `signals` once an actor is matched to
    it, even after that actor dies: it keeps being reported, as TL_UNKNOWN,
    so the group is never silently missing from the payload (a missing group
    would let the publisher's fail-safe check pass while Autoware still
    latches that light's last known color forever).

    `dead_actor_ids` is a set of actor ids already found dead; it is updated
    in place as new deaths are discovered. `last_write` is None initially, or
    the {"signature", "written_at"} dict returned by the previous call.
    Returns (new_last_write, newly_dead) where `newly_dead` is a list of
    (actor_id, relation_id) pairs discovered dead on this call.
    """
    newly_dead = []
    signals = []
    for actor_id, (actor, relation_id) in matches.items():
        if actor_id in dead_actor_ids:
            signals.append({"relation_id": relation_id, "color": TL_UNKNOWN})
            continue
        if not actor.is_alive:
            dead_actor_ids.add(actor_id)
            newly_dead.append((actor_id, relation_id))
            signals.append({"relation_id": relation_id, "color": TL_UNKNOWN})
            continue
        signals.append({"relation_id": relation_id, "color": COLORS.get(actor.state, TL_UNKNOWN)})

    signature = tuple(sorted((entry["relation_id"], entry["color"]) for entry in signals))
    now = time.time()
    if (
        last_write is not None
        and signature == last_write["signature"]
        and now - last_write["written_at"] < WRITE_INTERVAL_SEC
    ):
        return last_write, newly_dead

    payload = {"written_at": now, "signals": signals}
    temporary = f"{path}.tmp"
    with open(temporary, "w", encoding="utf-8") as stream:
        json.dump(payload, stream, separators=(",", ":"))
    os.replace(temporary, path)
    return {"signature": signature, "written_at": now}, newly_dead


def main():
    args = parse_args()
    stop_lines, opendrive_id_to_relation = map_stop_lines(args.map_path)
    if not stop_lines and not opendrive_id_to_relation:
        raise RuntimeError(f"no traffic-light regulatory elements found in {args.map_path}")

    client = carla.Client(args.host, args.port)
    client.set_timeout(args.timeout)
    world = client.get_world()
    world.wait_for_tick(args.timeout)
    actors = list(world.get_actors().filter("traffic.traffic_light*"))
    matches = match_actors(actors, stop_lines, opendrive_id_to_relation, args.max_match_distance)
    if len(matches) != len(actors):
        raise RuntimeError(f"matched only {len(matches)} of {len(actors)} CARLA traffic lights")

    stop = {"flag": False}

    def handle_signal(signum, _frame):
        print(
            f"carla_traffic_light_state_source.py: received signal {signum}, shutting down",
            flush=True,
        )
        stop["flag"] = True

    signal.signal(signal.SIGTERM, handle_signal)
    signal.signal(signal.SIGINT, handle_signal)

    last_write = None
    dead_actor_ids = set()
    consecutive_errors = 0
    while not stop["flag"]:
        try:
            last_write, newly_dead = write_snapshot(args.output, matches, last_write, dead_actor_ids)
            for actor_id, relation_id in newly_dead:
                print(
                    f"carla_traffic_light_state_source.py: actor {actor_id} "
                    f"(relation {relation_id}) is no longer alive, reporting TL_UNKNOWN for it",
                    flush=True,
                )
            world.wait_for_tick(args.timeout)
            consecutive_errors = 0
        except Exception as exc:
            consecutive_errors += 1
            print(
                f"carla_traffic_light_state_source.py: transient error "
                f"({consecutive_errors}/{MAX_CONSECUTIVE_ERRORS}): {exc}",
                flush=True,
            )
            if consecutive_errors >= MAX_CONSECUTIVE_ERRORS:
                sys.exit(
                    "carla_traffic_light_state_source.py: exceeded "
                    f"{MAX_CONSECUTIVE_ERRORS} consecutive errors, exiting"
                )
            time.sleep(ERROR_BACKOFF_SEC)


if __name__ == "__main__":
    main()
