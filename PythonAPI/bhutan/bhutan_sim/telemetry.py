# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Unified timeline records and the dashboard upload client.

The same record shapes are used for simulated runs (CARLA) and passive
real-world capture (camera, GNSS/IMU, read-only CAN/J1939). The dashboard's
D1 schema and the deck.gl playback consume exactly these fields, so keep the
two in sync when adding columns (see Dashboard/schema.sql).
"""

from __future__ import annotations

import hashlib
import json
import os
import time
import urllib.error
import urllib.request
from dataclasses import asdict, dataclass, field
from typing import Any, Dict, Iterable, Iterator, List, Optional

SCHEMA_VERSION = "1.0"

# Event classes. Real-world event labels and simulated events share this list
# so edge-case discovery rates are comparable across sources.
EVENT_CLASSES = (
    "collision",
    "lane_departure",
    "hard_brake",
    "lateral_accel_high",
    "downhill_overspeed",
    "speeding",
    "ttc_low",
    "obstacle_ahead",
    "stopped_vehicle_ahead",
    "lead_vehicle_close",
    "oncoming_encounter",
    "oncoming_encroachment",
    "vru_close_pass",
    "vru_crossing",
    "overtaken_close",
    "sight_distance_limited",
    "driver_intervention",
    "sensor_dropout",
    "manual_flag",
)

SEVERITIES = ("info", "warning", "critical")


@dataclass
class Sample:
    """One row of the synchronized timeline."""

    t: float                     # unix seconds
    frame: int
    lat: float
    lon: float
    alt: float = 0.0
    speed_mps: float = 0.0
    heading_deg: float = 0.0
    accel_x: float = 0.0         # longitudinal, m/s^2 (positive forward)
    accel_y: float = 0.0         # lateral, m/s^2
    accel_z: float = 0.0
    yaw_rate: float = 0.0        # rad/s
    throttle: float = 0.0
    brake: float = 0.0
    steer: float = 0.0
    grade_pct: float = 0.0
    lead_distance_m: Optional[float] = None
    lead_rel_speed_mps: Optional[float] = None
    source: str = "sim"          # sim | vehicle
    x: Optional[float] = None    # simulator-local coordinates when available
    y: Optional[float] = None

    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)


@dataclass
class Event:
    t: float
    frame: int
    event_class: str
    severity: str = "info"
    lat: Optional[float] = None
    lon: Optional[float] = None
    description: str = ""
    rule_id: Optional[str] = None
    data: Dict[str, Any] = field(default_factory=dict)
    review_status: str = "unreviewed"   # unreviewed | classified | reviewed

    def __post_init__(self) -> None:
        if self.event_class not in EVENT_CLASSES:
            raise ValueError("unknown event class '%s'" % self.event_class)
        if self.severity not in SEVERITIES:
            raise ValueError("unknown severity '%s'" % self.severity)

    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)


@dataclass
class RunManifest:
    run_id: str
    source: str                  # sim | vehicle
    vehicle_class: str
    route_id: str
    scenario_id: Optional[str] = None
    scenario_hash: Optional[str] = None
    started_at: float = 0.0
    ended_at: float = 0.0
    map_name: str = ""
    odd_zone: str = ""
    streams: Dict[str, bool] = field(default_factory=lambda: {"video": False, "gnss": False, "imu": False, "can": False, "events": False})
    consent_ref: Optional[str] = None
    notes: str = ""
    schema_version: str = SCHEMA_VERSION

    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)


def sha256_file(path: str) -> str:
    digest = hashlib.sha256()
    with open(path, "rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return "sha256:" + digest.hexdigest()


class TelemetryLogger:
    """Append-only JSONL writer for one run directory."""

    def __init__(self, run_dir: str, manifest: RunManifest):
        self.run_dir = run_dir
        self.manifest = manifest
        os.makedirs(run_dir, exist_ok=True)
        self._samples = open(os.path.join(run_dir, "telemetry.jsonl"), "a", encoding="utf-8")
        self._events = open(os.path.join(run_dir, "events.jsonl"), "a", encoding="utf-8")
        self._ground_truth = open(os.path.join(run_dir, "ground_truth.jsonl"), "a", encoding="utf-8")
        self.sample_count = 0
        self.event_count = 0

    def sample(self, sample: Sample) -> None:
        self._samples.write(json.dumps(sample.to_dict(), separators=(",", ":")) + "\n")
        self.sample_count += 1

    def event(self, event: Event) -> None:
        self._events.write(json.dumps(event.to_dict(), separators=(",", ":")) + "\n")
        self.event_count += 1
        self.manifest.streams["events"] = True

    def ground_truth(self, frame: Dict[str, Any]) -> None:
        self._ground_truth.write(json.dumps(frame, separators=(",", ":")) + "\n")

    def close(self) -> None:
        for handle in (self._samples, self._events, self._ground_truth):
            handle.flush()
            handle.close()
        if not self.manifest.ended_at:
            self.manifest.ended_at = time.time()
        manifest = self.manifest.to_dict()
        manifest["files"] = {
            name: sha256_file(os.path.join(self.run_dir, name))
            for name in ("telemetry.jsonl", "events.jsonl", "ground_truth.jsonl")
        }
        manifest["sample_count"] = self.sample_count
        manifest["event_count"] = self.event_count
        with open(os.path.join(self.run_dir, "run.json"), "w", encoding="utf-8") as handle:
            json.dump(manifest, handle, indent=2, sort_keys=True)


def read_jsonl(path: str) -> Iterator[Dict[str, Any]]:
    with open(path, "r", encoding="utf-8") as handle:
        for line in handle:
            line = line.strip()
            if line:
                yield json.loads(line)


def load_run(run_dir: str) -> Dict[str, Any]:
    """Load manifest, samples and events from a run directory."""
    with open(os.path.join(run_dir, "run.json"), "r", encoding="utf-8") as handle:
        manifest = json.load(handle)
    samples = list(read_jsonl(os.path.join(run_dir, "telemetry.jsonl")))
    events_path = os.path.join(run_dir, "events.jsonl")
    events = list(read_jsonl(events_path)) if os.path.exists(events_path) else []
    return {"manifest": manifest, "samples": samples, "events": events}


def chunked(items: Iterable[Any], size: int) -> Iterator[List[Any]]:
    batch: List[Any] = []
    for item in items:
        batch.append(item)
        if len(batch) >= size:
            yield batch
            batch = []
    if batch:
        yield batch


class DashboardError(RuntimeError):
    pass


class DashboardClient:
    """Minimal HTTP client for the Cloudflare Workers dashboard API.

    Uses only the standard library so field laptops and edge loggers do not
    need extra packages. Every write is idempotent on the server side when a
    client-generated id is supplied.
    """

    def __init__(self, base_url: str, token: str, tenant: str = "default", timeout: float = 30.0):
        self.base_url = base_url.rstrip("/")
        self.token = token
        self.tenant = tenant
        self.timeout = timeout

    def _request(self, method: str, path: str, body: Optional[Any] = None, raw: Optional[bytes] = None,
                 content_type: str = "application/json") -> Any:
        url = self.base_url + path
        data = raw if raw is not None else (json.dumps(body).encode("utf-8") if body is not None else None)
        request = urllib.request.Request(url, data=data, method=method)
        request.add_header("Authorization", "Bearer " + self.token)
        request.add_header("X-Tenant", self.tenant)
        if data is not None:
            request.add_header("Content-Type", content_type)
        try:
            with urllib.request.urlopen(request, timeout=self.timeout) as response:
                payload = response.read()
        except urllib.error.HTTPError as exc:
            detail = exc.read().decode("utf-8", "replace")
            raise DashboardError("%s %s failed with %d: %s" % (method, path, exc.code, detail)) from exc
        except urllib.error.URLError as exc:
            raise DashboardError("%s %s failed: %s" % (method, path, exc.reason)) from exc
        if not payload:
            return None
        return json.loads(payload.decode("utf-8"))

    def health(self) -> Any:
        return self._request("GET", "/api/health")

    def create_run(self, manifest: RunManifest) -> Any:
        return self._request("POST", "/api/runs", manifest.to_dict())

    def upload_telemetry(self, run_id: str, samples: Iterable[Dict[str, Any]], chunk_size: int = 2000) -> List[Any]:
        results = []
        for seq, batch in enumerate(chunked(samples, chunk_size)):
            results.append(self._request("POST", "/api/runs/%s/telemetry?seq=%d" % (run_id, seq), {"samples": batch}))
        return results

    def upload_events(self, run_id: str, events: Iterable[Dict[str, Any]]) -> Any:
        return self._request("POST", "/api/runs/%s/events" % run_id, {"events": list(events)})

    def finish_run(self, run_id: str, quality: Optional[Dict[str, Any]] = None) -> Any:
        return self._request("POST", "/api/runs/%s/finish" % run_id, {"quality": quality or {}})

    def upload_scenarios(self, manifest: Dict[str, Any]) -> Any:
        return self._request("POST", "/api/scenarios/import", manifest)

    def upload_evaluation(self, report: Dict[str, Any]) -> Any:
        return self._request("POST", "/api/evaluations", report)

    def register_clip(self, clip: Dict[str, Any]) -> Any:
        return self._request("POST", "/api/clips", clip)

    def upload_clip_bytes(self, clip_id: str, path: str, content_type: str = "video/mp4") -> Any:
        with open(path, "rb") as handle:
            raw = handle.read()
        return self._request("PUT", "/api/clips/%s/object" % clip_id, raw=raw, content_type=content_type)

    def upload_run_dir(self, run_dir: str, quality: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """Upload a complete run directory produced by :class:`TelemetryLogger`."""
        run = load_run(run_dir)
        manifest = run["manifest"]
        run_manifest = RunManifest(**{k: v for k, v in manifest.items() if k in RunManifest.__dataclass_fields__})
        created = self.create_run(run_manifest)
        self.upload_telemetry(run_manifest.run_id, run["samples"])
        if run["events"]:
            self.upload_events(run_manifest.run_id, run["events"])
        finished = self.finish_run(run_manifest.run_id, quality)
        return {"created": created, "finished": finished}
