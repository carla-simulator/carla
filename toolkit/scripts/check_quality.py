#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Run the data-quality gates over a run directory (simulated or real-world capture)."""

import argparse
import json
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.quality import assess_run, events_per_100km  # noqa: E402
from bhutan_sim.telemetry import load_run  # noqa: E402


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run-dir", required=True)
    parser.add_argument("--rate-hz", type=float, default=20.0, help="expected telemetry rate")
    parser.add_argument("--segment-s", type=float, default=30.0)
    parser.add_argument("--privacy-status", default=None, choices=["unprocessed", "redacted", "not_required"],
                        help="override the privacy status recorded in run.json")
    parser.add_argument("--out", default=None, help="write the report JSON here (default: <run-dir>/quality.json)")
    args = parser.parse_args()

    run = load_run(args.run_dir)
    manifest = run["manifest"]
    privacy = args.privacy_status or manifest.get("privacy_status") or ("not_required" if manifest.get("source") == "sim" else "unprocessed")
    report = assess_run(manifest["run_id"], run["samples"], run["events"], manifest.get("streams"),
                        expected_rate_hz=args.rate_hz, segment_len_s=args.segment_s, privacy_status=privacy)
    data = report.to_dict()
    data["events_per_100km"] = events_per_100km(run["events"], report.distance_km)
    out = args.out or os.path.join(args.run_dir, "quality.json")
    with open(out, "w", encoding="utf-8") as handle:
        json.dump(data, handle, indent=2, sort_keys=True)
    print("run %s: %d samples, %.1f s, %.2f km" % (manifest["run_id"], report.sample_count, report.duration_s, report.distance_km))
    print("segments accepted: %d/%d (%.0f%%)  replay_complete=%s  passed=%s" % (
        report.accepted_segments, len(report.segments), report.acceptance_rate * 100, report.replay_complete, report.passed))
    for issue in report.issues:
        print("  issue:", issue)
    for segment in report.segments:
        failed = [g.id for g in segment.gates if not g.passed]
        if failed:
            print("  segment %d failed: %s" % (segment.index, ", ".join(failed)))


if __name__ == "__main__":
    main()
