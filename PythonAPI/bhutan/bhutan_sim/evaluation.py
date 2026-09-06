# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Perception benchmark and failure clustering.

Ground truth comes from CARLA actor bounding boxes (see ``runner``) or from
human labels on real clips. Detections come from a model adapter: a JSONL file
with one frame per line ``{"frame": int, "detections": [{"cls": str, "x": m,
"y": m, "score": float}]}`` in the ego bird's-eye frame. Matching is done on
bird's-eye centre distance, which is what a planner consumes and avoids
camera-projection ambiguity across sensor rigs.
"""

from __future__ import annotations

import hashlib
import json
import time
from dataclasses import dataclass, field
from typing import Any, Dict, Iterable, List, Optional, Sequence, Tuple

CLASS_ALIASES = {
    "car": "vehicle",
    "truck": "vehicle",
    "bus": "vehicle",
    "van": "vehicle",
    "vehicle": "vehicle",
    "motorcycle": "motorcycle",
    "motorbike": "motorcycle",
    "bicycle": "bicycle",
    "pedestrian": "pedestrian",
    "person": "pedestrian",
    "walker": "pedestrian",
    "livestock": "animal",
    "animal": "animal",
    "debris": "obstacle",
    "cone": "obstacle",
    "barrier": "obstacle",
    "obstacle": "obstacle",
}


def canonical_class(name: str) -> str:
    return CLASS_ALIASES.get(str(name).lower(), str(name).lower())


@dataclass
class Counts:
    tp: int = 0
    fp: int = 0
    fn: int = 0

    @property
    def precision(self) -> float:
        return self.tp / (self.tp + self.fp) if (self.tp + self.fp) else 0.0

    @property
    def recall(self) -> float:
        return self.tp / (self.tp + self.fn) if (self.tp + self.fn) else 0.0

    @property
    def f1(self) -> float:
        p, r = self.precision, self.recall
        return 2 * p * r / (p + r) if (p + r) else 0.0

    def to_dict(self) -> Dict[str, Any]:
        return {"tp": self.tp, "fp": self.fp, "fn": self.fn,
                "precision": round(self.precision, 4), "recall": round(self.recall, 4), "f1": round(self.f1, 4)}


def _match_frame(gt: Sequence[Dict[str, Any]], det: Sequence[Dict[str, Any]], max_dist_m: float,
                 score_threshold: float) -> Tuple[List[Tuple[int, int]], List[int], List[int]]:
    """Greedy nearest-neighbour matching within a class."""
    det = [d for d in det if float(d.get("score", 1.0)) >= score_threshold]
    pairs: List[Tuple[int, int, float]] = []
    for gi, g in enumerate(gt):
        for di, d in enumerate(det):
            if canonical_class(g["cls"]) != canonical_class(d["cls"]):
                continue
            dist = ((float(g["x"]) - float(d["x"])) ** 2 + (float(g["y"]) - float(d["y"])) ** 2) ** 0.5
            if dist <= max_dist_m:
                pairs.append((gi, di, dist))
    pairs.sort(key=lambda p: p[2])
    used_g, used_d, matched = set(), set(), []
    for gi, di, _ in pairs:
        if gi in used_g or di in used_d:
            continue
        used_g.add(gi)
        used_d.add(di)
        matched.append((gi, di))
    unmatched_g = [i for i in range(len(gt)) if i not in used_g]
    unmatched_d = [i for i in range(len(det)) if i not in used_d]
    return matched, unmatched_g, unmatched_d


def sha256_of(obj: Any) -> str:
    encoded = json.dumps(obj, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return "sha256:" + hashlib.sha256(encoded).hexdigest()


@dataclass
class EvaluationReport:
    evaluation_id: str
    model_id: str
    model_version: str
    run_id: str
    scenario_id: Optional[str]
    conditions: Dict[str, str]
    overall: Counts
    by_class: Dict[str, Counts]
    by_condition: Dict[str, Dict[str, Counts]]   # dimension -> value -> counts
    failure_clusters: List[Dict[str, Any]]
    frames_evaluated: int
    match_distance_m: float
    score_threshold: float
    inputs: Dict[str, str] = field(default_factory=dict)
    created_at: float = field(default_factory=time.time)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "evaluation_id": self.evaluation_id,
            "model_id": self.model_id,
            "model_version": self.model_version,
            "run_id": self.run_id,
            "scenario_id": self.scenario_id,
            "conditions": dict(self.conditions),
            "overall": self.overall.to_dict(),
            "by_class": {k: v.to_dict() for k, v in self.by_class.items()},
            "by_condition": {dim: {val: c.to_dict() for val, c in vals.items()} for dim, vals in self.by_condition.items()},
            "failure_clusters": list(self.failure_clusters),
            "frames_evaluated": self.frames_evaluated,
            "match_distance_m": self.match_distance_m,
            "score_threshold": self.score_threshold,
            "inputs": dict(self.inputs),
            "created_at": self.created_at,
            "reproducible": bool(self.inputs.get("ground_truth") and self.inputs.get("detections") and self.inputs.get("scenario")),
        }


def evaluate_perception(ground_truth: Iterable[Dict[str, Any]], detections: Iterable[Dict[str, Any]],
                        model_id: str, model_version: str, run_id: str, scenario: Optional[Dict[str, Any]] = None,
                        conditions: Optional[Dict[str, str]] = None, match_distance_m: float = 2.0,
                        score_threshold: float = 0.3, max_range_m: float = 60.0) -> EvaluationReport:
    """Compute precision/recall by class and by condition, plus failure clusters."""
    gt_frames = {int(f["frame"]): f for f in ground_truth}
    det_frames = {int(f["frame"]): f for f in detections}
    conditions = dict(conditions or {})
    if scenario:
        conditions.setdefault("weather", scenario.get("visibility_class") or scenario.get("params", {}).get("weather_preset", "unknown"))
        conditions.setdefault("lighting", scenario.get("lighting_class", "unknown"))
        conditions.setdefault("route_class", scenario.get("route_class", "unknown"))
    for key in ("weather", "lighting", "route_class"):
        conditions.setdefault(key, "unknown")

    overall = Counts()
    by_class: Dict[str, Counts] = {}
    by_condition: Dict[str, Dict[str, Counts]] = {dim: {val: Counts()} for dim, val in conditions.items()}
    misses: Dict[Tuple[str, str, str, str], int] = {}

    for frame_id, gt_frame in gt_frames.items():
        gt_objects = [o for o in gt_frame.get("objects", []) if (float(o["x"]) ** 2 + float(o["y"]) ** 2) ** 0.5 <= max_range_m]
        det_objects = det_frames.get(frame_id, {}).get("detections", [])
        matched, un_g, un_d = _match_frame(gt_objects, det_objects, match_distance_m, score_threshold)

        def bump(cls: str, attr: str, n: int = 1) -> None:
            setattr(overall, attr, getattr(overall, attr) + n)
            c = by_class.setdefault(cls, Counts())
            setattr(c, attr, getattr(c, attr) + n)
            for dim, val in conditions.items():
                cc = by_condition[dim][val]
                setattr(cc, attr, getattr(cc, attr) + n)

        for gi, _ in matched:
            bump(canonical_class(gt_objects[gi]["cls"]), "tp")
        for gi in un_g:
            cls = canonical_class(gt_objects[gi]["cls"])
            bump(cls, "fn")
            dist = (float(gt_objects[gi]["x"]) ** 2 + float(gt_objects[gi]["y"]) ** 2) ** 0.5
            band = "near" if dist < 20 else "mid" if dist < 40 else "far"
            key = (cls, conditions["weather"], conditions["lighting"], band)
            misses[key] = misses.get(key, 0) + 1
        det_scored = [d for d in det_objects if float(d.get("score", 1.0)) >= score_threshold]
        for di in un_d:
            bump(canonical_class(det_scored[di]["cls"]), "fp")

    clusters = [
        {"cls": k[0], "weather": k[1], "lighting": k[2], "range_band": k[3], "missed": v}
        for k, v in sorted(misses.items(), key=lambda kv: -kv[1])
    ]
    inputs = {
        "ground_truth": sha256_of(sorted(gt_frames.items())),
        "detections": sha256_of(sorted(det_frames.items())),
        "scenario": (scenario or {}).get("content_hash") or (sha256_of(scenario) if scenario else ""),
        "parameters": sha256_of({"match_distance_m": match_distance_m, "score_threshold": score_threshold, "max_range_m": max_range_m}),
    }
    evaluation_id = "ev-" + hashlib.sha256((model_id + model_version + run_id + inputs["ground_truth"] + inputs["detections"]).encode()).hexdigest()[:16]
    return EvaluationReport(
        evaluation_id=evaluation_id,
        model_id=model_id,
        model_version=model_version,
        run_id=run_id,
        scenario_id=(scenario or {}).get("id"),
        conditions=conditions,
        overall=overall,
        by_class=by_class,
        by_condition=by_condition,
        failure_clusters=clusters,
        frames_evaluated=len(gt_frames),
        match_distance_m=match_distance_m,
        score_threshold=score_threshold,
        inputs=inputs,
    )


def merge_reports(reports: Sequence[EvaluationReport]) -> Dict[str, Any]:
    """Aggregate several per-run reports into one benchmark table."""
    by_class: Dict[str, Counts] = {}
    by_condition: Dict[str, Dict[str, Counts]] = {}
    overall = Counts()
    for r in reports:
        for attr in ("tp", "fp", "fn"):
            setattr(overall, attr, getattr(overall, attr) + getattr(r.overall, attr))
        for cls, c in r.by_class.items():
            agg = by_class.setdefault(cls, Counts())
            for attr in ("tp", "fp", "fn"):
                setattr(agg, attr, getattr(agg, attr) + getattr(c, attr))
        for dim, vals in r.by_condition.items():
            for val, c in vals.items():
                agg = by_condition.setdefault(dim, {}).setdefault(val, Counts())
                for attr in ("tp", "fp", "fn"):
                    setattr(agg, attr, getattr(agg, attr) + getattr(c, attr))
    return {
        "reports": len(reports),
        "overall": overall.to_dict(),
        "by_class": {k: v.to_dict() for k, v in by_class.items()},
        "by_condition": {dim: {val: c.to_dict() for val, c in vals.items()} for dim, vals in by_condition.items()},
    }
