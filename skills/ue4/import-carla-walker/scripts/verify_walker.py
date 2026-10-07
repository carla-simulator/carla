#!/usr/bin/env python3
"""Prove an imported walker really works, on a running CARLA server.

    python3 verify_walker.py --id 0053
    python3 verify_walker.py --id 0053 --keep     # leave it walking

Assets on disk prove nothing, and neither does a factory entry: a walker whose
SkeletalMesh failed to load still LISTS in the blueprint library while being
invisible, and one whose capsule does not match its mesh spawns fine and then
floats above the pavement or sinks into it. So this checks four things that only
together mean "imported correctly":

  1. the blueprint is in the library and spawns
  2. the mesh is really bound  — get_bones() returns the 26 GEN3 bones
  3. it stands ON the ground   — the BOTTOM of its bounding box meets the road
  4. the AI controller moves it — the location changes over a few seconds

Run with the CARLA client env active, against a server from the run-carla-server
skill. Walkers need navigation data, so the map must have its .bin — every stock
town does. The default -nullrhi server is enough; nothing here renders.
"""
from __future__ import annotations

import argparse
import math
import sys
import time

try:
    import carla
except ImportError:
    sys.exit(
        "ERROR: cannot 'import carla'.\n"
        "       Activate the environment holding the CARLA wheel."
    )

# GEN3 rig: what get_bones() must return for the mesh to be the one we imported.
GEN3_BONE_COUNT = 26

# How far the bottom of the walker's bounding box may sit from the road surface.
# Covers sidewalk height variation and the capsule's settle. NOT applied to the
# actor's own z — that is the capsule centre, ~0.93 m up on a GEN3 walker.
GROUND_TOLERANCE_M = 0.6

# How long the AI controller gets to prove it can move the walker.
WALK_SECONDS = 4.0
WALK_MIN_DISTANCE_M = 0.3


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--id", required=True, help="the walker id, e.g. 0053")
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=2000)
    ap.add_argument("--timeout", type=float, default=30.0)
    ap.add_argument("--keep", action="store_true",
                    help="leave the walker in the world instead of destroying it")
    args = ap.parse_args()

    blueprint_id = f"walker.pedestrian.{args.id.lower()}"

    client = carla.Client(args.host, args.port)
    client.set_timeout(args.timeout)
    world = client.get_world()
    library = world.get_blueprint_library()

    # 1. in the library?
    matches = [bp.id for bp in library.filter(blueprint_id)]
    if not matches:
        peds = sorted(bp.id for bp in library.filter("walker.pedestrian.*"))
        print(f"FAIL  {blueprint_id} is not in the blueprint library.")
        print(f"      {len(peds)} walkers are registered"
              + (f" (last: {', '.join(peds[-3:])})" if peds else ""))
        print("      The factory entry is missing — see the registration step of "
              "import_walker.py.")
        return 1
    walker_bp = library.find(blueprint_id)
    # as_str() raises on non-string attributes ("bad attribute cast"), so each is
    # read through the accessor matching its type.
    readers = {
        carla.ActorAttributeType.Bool: lambda a: str(a.as_bool()),
        carla.ActorAttributeType.Int: lambda a: str(a.as_int()),
        carla.ActorAttributeType.Float: lambda a: f"{a.as_float():g}",
        carla.ActorAttributeType.String: lambda a: a.as_str(),
    }
    attributes = {}
    for attribute in walker_bp:
        reader = readers.get(attribute.type)
        try:
            attributes[attribute.id] = reader(attribute) if reader else str(attribute)
        except (RuntimeError, ValueError):
            attributes[attribute.id] = "<unreadable>"
    print(f"PASS  {blueprint_id} in the library")
    for key in ("gender", "age", "generation", "role_name", "speed"):
        if key in attributes:
            print(f"      {key} = {attributes[key]}")

    # 2. spawns?
    spawn_location = world.get_random_location_from_navigation()
    if spawn_location is None:
        print("FAIL  the map returned no navigation location — no pedestrian nav "
              "data for this map.")
        return 1
    transform = carla.Transform(spawn_location)
    walker = world.try_spawn_actor(walker_bp, transform)
    if walker is None:
        print(f"FAIL  {blueprint_id} would not spawn at {spawn_location}.")
        print("      A walker that lists but will not spawn usually has an "
              "unloadable SkeletalMesh on its blueprint.")
        return 1
    print(f"PASS  spawned id={walker.id} at "
          f"({spawn_location.x:.1f}, {spawn_location.y:.1f}, {spawn_location.z:.1f})")

    controller = None
    try:
        # 3. is the mesh really bound? Bones come from the skeletal mesh, so an
        #    empty or short list means the blueprint points at nothing.
        world.wait_for_tick()
        bones = walker.get_bones().bone_transforms
        names = {bone.name for bone in bones}
        crl = {n for n in names if n.startswith("crl_")}
        if len(crl) < GEN3_BONE_COUNT:
            print(f"FAIL  get_bones() returned {len(crl)} crl_* bones, expected "
                  f"{GEN3_BONE_COUNT} — the mesh is not the GEN3 one.")
            return 1
        print(f"PASS  mesh bound — {len(crl)} GEN3 bones "
              f"(root: {'crl_root' in names})")

        # 4. on the ground?
        #
        # NOT actor z against the road: a walker's origin is its capsule centre, and
        # GEN3 walkers carry a 1.86 m capsule around a 1.2 m mesh, so a stock walker
        # sits ~0.93 m "above" the road by that measure. What matters is where the
        # visible body ends, so compare the BOTTOM of the actor's bounding box —
        # which CARLA derives from the mesh, at its blueprint scale.
        waypoint = world.get_map().get_waypoint(
            walker.get_location(), project_to_road=True)
        location = walker.get_location()
        box = walker.bounding_box
        bottom_z = location.z + box.location.z - box.extent.z
        delta_z = bottom_z - waypoint.transform.location.z
        if abs(delta_z) > GROUND_TOLERANCE_M:
            print(f"FAIL  the walker's feet are {delta_z:+.2f} m from the road "
                  f"surface (tolerance {GROUND_TOLERANCE_M} m).")
            print("      The capsule half-height and the mesh's relative z do not "
                  "match this mesh — it is floating or sunk.")
            print(f"      actor z {location.z:.2f}, box half-height "
                  f"{box.extent.z:.2f}, road z "
                  f"{waypoint.transform.location.z:.2f}")
            return 1
        print(f"PASS  feet on the ground ({delta_z:+.2f} m from the surface, "
              f"body {2 * box.extent.z:.2f} m tall)")

        # 5. does it walk?
        controller_bp = library.find("controller.ai.walker")
        controller = world.spawn_actor(controller_bp, carla.Transform(), attach_to=walker)
        controller.start()
        target = world.get_random_location_from_navigation()
        if target is not None:
            controller.go_to_location(target)
        controller.set_max_speed(1.4)

        start = walker.get_location()
        deadline = time.time() + WALK_SECONDS
        while time.time() < deadline:
            world.wait_for_tick()
        end = walker.get_location()
        travelled = math.dist((start.x, start.y, start.z), (end.x, end.y, end.z))
        if travelled < WALK_MIN_DISTANCE_M:
            print(f"FAIL  moved only {travelled:.2f} m in {WALK_SECONDS:.0f} s.")
            print("      The AI controller is driving it but the walker is stuck — "
                  "usually the anim blueprint is not ABP_GEN3,")
            print("      or the mesh is bound to a skeleton the GEN3 animations do "
                  "not target.")
            return 1
        print(f"PASS  walks — {travelled:.2f} m in {WALK_SECONDS:.0f} s")

        print()
        print(f"OK    {blueprint_id} imported correctly")
        return 0
    finally:
        if not args.keep:
            if controller is not None:
                controller.stop()
                controller.destroy()
            walker.destroy()
        else:
            print("      --keep: walker left in the world")


if __name__ == "__main__":
    sys.exit(main())
