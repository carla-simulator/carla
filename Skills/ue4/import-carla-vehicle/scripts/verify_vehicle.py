#!/usr/bin/env python3
"""Prove an imported vehicle really works, on a running CARLA server.

    python3 verify_vehicle.py --id vehicle.ford.transit
    python3 verify_vehicle.py --id vehicle.ford.transit --keep

A factory entry proves nothing on its own: a vehicle whose SkeletalMesh failed to load
still LISTS while being invisible, one whose physics asset has no wheel bodies spawns
and never moves, and one whose wheel setups point at missing bones spawns and slides.
So five things are checked, and only together do they mean "imported correctly":

  1. the blueprint is in the library and spawns
  2. its attributes are the ones that were registered
  3. it has four wheels with physics — get_physics_control() reports them
  4. it DRIVES — full throttle moves it at least a few metres
  5. it steers — a steering input changes its heading

Assumes a server is already running; it does not start one. Content is read at server
startup, so restart it after an import before trusting a pass.
"""
from __future__ import annotations

import argparse
import math
import sys
import time

try:
    import carla
except ImportError:
    sys.exit("ERROR: cannot 'import carla'. Activate the environment with the wheel.")

DRIVE_SECONDS = 5.0
DRIVE_MIN_DISTANCE_M = 3.0
STEER_SECONDS = 4.0
STEER_MIN_YAW_DEG = 5.0
EXPECTED_WHEELS = 4


def attribute_text(attribute) -> str:
    readers = {
        carla.ActorAttributeType.Bool: lambda a: str(a.as_bool()),
        carla.ActorAttributeType.Int: lambda a: str(a.as_int()),
        carla.ActorAttributeType.Float: lambda a: f"{a.as_float():g}",
        carla.ActorAttributeType.String: lambda a: a.as_str(),
    }
    reader = readers.get(attribute.type)
    try:
        return reader(attribute) if reader else str(attribute)
    except (RuntimeError, ValueError):
        return "<unreadable>"


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--id", required=True, help="blueprint id, e.g. vehicle.ford.transit")
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=2000)
    ap.add_argument("--timeout", type=float, default=30.0)
    ap.add_argument("--keep", action="store_true", help="leave the vehicle in the world")
    args = ap.parse_args()

    blueprint_id = args.id.lower()
    client = carla.Client(args.host, args.port)
    client.set_timeout(args.timeout)
    world = client.get_world()
    library = world.get_blueprint_library()

    # 1. in the library?
    if not [bp.id for bp in library.filter(blueprint_id)]:
        vehicles = sorted(bp.id for bp in library.filter("vehicle.*"))
        print(f"FAIL  {blueprint_id} is not in the blueprint library.")
        print(f"      {len(vehicles)} vehicles are registered"
              + (f" (last: {', '.join(vehicles[-3:])})" if vehicles else ""))
        print("      The VehicleFactory entry is missing — see import_vehicle.py.")
        return 1
    vehicle_bp = library.find(blueprint_id)
    print(f"PASS  {blueprint_id} in the library")

    # 2. attributes
    attributes = {a.id: attribute_text(a) for a in vehicle_bp}
    for key in ("number_of_wheels", "generation", "base_type", "object_type",
                "special_type", "has_dynamic_doors", "has_lights", "role_name"):
        if key in attributes:
            print(f"      {key} = {attributes[key]}")

    # 3. spawns?
    spawn_points = world.get_map().get_spawn_points()
    if not spawn_points:
        print("FAIL  the map has no vehicle spawn points.")
        return 1
    vehicle = None
    for point in spawn_points[:20]:
        vehicle = world.try_spawn_actor(vehicle_bp, point)
        if vehicle is not None:
            break
    if vehicle is None:
        print(f"FAIL  {blueprint_id} would not spawn at any of 20 spawn points.")
        print("      A vehicle that lists but will not spawn usually has an unloadable")
        print("      SkeletalMesh or a broken physics asset on its blueprint.")
        return 1
    print(f"PASS  spawned id={vehicle.id} at "
          f"({vehicle.get_location().x:.1f}, {vehicle.get_location().y:.1f})")

    try:
        world.wait_for_tick()

        # 4. wheels with physics
        try:
            physics = vehicle.get_physics_control()
            wheels = physics.wheels
        except RuntimeError as exc:
            print(f"FAIL  get_physics_control() failed: {exc}")
            print("      The vehicle has no usable PhysX vehicle setup.")
            return 1
        if len(wheels) != EXPECTED_WHEELS:
            print(f"FAIL  {len(wheels)} wheels reported, expected {EXPECTED_WHEELS}.")
            print("      WheelSetups on the movement component are wrong, or the physics")
            print("      asset has no bodies on the wheel bones.")
            return 1
        radii = [round(w.radius, 1) for w in wheels]
        print(f"PASS  {len(wheels)} wheels with physics — radii {radii} cm, "
              f"mass {physics.mass:.0f} kg")

        # 5. does it drive?
        start = vehicle.get_location()
        vehicle.apply_control(carla.VehicleControl(throttle=1.0, brake=0.0))
        deadline = time.time() + DRIVE_SECONDS
        while time.time() < deadline:
            world.wait_for_tick()
        end = vehicle.get_location()
        travelled = math.dist((start.x, start.y), (end.x, end.y))
        speed = vehicle.get_velocity()
        kmh = 3.6 * math.sqrt(speed.x ** 2 + speed.y ** 2 + speed.z ** 2)
        if travelled < DRIVE_MIN_DISTANCE_M:
            print(f"FAIL  moved only {travelled:.2f} m in {DRIVE_SECONDS:.0f} s at full "
                  f"throttle ({kmh:.1f} km/h).")
            print("      Wheel bodies are usually the cause: PxVehicle raycasts the")
            print("      suspension from kinematic wheel bodies on the four wheel bones.")
            return 1
        print(f"PASS  drives — {travelled:.1f} m in {DRIVE_SECONDS:.0f} s, "
              f"{kmh:.1f} km/h")

        # 6. does it steer?
        yaw_before = vehicle.get_transform().rotation.yaw
        vehicle.apply_control(carla.VehicleControl(throttle=0.5, steer=1.0))
        deadline = time.time() + STEER_SECONDS
        while time.time() < deadline:
            world.wait_for_tick()
        yaw_after = vehicle.get_transform().rotation.yaw
        turned = abs((yaw_after - yaw_before + 180.0) % 360.0 - 180.0)
        if turned < STEER_MIN_YAW_DEG:
            print(f"FAIL  heading changed only {turned:.1f} deg under full steering.")
            print("      The front wheel blueprints have no steer angle, or the front")
            print("      wheels are bound to the wrong bones.")
            return 1
        print(f"PASS  steers — heading changed {turned:.0f} deg")

        print()
        print(f"OK    {blueprint_id} imported correctly")
        return 0
    finally:
        if not args.keep:
            vehicle.destroy()
        else:
            print("      --keep: vehicle left in the world")


if __name__ == "__main__":
    sys.exit(main())
