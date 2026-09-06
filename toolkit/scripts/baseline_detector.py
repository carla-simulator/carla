#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Synthetic baseline "model" used to validate the measurement pipeline.

It reads the ground truth recorded by run_scenario.py and produces detections
with configurable, condition-dependent degradation (range fall-off, fog and
night penalties, position noise, false positives). It is NOT a perception
model; it exists so the evaluation API, dashboards and reproducibility checks
can be exercised before a real model adapter is plugged in.
"""

import argparse
import json
import os
import random
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.library import DEFAULT_LIBRARY_PATH, load_library  # noqa: E402
from bhutan_sim.telemetry import read_jsonl  # noqa: E402


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--run-dir", required=True)
    parser.add_argument("--out", default=None, help="output detections JSONL (default: <run-dir>/detections-baseline.jsonl)")
    parser.add_argument("--base-recall", type=float, default=0.95)
    parser.add_argument("--range-halflife-m", type=float, default=45.0, help="range at which recall halves")
    parser.add_argument("--weather-penalty", type=float, default=0.25, help="recall multiplier loss in fog/heavy rain")
    parser.add_argument("--night-penalty", type=float, default=0.2)
    parser.add_argument("--position-noise-m", type=float, default=0.5)
    parser.add_argument("--false-positive-rate", type=float, default=0.05, help="expected false positives per frame")
    parser.add_argument("--seed", type=int, default=0)
    parser.add_argument("--library", default=DEFAULT_LIBRARY_PATH)
    args = parser.parse_args()

    scenario = {}
    manifest_path = os.path.join(args.run_dir, "run.json")
    if os.path.exists(manifest_path):
        with open(manifest_path, "r", encoding="utf-8") as handle:
            manifest = json.load(handle)
        for template in load_library(args.library):
            if template.id == manifest.get("scenario_id"):
                scenario = template.to_dict()
                break
    visibility = scenario.get("visibility_class", "clear")
    lighting = scenario.get("lighting_class", "daylight")
    penalty = 1.0
    if visibility in ("fog", "heavy_rain"):
        penalty -= args.weather_penalty
    if lighting == "night":
        penalty -= args.night_penalty

    rng = random.Random(args.seed)
    out_path = args.out or os.path.join(args.run_dir, "detections-baseline.jsonl")
    frames = 0
    with open(out_path, "w", encoding="utf-8") as out:
        for gt in read_jsonl(os.path.join(args.run_dir, "ground_truth.jsonl")):
            detections = []
            for obj in gt["objects"]:
                dist = (obj["x"] ** 2 + obj["y"] ** 2) ** 0.5
                recall = args.base_recall * penalty * (0.5 ** (dist / args.range_halflife_m))
                if rng.random() < recall:
                    detections.append({
                        "cls": obj["cls"],
                        "x": round(obj["x"] + rng.gauss(0, args.position_noise_m), 2),
                        "y": round(obj["y"] + rng.gauss(0, args.position_noise_m), 2),
                        "score": round(min(1.0, max(0.05, recall + rng.gauss(0, 0.1))), 3),
                    })
            if rng.random() < args.false_positive_rate:
                detections.append({"cls": rng.choice(["vehicle", "pedestrian", "obstacle"]),
                                   "x": round(rng.uniform(5, 50), 2), "y": round(rng.uniform(-4, 4), 2), "score": round(rng.uniform(0.3, 0.7), 3)})
            out.write(json.dumps({"frame": gt["frame"], "detections": detections}, separators=(",", ":")) + "\n")
            frames += 1
    print("wrote %d frames to %s (visibility=%s lighting=%s penalty=%.2f)" % (frames, out_path, visibility, lighting, penalty))


if __name__ == "__main__":
    main()
