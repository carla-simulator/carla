#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Export scenario templates as ASAM OpenSCENARIO 1.2 (.xosc) for CARLA ScenarioRunner, esmini and partner simulators.

Examples:
  python scripts/export_scenario.py --scenario bt-valley-fog-01 --out-dir _out/xosc
  python scripts/export_scenario.py --family landslide_debris --map Town10HD_Opt
  python scripts/export_scenario.py --all
Then, with ScenarioRunner:
  python scenario_runner.py --openscenario _out/xosc/bt-valley-fog-01.xosc --openscenarioparams "road_id:12,ego_lane_id:-1,ego_s:35"
"""

import argparse
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.library import DEFAULT_LIBRARY_PATH, load_library  # noqa: E402
from bhutan_sim.openscenario import write_xosc  # noqa: E402


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--library", default=DEFAULT_LIBRARY_PATH)
    parser.add_argument("--scenario", action="append", default=[], help="template id (repeatable)")
    parser.add_argument("--family", action="append", default=[], help="export every template of a family (repeatable)")
    parser.add_argument("--all", action="store_true", help="export the whole library")
    parser.add_argument("--map", default="Town10HD_Opt", help="LogicFile name written into the RoadNetwork element")
    parser.add_argument("--out-dir", default="_out/xosc")
    args = parser.parse_args()
    if not (args.scenario or args.family or args.all):
        parser.error("choose --scenario, --family or --all")
    templates = load_library(args.library)
    selected = [t for t in templates if args.all or t.id in args.scenario or t.family in args.family]
    if not selected:
        sys.exit("no templates matched")
    os.makedirs(args.out_dir, exist_ok=True)
    for template in selected:
        path = write_xosc(template, os.path.join(args.out_dir, "%s.xosc" % template.id), map_name=args.map)
        print(path)
    print("%d OpenSCENARIO files written to %s" % (len(selected), args.out_dir))


if __name__ == "__main__":
    main()
