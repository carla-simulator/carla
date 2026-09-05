#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Profile a real GNSS trace into route archetypes and recommend scenario families to generate."""

import argparse
import json
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.route import load_trace, profile  # noqa: E402
from bhutan_sim.taxonomy import DEFAULT_TAXONOMY_PATH, Taxonomy  # noqa: E402


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--trace", required=True, help="CSV with lat,lon[,alt][,t] columns or JSONL")
    parser.add_argument("--taxonomy", default=DEFAULT_TAXONOMY_PATH)
    parser.add_argument("--out", default=None, help="write the profile JSON here")
    args = parser.parse_args()

    result = profile(load_trace(args.trace), Taxonomy.load(args.taxonomy))
    print("%d points, %.2f km, %d segments" % (result["points"], result["total_km"], len(result["segments"])))
    for cls, count in sorted(result["route_classes"].items(), key=lambda kv: -kv[1]):
        print("  %-24s %d" % (cls, count))
    print("recommended families:")
    for rec in result["recommended_families"][:10]:
        print("  %-26s %-14s %3d segments (%.0f%%)" % (rec["family"], rec["group"], rec["matching_segments"], rec["coverage"] * 100))
    if args.out:
        with open(args.out, "w", encoding="utf-8") as handle:
            json.dump(result, handle, indent=2)


if __name__ == "__main__":
    main()
