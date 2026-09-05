#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Benchmark a model's detections against a run's ground truth and optionally upload the report."""

import argparse
import json
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.evaluation import evaluate_perception  # noqa: E402
from bhutan_sim.library import DEFAULT_LIBRARY_PATH, load_library  # noqa: E402
from bhutan_sim.telemetry import DashboardClient, read_jsonl  # noqa: E402


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run-dir", required=True, help="run directory produced by run_scenario.py")
    parser.add_argument("--detections", required=True, help="detections JSONL from the model adapter")
    parser.add_argument("--model-id", required=True)
    parser.add_argument("--model-version", default="0")
    parser.add_argument("--library", default=DEFAULT_LIBRARY_PATH)
    parser.add_argument("--match-distance", type=float, default=2.0)
    parser.add_argument("--score-threshold", type=float, default=0.3)
    parser.add_argument("--out", default=None, help="report JSON path (default: <run-dir>/evaluation-<model>.json)")
    parser.add_argument("--upload", action="store_true")
    parser.add_argument("--dashboard", default=os.environ.get("BHUTAN_DASHBOARD_URL"))
    parser.add_argument("--token", default=os.environ.get("BHUTAN_DASHBOARD_TOKEN"))
    parser.add_argument("--tenant", default=os.environ.get("BHUTAN_TENANT", "default"))
    args = parser.parse_args()

    with open(os.path.join(args.run_dir, "run.json"), "r", encoding="utf-8") as handle:
        manifest = json.load(handle)
    scenario = None
    if manifest.get("scenario_id"):
        for template in load_library(args.library):
            if template.id == manifest["scenario_id"]:
                scenario = template.to_dict()
                break
    report = evaluate_perception(
        read_jsonl(os.path.join(args.run_dir, "ground_truth.jsonl")),
        read_jsonl(args.detections),
        model_id=args.model_id,
        model_version=args.model_version,
        run_id=manifest["run_id"],
        scenario=scenario,
        match_distance_m=args.match_distance,
        score_threshold=args.score_threshold,
    )
    data = report.to_dict()
    out = args.out or os.path.join(args.run_dir, "evaluation-%s.json" % args.model_id)
    with open(out, "w", encoding="utf-8") as handle:
        json.dump(data, handle, indent=2, sort_keys=True)
    overall = data["overall"]
    print("%s: precision=%.3f recall=%.3f f1=%.3f over %d frames (conditions=%s)" % (
        data["evaluation_id"], overall["precision"], overall["recall"], overall["f1"], data["frames_evaluated"], data["conditions"]))
    for cluster in data["failure_clusters"][:5]:
        print("  missed %3d  %-11s %-10s %-9s %s" % (cluster["missed"], cluster["cls"], cluster["weather"], cluster["lighting"], cluster["range_band"]))
    if args.upload:
        if not (args.dashboard and args.token):
            parser.error("--upload needs --dashboard and --token")
        client = DashboardClient(args.dashboard, args.token, args.tenant)
        print("uploaded:", client.upload_evaluation(data))


if __name__ == "__main__":
    main()
