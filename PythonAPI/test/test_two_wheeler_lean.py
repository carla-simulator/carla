#!/usr/bin/env python3
"""Check inward lean on a dedicated CARLA server (replaces its current map).

Example: python test_two_wheeler_lean.py --port 4654 --output lean.json
Works with a -nullrhi server. Tests all seven legacy two-wheelers on a flat,
wide OpenDRIVE road, so road banking and collisions cannot mimic body lean.
"""

import argparse
import json
import math
from pathlib import Path

import carla


VEHICLES = (
    "vehicle.bh.crossbike", "vehicle.diamondback.century",
    "vehicle.gazelle.omafiets", "vehicle.harley-davidson.low_rider",
    "vehicle.kawasaki.ninja", "vehicle.vespa.zx125", "vehicle.yamaha.yzf",
)
ROAD = """<?xml version="1.0"?>
<OpenDRIVE>
 <header revMajor="1" revMinor="4" name="lean_test" version="1.0"
         date="" north="0" south="0" east="0" west="0"/>
 <road name="flat" length="1000" id="0" junction="-1">
  <planView><geometry s="0" x="0" y="0" hdg="0" length="1000"><line/></geometry></planView>
  <elevationProfile><elevation s="0" a="0" b="0" c="0" d="0"/></elevationProfile>
  <lanes><laneSection s="0">
   <center><lane id="0" type="none" level="false"><roadMark sOffset="0" type="none"/></lane></center>
   <right><lane id="-1" type="driving" level="false">
    <width sOffset="0" a="200" b="0" c="0" d="0"/>
    <roadMark sOffset="0" type="none"/>
   </lane></right>
  </laneSection></lanes>
 </road>
</OpenDRIVE>"""


def run_case(world, blueprint, steer):
    vehicle = world.spawn_actor(
        blueprint, carla.Transform(carla.Location(x=500, y=100, z=1)))
    samples = []
    try:
        for tick in range(180):
            speed = vehicle.get_velocity().length()
            vehicle.apply_control(carla.VehicleControl(
                throttle=max(0, min(0.6, (6 - speed) * 0.4)),
                brake=max(0, min(0.2, (speed - 6) * 0.2)),
                steer=steer if tick >= 80 else 0))
            world.tick()
            transform = vehicle.get_transform()
            # Positive means the top of the vehicle tilts to its right.
            # Measure geometry, independent of Euler roll sign conventions.
            right = carla.Rotation(yaw=transform.rotation.yaw).get_right_vector()
            lean = math.degrees(math.asin(max(-1, min(
                1, transform.get_up_vector().dot(right)))))
            samples.append({
                "time": tick * 0.05,
                "speed": vehicle.get_velocity().length(),
                "lean_right": lean,
                "yaw_rate": vehicle.get_angular_velocity().z,
            })
    finally:
        vehicle.destroy()
        world.tick()

    means = {key: sum(row[key] for row in samples[-40:]) / 40
             for key in ("speed", "lean_right", "yaw_rate")}
    # Require actual movement/turning: a stationary or non-steering vehicle
    # must not pass merely because the lean sign happens to match the input.
    passed = means["speed"] > 4
    if steer:
        direction = math.copysign(1, steer)
        passed &= means["yaw_rate"] * direction > 5
        passed &= means["lean_right"] * direction > 0.5
    else:
        passed &= abs(means["lean_right"]) < 1
        passed &= abs(means["yaw_rate"]) < 1
    result = dict(vehicle=blueprint.id, steer=steer, passed=passed, **means)
    print(json.dumps(result), flush=True)
    return dict(result, samples=samples)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, required=True,
                        help="Dedicated test server port; its map will be replaced")
    parser.add_argument("--output", type=Path, default=Path("two_wheeler_lean.json"))
    args = parser.parse_args()
    client = carla.Client(args.host, args.port)
    client.set_timeout(120)
    world = client.generate_opendrive_world(
        ROAD, carla.OpendriveGenerationParameters(
            vertex_distance=10, wall_height=0, additional_width=0))
    original_settings = world.get_settings()
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)
    results = []
    try:
        library = world.get_blueprint_library()
        for name in VEHICLES:
            for steer in (-0.35, 0, 0.35):
                results.append(run_case(world, library.find(name), steer))
    finally:
        world.apply_settings(original_settings)
        args.output.write_text(json.dumps(results, indent=2) + "\n")
    failures = sum(not row["passed"] for row in results)
    print(f"{len(results) - failures}/{len(results)} cases passed")
    return int(failures != 0)


if __name__ == "__main__":
    raise SystemExit(main())
