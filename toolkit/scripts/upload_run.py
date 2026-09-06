#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Upload a run directory (telemetry, events, quality report) to the Cloudflare Workers dashboard."""

import argparse
import json
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.telemetry import DashboardClient  # noqa: E402


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run-dir", required=True)
    parser.add_argument("--dashboard", default=os.environ.get("BHUTAN_DASHBOARD_URL"), required=not os.environ.get("BHUTAN_DASHBOARD_URL"))
    parser.add_argument("--token", default=os.environ.get("BHUTAN_DASHBOARD_TOKEN"), required=not os.environ.get("BHUTAN_DASHBOARD_TOKEN"))
    parser.add_argument("--tenant", default=os.environ.get("BHUTAN_TENANT", "default"))
    args = parser.parse_args()

    quality = None
    quality_path = os.path.join(args.run_dir, "quality.json")
    if os.path.exists(quality_path):
        with open(quality_path, "r", encoding="utf-8") as handle:
            quality = json.load(handle)
    client = DashboardClient(args.dashboard, args.token, args.tenant)
    print(json.dumps(client.upload_run_dir(args.run_dir, quality), indent=2))


if __name__ == "__main__":
    main()
