#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Generate the Bhutan scenario library from the taxonomy and optionally push it to the dashboard."""

import argparse
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.library import DEFAULT_LIBRARY_PATH, write_library  # noqa: E402
from bhutan_sim.taxonomy import DEFAULT_TAXONOMY_PATH, Taxonomy  # noqa: E402
from bhutan_sim.telemetry import DashboardClient  # noqa: E402


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--taxonomy", default=DEFAULT_TAXONOMY_PATH, help="taxonomy JSON path")
    parser.add_argument("--out", default=DEFAULT_LIBRARY_PATH, help="output library JSON path")
    parser.add_argument("--seed", type=int, default=0, help="base seed for deterministic variants")
    parser.add_argument("--dashboard", default=os.environ.get("BHUTAN_DASHBOARD_URL"), help="dashboard base URL to upload to")
    parser.add_argument("--token", default=os.environ.get("BHUTAN_DASHBOARD_TOKEN"), help="dashboard API token")
    parser.add_argument("--tenant", default=os.environ.get("BHUTAN_TENANT", "default"))
    args = parser.parse_args()

    taxonomy = Taxonomy.load(args.taxonomy)
    manifest = write_library(args.out, taxonomy, base_seed=args.seed)
    print("wrote %d scenarios across %d families to %s" % (manifest["count"], len(manifest["families"]), args.out))
    if args.dashboard:
        if not args.token:
            parser.error("--token (or BHUTAN_DASHBOARD_TOKEN) is required with --dashboard")
        client = DashboardClient(args.dashboard, args.token, args.tenant)
        result = client.upload_scenarios(manifest)
        print("uploaded:", result)


if __name__ == "__main__":
    main()
