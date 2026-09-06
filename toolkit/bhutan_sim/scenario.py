# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Parameterized scenario templates.

A :class:`ScenarioTemplate` is the unit exchanged between the scenario library,
the CARLA runner and the dashboard. It is JSON serializable and versioned so
that every benchmark run can be replayed from the same inputs.
"""

from __future__ import annotations

import hashlib
import json
from dataclasses import asdict, dataclass, field
from typing import Any, Dict, List, Optional

from . import weather as weather_mod

SCHEMA_VERSION = "1.0"

VEHICLE_CLASSES = ("truck", "shuttle", "car")


@dataclass
class ActorSpec:
    """A non-ego actor placed relative to the ego vehicle."""

    role: str
    lane: str = "ego"
    distance_m: float = 50.0
    lateral_m: float = 0.0
    speed_mps: Optional[float] = None

    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "ActorSpec":
        return cls(
            role=data["role"],
            lane=data.get("lane", "ego"),
            distance_m=float(data.get("distance_m", 50.0)),
            lateral_m=float(data.get("lateral_m", 0.0)),
            speed_mps=data.get("speed_mps"),
        )


@dataclass
class SensorDegradation:
    """Sensor noise applied when the scenario runs in CARLA."""

    camera_blur: float = 0.0        # 0..1, mapped to motion blur / lens flare intensity
    gnss_noise_m: float = 0.0       # standard deviation in metres
    imu_accel_noise: float = 0.0    # standard deviation in m/s^2
    imu_gyro_noise: float = 0.0     # standard deviation in rad/s
    lidar_noise_m: float = 0.0      # per-point distance noise
    dropout_probability: float = 0.0  # probability that a telemetry sample is dropped

    def to_dict(self) -> Dict[str, Any]:
        return asdict(self)

    @classmethod
    def from_dict(cls, data: Optional[Dict[str, Any]]) -> "SensorDegradation":
        data = data or {}
        return cls(**{k: float(v) for k, v in data.items() if k in cls.__dataclass_fields__})


@dataclass
class ScenarioParameters:
    """Continuous and categorical knobs of a scenario variant."""

    weather_preset: str = "clear_day"
    time_of_day: str = "day"
    traffic_density: float = 0.2
    lane_quality: float = 0.7
    road_curvature: float = 0.0   # 1/m; 0.05 is a 20 m radius
    grade_pct: float = 0.0        # negative is downhill
    speed_limit_kph: float = 40.0
    vehicle_class: str = "truck"
    payload_kg: float = 0.0
    lane_width_m: float = 3.3
    duration_s: float = 60.0
    sensor_degradation: SensorDegradation = field(default_factory=SensorDegradation)

    def to_dict(self) -> Dict[str, Any]:
        data = asdict(self)
        data["sensor_degradation"] = self.sensor_degradation.to_dict()
        return data

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "ScenarioParameters":
        kwargs = {k: v for k, v in data.items() if k in cls.__dataclass_fields__ and k != "sensor_degradation"}
        kwargs["sensor_degradation"] = SensorDegradation.from_dict(data.get("sensor_degradation"))
        return cls(**kwargs)

    def resolved_weather(self) -> Dict[str, float]:
        return weather_mod.resolve(self.weather_preset, self.time_of_day)

    @property
    def lighting_class(self) -> str:
        return weather_mod.LIGHTING_CLASS[self.time_of_day]

    @property
    def visibility_class(self) -> str:
        return weather_mod.visibility_class(self.resolved_weather())

    @property
    def route_class(self) -> str:
        """Coarse route class combining curvature and grade."""
        if self.road_curvature >= 0.08:
            geometry = "hairpin"
        elif self.road_curvature >= 0.03:
            geometry = "curve"
        else:
            geometry = "straight"
        if self.grade_pct <= -6:
            slope = "steep_descent"
        elif self.grade_pct >= 6:
            slope = "steep_climb"
        else:
            slope = "flat"
        return "%s_%s" % (geometry, slope)


@dataclass
class ScenarioTemplate:
    """A versioned, replayable scenario definition."""

    id: str
    family: str
    group: str
    name: str
    description: str
    tags: List[str]
    params: ScenarioParameters
    actors: List[ActorSpec] = field(default_factory=list)
    expected_events: List[str] = field(default_factory=list)
    seed: int = 0
    version: str = "1"
    schema_version: str = SCHEMA_VERSION
    review_status: str = "unreviewed"   # unreviewed | reviewed | rejected

    def to_dict(self) -> Dict[str, Any]:
        return {
            "id": self.id,
            "family": self.family,
            "group": self.group,
            "name": self.name,
            "description": self.description,
            "tags": list(self.tags),
            "params": self.params.to_dict(),
            "actors": [a.to_dict() for a in self.actors],
            "expected_events": list(self.expected_events),
            "seed": self.seed,
            "version": self.version,
            "schema_version": self.schema_version,
            "review_status": self.review_status,
            "route_class": self.params.route_class,
            "lighting_class": self.params.lighting_class,
            "visibility_class": self.params.visibility_class,
            "content_hash": self.content_hash(),
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "ScenarioTemplate":
        return cls(
            id=data["id"],
            family=data["family"],
            group=data.get("group", ""),
            name=data.get("name", data["id"]),
            description=data.get("description", ""),
            tags=list(data.get("tags", [])),
            params=ScenarioParameters.from_dict(data.get("params", {})),
            actors=[ActorSpec.from_dict(a) for a in data.get("actors", [])],
            expected_events=list(data.get("expected_events", [])),
            seed=int(data.get("seed", 0)),
            version=str(data.get("version", "1")),
            schema_version=str(data.get("schema_version", SCHEMA_VERSION)),
            review_status=data.get("review_status", "unreviewed"),
        )

    def content_hash(self) -> str:
        """Stable hash of the replay-relevant content (excludes review status)."""
        payload = {
            "id": self.id,
            "family": self.family,
            "params": self.params.to_dict(),
            "actors": [a.to_dict() for a in self.actors],
            "seed": self.seed,
            "version": self.version,
        }
        encoded = json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("utf-8")
        return "sha256:" + hashlib.sha256(encoded).hexdigest()

    def to_json(self, indent: Optional[int] = 2) -> str:
        return json.dumps(self.to_dict(), indent=indent, sort_keys=True)
