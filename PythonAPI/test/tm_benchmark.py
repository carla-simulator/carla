#!/usr/bin/env python3
# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Offline Traffic Manager navigation benchmark.

Runs a single TM-controlled vehicle through a deterministic matrix of
(spawn point x target speed) scenarios in synchronous mode and scores each
run on lane keeping, road adherence, speed adherence and collisions.
Intended for one-change-at-a-time A/B validation of TM modifications:

    # Score the currently installed client/server pair:
    python3 tm_benchmark.py run --label baseline --out baseline.json

    # After rebuilding with a candidate change:
    python3 tm_benchmark.py run --label candidate --out candidate.json

    # Compare:
    python3 tm_benchmark.py compare baseline.json candidate.json

The benchmark is deterministic per (server build, client build, seed):
synchronous ticking, a seeded Traffic Manager, one vehicle, no other
traffic, traffic lights/signs ignored so control quality is measured
rather than intersection etiquette.
"""

import argparse
import json
import math
import os
import random
import sys
import time

import carla

# The agents package ships in source form next to this script; make the
# `--controller agent` reference controller importable without PYTHONPATH.
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                "..", "carla"))

# ---------------------------------------------------------------------------
# Defaults and acceptance thresholds
# ---------------------------------------------------------------------------

DEFAULT_MAP = "Town10HD_Opt"
DEFAULT_SPEEDS_KMH = [30.0, 60.0, 90.0, 130.0]
DEFAULT_NUM_SPAWNS = 3
DEFAULT_RUN_SIM_SECONDS = 40.0
DEFAULT_SEED = 7
FIXED_DELTA_SECONDS = 0.05
VEHICLE_PREFERENCE = [
    "vehicle.lincoln.mkz",
    "vehicle.tesla.model3",
    "vehicle.dodge.charger",
    "vehicle.audi.tt",
]
GRACE_SECONDS = 3.0          # ignore metrics while the vehicle gets rolling
STUCK_SPEED_MS = 0.5         # below this counts toward stuck time
STUCK_LIMIT_SECONDS = 6.0    # continuous stuck time that fails a run

# Per-run acceptance thresholds. A run PASSES when all of these hold.
THRESHOLDS = {
    "offroad_pct_max": 0.0,       # vehicle must never leave the drivable area
    "lane_dev_rms_max": 0.6,      # metres from nearest lane centre
    "lane_dev_p95_max": 1.2,
    "lane_dev_max_max": 1.75,     # half a lane: beyond this we are in the next lane
    "collisions_max": 0,
    "stuck_max_seconds": STUCK_LIMIT_SECONDS,
    "speed_adherence_min": 0.5,   # mean speed / target speed (corners may slow us)
}


# ---------------------------------------------------------------------------
# Metric collection
# ---------------------------------------------------------------------------

class RunRecorder:
    """Accumulates per-tick metrics for one benchmark run."""

    def __init__(self, world_map, target_kmh, trace=False):
        self.map = world_map
        self.target_ms = target_kmh / 3.6
        self.ticks = 0
        self.measured_ticks = 0
        self.offroad_ticks = 0
        self.deviations = []
        self.junction_deviations = []
        self.speeds_ms = []
        self.collisions = []
        self.distance_m = 0.0
        self.stuck_seconds = 0.0
        self.max_stuck_seconds = 0.0
        self.trajectory = []          # downsampled [x, y] for plotting
        self.trace = [] if trace else None   # per-tick [t, speed, dev, steer, throttle, brake, yaw_rate]
        self._last_location = None
        self._last_yaw = None
        self._traj_countdown = 0

    def on_collision(self, event):
        other = event.other_actor.type_id if event.other_actor else "unknown"
        self.collisions.append({"other": other, "frame": event.frame})

    def on_tick(self, vehicle, sim_elapsed_s):
        self.ticks += 1
        location = vehicle.get_location()
        velocity = vehicle.get_velocity()
        speed = math.sqrt(velocity.x ** 2 + velocity.y ** 2 + velocity.z ** 2)

        if self._last_location is not None:
            self.distance_m += math.hypot(
                location.x - self._last_location.x,
                location.y - self._last_location.y)
        self._last_location = location

        if self._traj_countdown <= 0:
            self.trajectory.append([round(location.x, 2), round(location.y, 2)])
            self._traj_countdown = int(0.5 / FIXED_DELTA_SECONDS)
        self._traj_countdown -= 1

        if sim_elapsed_s < GRACE_SECONDS:
            return

        self.measured_ticks += 1
        self.speeds_ms.append(speed)

        if speed < STUCK_SPEED_MS:
            self.stuck_seconds += FIXED_DELTA_SECONDS
            self.max_stuck_seconds = max(self.max_stuck_seconds, self.stuck_seconds)
        else:
            self.stuck_seconds = 0.0

        on_road = self.map.get_waypoint(
            location, project_to_road=False, lane_type=carla.LaneType.Driving)
        if on_road is None:
            self.offroad_ticks += 1

        nearest = self.map.get_waypoint(
            location, project_to_road=True, lane_type=carla.LaneType.Driving)
        deviation = None
        if nearest is not None:
            wp_loc = nearest.transform.location
            deviation = math.hypot(location.x - wp_loc.x, location.y - wp_loc.y)
            # Inside junctions the nearest-lane-center reference is ill-defined
            # (overlapping lane geometries make it flip between references),
            # so junction ticks are scored separately and excluded from the
            # lane-keeping thresholds.
            if nearest.is_junction:
                self.junction_deviations.append(deviation)
            else:
                self.deviations.append(deviation)

        if self.trace is not None:
            control = vehicle.get_control()
            yaw = vehicle.get_transform().rotation.yaw
            yaw_rate = 0.0
            if self._last_yaw is not None:
                dyaw = (yaw - self._last_yaw + 180.0) % 360.0 - 180.0
                yaw_rate = dyaw / FIXED_DELTA_SECONDS
            self._last_yaw = yaw
            self.trace.append([
                round(sim_elapsed_s, 2),
                round(speed, 2),
                round(deviation, 3) if deviation is not None else None,
                round(control.steer, 4),
                round(control.throttle, 3),
                round(control.brake, 3),
                round(yaw_rate, 2),
            ])

    def summary(self, spawn_index, target_kmh):
        devs = sorted(self.deviations)
        speeds = self.speeds_ms

        def percentile(values, p):
            if not values:
                return None
            idx = min(len(values) - 1, int(round(p / 100.0 * (len(values) - 1))))
            return values[idx]

        dev_rms = (math.sqrt(sum(d * d for d in devs) / len(devs))
                   if devs else None)
        mean_speed = sum(speeds) / len(speeds) if speeds else 0.0
        result = {
            "spawn_index": spawn_index,
            "target_kmh": target_kmh,
            "measured_ticks": self.measured_ticks,
            "distance_m": round(self.distance_m, 1),
            "offroad_pct": (round(100.0 * self.offroad_ticks / self.measured_ticks, 2)
                            if self.measured_ticks else None),
            "lane_dev_rms_m": round(dev_rms, 3) if dev_rms is not None else None,
            "lane_dev_p95_m": (round(percentile(devs, 95), 3)
                               if devs else None),
            "lane_dev_max_m": round(devs[-1], 3) if devs else None,
            "mean_speed_kmh": round(mean_speed * 3.6, 1),
            "speed_adherence": (round(mean_speed / self.target_ms, 3)
                                if self.target_ms > 0 else None),
            "collisions": len(self.collisions),
            "collision_details": self.collisions[:10],
            "junction_dev_p95_m": (round(percentile(sorted(self.junction_deviations), 95), 3)
                                   if self.junction_deviations else None),
            "max_stuck_seconds": round(self.max_stuck_seconds, 1),
            "trajectory": self.trajectory,
        }
        if self.trace is not None:
            result["trace_columns"] = ["t", "speed_ms", "dev_m", "steer",
                                       "throttle", "brake", "yaw_rate_deg_s"]
            result["trace"] = self.trace
        result["pass"], result["failures"] = evaluate(result)
        return result


def evaluate(run):
    """Apply THRESHOLDS to a run summary. Returns (passed, failure list)."""
    failures = []
    checks = [
        ("offroad_pct", "offroad_pct_max", "<="),
        ("lane_dev_rms_m", "lane_dev_rms_max", "<="),
        ("lane_dev_p95_m", "lane_dev_p95_max", "<="),
        ("lane_dev_max_m", "lane_dev_max_max", "<="),
        ("collisions", "collisions_max", "<="),
        ("max_stuck_seconds", "stuck_max_seconds", "<="),
        ("speed_adherence", "speed_adherence_min", ">="),
    ]
    for metric, threshold_key, op in checks:
        value = run.get(metric)
        limit = THRESHOLDS[threshold_key]
        if value is None:
            failures.append(f"{metric}: no data")
            continue
        ok = value <= limit if op == "<=" else value >= limit
        if not ok:
            failures.append(f"{metric}={value} (limit {op} {limit})")
    return (len(failures) == 0), failures


# ---------------------------------------------------------------------------
# Benchmark driver
# ---------------------------------------------------------------------------

def pick_blueprint(world):
    library = world.get_blueprint_library()
    for candidate in VEHICLE_PREFERENCE:
        found = library.filter(candidate)
        if found:
            return found[0]
    vehicles = library.filter("vehicle.*")
    if not vehicles:
        raise RuntimeError("no vehicle blueprints available")
    return sorted(vehicles, key=lambda bp: bp.id)[0]


def pick_spawn_indices(spawn_points, num_spawns):
    n = len(spawn_points)
    if n == 0:
        raise RuntimeError("map has no spawn points")
    return [int(round(i * n / num_spawns)) % n for i in range(num_spawns)]


def run_single(client, world, traffic_manager, tm_port, spawn_index,
               target_kmh, sim_seconds, verbose, controller="tm",
               trace=False, seed=DEFAULT_SEED):
    world_map = world.get_map()
    spawn = world_map.get_spawn_points()[spawn_index]
    blueprint = pick_blueprint(world)
    blueprint.set_attribute("role_name", "tm_benchmark")

    vehicle = world.try_spawn_actor(blueprint, spawn)
    if vehicle is None:
        raise RuntimeError(f"spawn point {spawn_index} is blocked")

    recorder = RunRecorder(world_map, target_kmh, trace=trace)
    collision_bp = world.get_blueprint_library().find("sensor.other.collision")
    collision_sensor = world.spawn_actor(
        collision_bp, carla.Transform(), attach_to=vehicle)
    collision_sensor.listen(recorder.on_collision)

    agent = None
    destinations = None
    try:
        # One settling tick so the vehicle lands on its suspension before the
        # controller takes over.
        world.tick()
        if controller == "tm":
            # Re-seed per run: without this the TM RNG state depends on every
            # preceding run in the matrix, so a behavior change in one run
            # perturbs all later runs (breaks per-run A/B comparability).
            traffic_manager.set_random_device_seed(seed)
            vehicle.set_autopilot(True, tm_port)
            traffic_manager.set_desired_speed(vehicle, target_kmh)
            traffic_manager.ignore_lights_percentage(vehicle, 100.0)
            traffic_manager.ignore_signs_percentage(vehicle, 100.0)
        else:
            # Reference controller: the PythonAPI BasicAgent (pure-Python
            # planner + PID, no TM involvement). Same vehicle, same physics —
            # isolates the vehicle model from the TM stack.
            from agents.navigation.basic_agent import BasicAgent
            agent = BasicAgent(vehicle, target_speed=target_kmh, opt_dict={
                "ignore_traffic_lights": True,
                "ignore_stop_signs": True,
                "ignore_vehicles": True,
            })
            rng = random.Random(seed + spawn_index)
            spawn_points = world_map.get_spawn_points()
            destinations = lambda: rng.choice(spawn_points).location
            agent.set_destination(destinations())

        total_ticks = int(sim_seconds / FIXED_DELTA_SECONDS)
        for tick in range(total_ticks):
            if agent is not None:
                if agent.done():
                    agent.set_destination(destinations())
                vehicle.apply_control(agent.run_step())
            world.tick()
            recorder.on_tick(vehicle, tick * FIXED_DELTA_SECONDS)
            if verbose and tick % int(5.0 / FIXED_DELTA_SECONDS) == 0:
                dev = recorder.deviations[-1] if recorder.deviations else 0.0
                spd = recorder.speeds_ms[-1] * 3.6 if recorder.speeds_ms else 0.0
                print(f"    t={tick * FIXED_DELTA_SECONDS:5.1f}s "
                      f"speed={spd:6.1f} km/h dev={dev:5.2f} m", flush=True)
    finally:
        # Sensor first: destroying the parent under a live sensor crashes
        # the client.
        try:
            collision_sensor.stop()
            collision_sensor.destroy()
        except RuntimeError:
            pass
        try:
            if agent is None:
                vehicle.set_autopilot(False, tm_port)
            vehicle.destroy()
        except RuntimeError:
            pass
        world.tick()

    return recorder.summary(spawn_index, target_kmh)


def cmd_run(args):
    client = carla.Client(args.host, args.port)
    client.set_timeout(120.0)

    world = client.get_world()
    current_map = world.get_map().name.split("/")[-1]
    if current_map != args.map:
        print(f"Loading {args.map} (current: {current_map})...", flush=True)
        world = client.load_world(args.map)
        time.sleep(5.0)

    original_settings = world.get_settings()
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = FIXED_DELTA_SECONDS
    world.apply_settings(settings)

    traffic_manager = client.get_trafficmanager(args.tm_port)
    traffic_manager.set_synchronous_mode(True)
    traffic_manager.set_random_device_seed(args.seed)

    spawn_points = world.get_map().get_spawn_points()
    spawn_indices = (args.spawn_indices
                     if args.spawn_indices
                     else pick_spawn_indices(spawn_points, args.num_spawns))

    report = {
        "label": args.label,
        "map": args.map,
        "controller": args.controller,
        "seed": args.seed,
        "fixed_delta_seconds": FIXED_DELTA_SECONDS,
        "run_sim_seconds": args.duration,
        "speeds_kmh": args.speeds,
        "spawn_indices": spawn_indices,
        "thresholds": THRESHOLDS,
        "runs": [],
    }

    try:
        total = len(spawn_indices) * len(args.speeds)
        done = 0
        for spawn_index in spawn_indices:
            for target_kmh in args.speeds:
                done += 1
                print(f"[{done}/{total}] spawn {spawn_index} @ "
                      f"{target_kmh:.0f} km/h ...", flush=True)
                run = run_single(client, world, traffic_manager, args.tm_port,
                                 spawn_index, target_kmh, args.duration,
                                 args.verbose, controller=args.controller,
                                 trace=args.trace, seed=args.seed)
                report["runs"].append(run)
                status = "PASS" if run["pass"] else "FAIL"
                print(f"    {status}  dev_rms={run['lane_dev_rms_m']} m  "
                      f"dev_max={run['lane_dev_max_m']} m  "
                      f"offroad={run['offroad_pct']}%  "
                      f"speed={run['mean_speed_kmh']} km/h  "
                      f"collisions={run['collisions']}", flush=True)
                if run["failures"]:
                    for failure in run["failures"]:
                        print(f"      - {failure}", flush=True)
                # Checkpoint after every run so a crash keeps partial results.
                with open(args.out, "w") as handle:
                    json.dump(report, handle, indent=1)
    finally:
        traffic_manager.set_synchronous_mode(False)
        world.apply_settings(original_settings)

    passed = sum(1 for run in report["runs"] if run["pass"])
    report["passed_runs"] = passed
    report["total_runs"] = len(report["runs"])
    with open(args.out, "w") as handle:
        json.dump(report, handle, indent=1)

    print(f"\n{args.label}: {passed}/{len(report['runs'])} runs passed "
          f"-> {args.out}", flush=True)
    return 0 if passed == len(report["runs"]) else 1


# ---------------------------------------------------------------------------
# A/B comparison
# ---------------------------------------------------------------------------

COMPARE_METRICS = [
    # (key, lower_is_better)
    ("offroad_pct", True),
    ("lane_dev_rms_m", True),
    ("lane_dev_p95_m", True),
    ("lane_dev_max_m", True),
    ("collisions", True),
    ("max_stuck_seconds", True),
    ("speed_adherence", False),
]


def cmd_compare(args):
    with open(args.baseline) as handle:
        baseline = json.load(handle)
    with open(args.candidate) as handle:
        candidate = json.load(handle)

    def index(report):
        return {(run["spawn_index"], run["target_kmh"]): run
                for run in report["runs"]}

    base_runs = index(baseline)
    cand_runs = index(candidate)
    keys = sorted(set(base_runs) & set(cand_runs))
    if not keys:
        print("no overlapping (spawn, speed) runs to compare")
        return 2

    print(f"{'scenario':<22}{'metric':<20}{'base':>10}{'cand':>10}  verdict")
    print("-" * 74)
    better = worse = 0
    for key in keys:
        spawn_index, target_kmh = key
        scenario = f"spawn {spawn_index} @ {target_kmh:.0f}"
        for metric, lower_better in COMPARE_METRICS:
            base_value = base_runs[key].get(metric)
            cand_value = cand_runs[key].get(metric)
            if base_value is None or cand_value is None:
                continue
            delta = cand_value - base_value
            if abs(delta) < 1e-3:
                verdict = ""
            elif (delta < 0) == lower_better:
                verdict = "better"
                better += 1
            else:
                verdict = "WORSE"
                worse += 1
            if verdict or args.all:
                print(f"{scenario:<22}{metric:<20}"
                      f"{base_value:>10}{cand_value:>10}  {verdict}")

    base_pass = sum(1 for run in baseline["runs"] if run["pass"])
    cand_pass = sum(1 for run in candidate["runs"] if run["pass"])
    print("-" * 74)
    print(f"pass rate: {base_pass}/{len(baseline['runs'])} -> "
          f"{cand_pass}/{len(candidate['runs'])}   "
          f"metric deltas: {better} better, {worse} worse")
    return 0 if (cand_pass >= base_pass and worse == 0) else 1


# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)

    run_parser = sub.add_parser("run", help="execute the benchmark matrix")
    run_parser.add_argument("--host", default="127.0.0.1")
    run_parser.add_argument("--port", type=int, default=2000)
    run_parser.add_argument("--tm-port", type=int, default=8000)
    run_parser.add_argument("--map", default=DEFAULT_MAP)
    run_parser.add_argument("--speeds", type=lambda s: [float(x) for x in s.split(",")],
                            default=DEFAULT_SPEEDS_KMH,
                            help="comma-separated target speeds in km/h")
    run_parser.add_argument("--num-spawns", type=int, default=DEFAULT_NUM_SPAWNS)
    run_parser.add_argument("--spawn-indices", type=lambda s: [int(x) for x in s.split(",")],
                            default=None,
                            help="explicit spawn point indices (overrides --num-spawns)")
    run_parser.add_argument("--duration", type=float, default=DEFAULT_RUN_SIM_SECONDS,
                            help="simulated seconds per run")
    run_parser.add_argument("--seed", type=int, default=DEFAULT_SEED)
    run_parser.add_argument("--label", default="unnamed")
    run_parser.add_argument("--out", default="tm_benchmark_report.json")
    run_parser.add_argument("--controller", choices=["tm", "agent"], default="tm",
                            help="'tm' = Traffic Manager autopilot; 'agent' = "
                                 "PythonAPI BasicAgent reference controller")
    run_parser.add_argument("--trace", action="store_true",
                            help="store per-tick control/deviation traces in the report")
    run_parser.add_argument("--verbose", action="store_true")
    run_parser.set_defaults(func=cmd_run)

    compare_parser = sub.add_parser("compare", help="compare two reports")
    compare_parser.add_argument("baseline")
    compare_parser.add_argument("candidate")
    compare_parser.add_argument("--all", action="store_true",
                                help="print unchanged metrics too")
    compare_parser.set_defaults(func=cmd_compare)

    args = parser.parse_args()
    sys.exit(args.func(args))


if __name__ == "__main__":
    main()
