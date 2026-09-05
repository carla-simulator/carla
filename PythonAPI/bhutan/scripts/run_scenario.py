#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Run one or more Bhutan scenario templates in CARLA and record telemetry, events and ground truth.

Examples:
    python run_scenario.py --scenario bt-valley-fog-01
    python run_scenario.py --family landslide_debris --map Town10HD_Opt --save-camera
    python run_scenario.py --scenario bt-night-glare-02 --upload --dashboard https://atlas.example.workers.dev --token ...
"""

import argparse
import json
import os
import sys
import time

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.library import DEFAULT_LIBRARY_PATH, load_library  # noqa: E402
from bhutan_sim.telemetry import DashboardClient  # noqa: E402


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--host", default="localhost", help="IP of the host CARLA Simulator (default: localhost)")
    parser.add_argument("--port", type=int, default=2000, help="TCP port of CARLA Simulator (default: 2000)")
    parser.add_argument("--tm-port", type=int, default=8000, help="Traffic Manager port (default: 8000)")
    parser.add_argument("--library", default=DEFAULT_LIBRARY_PATH, help="scenario library JSON")
    parser.add_argument("--scenario", action="append", default=[], help="scenario id to run (repeatable)")
    parser.add_argument("--family", action="append", default=[], help="run every variant of a family (repeatable)")
    parser.add_argument("--map", default=None, help="load this map before running (default: keep current)")
    parser.add_argument("--out-dir", default="_out/bhutan_runs", help="root directory for run outputs")
    parser.add_argument("--fixed-delta", type=float, default=0.05, help="simulation step in seconds (default: 0.05)")
    parser.add_argument("--duration", type=float, default=None, help="override scenario duration in seconds")
    parser.add_argument("--seed", type=int, default=None, help="override the template seed")
    parser.add_argument("--save-camera", action="store_true", help="save forward RGB frames (slow)")
    parser.add_argument("--upload", action="store_true", help="upload each run to the dashboard")
    parser.add_argument("--dashboard", default=os.environ.get("BHUTAN_DASHBOARD_URL"))
    parser.add_argument("--token", default=os.environ.get("BHUTAN_DASHBOARD_TOKEN"))
    parser.add_argument("--tenant", default=os.environ.get("BHUTAN_TENANT", "default"))
    args = parser.parse_args()

    try:
        import carla
    except ImportError:
        parser.error("the carla Python package is not installed; build or pip install the CARLA client first")

    from bhutan_sim.runner import ScenarioRunner  # noqa: E402  (needs carla)

    templates = load_library(args.library)
    selected = [t for t in templates if t.id in set(args.scenario) or t.family in set(args.family)]
    if not selected:
        parser.error("no scenarios selected; use --scenario <id> or --family <family>")

    upload_client = None
    if args.upload:
        if not (args.dashboard and args.token):
            parser.error("--upload needs --dashboard and --token (or the BHUTAN_DASHBOARD_* environment variables)")
        upload_client = DashboardClient(args.dashboard, args.token, args.tenant)

    client = carla.Client(args.host, args.port)
    client.set_timeout(60.0)

    results = []
    for template in selected:
        if args.duration:
            template.params.duration_s = args.duration
        out_dir = os.path.join(args.out_dir, "%s-%d" % (template.id, int(time.time())))
        print("running %s (%s) -> %s" % (template.id, template.name, out_dir))
        runner = ScenarioRunner(client, template, out_dir, seed=args.seed, tm_port=args.tm_port,
                                fixed_delta=args.fixed_delta, save_camera=args.save_camera, map_name=args.map)
        result = runner.run(upload_client=upload_client)
        results.append(result)
        print(json.dumps({k: v for k, v in result.items() if k != "quality"}, indent=2))
        print("quality: acceptance=%.0f%% replay_complete=%s passed=%s" % (
            result["quality"]["acceptance_rate"] * 100, result["quality"]["replay_complete"], result["quality"]["passed"]))
    with open(os.path.join(args.out_dir, "batch-%d.json" % int(time.time())), "w", encoding="utf-8") as handle:
        json.dump(results, handle, indent=2)


if __name__ == "__main__":
    main()
