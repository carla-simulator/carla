# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Scenario taxonomy loader and variant sampler."""

from __future__ import annotations

import json
import os
import random
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional, Sequence, Tuple

from .scenario import ActorSpec, ScenarioParameters, ScenarioTemplate, SensorDegradation

DEFAULT_TAXONOMY_PATH = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "scenarios", "taxonomy.json")

Range = Tuple[float, float]


@dataclass
class ScenarioFamily:
    """One archetype from the taxonomy with parameter ranges."""

    id: str
    group: str
    name: str
    description: str
    tags: List[str]
    ranges: Dict[str, Range]
    weather_presets: List[str]
    times_of_day: List[str]
    actors: List[Dict[str, Any]] = field(default_factory=list)
    expected_events: List[str] = field(default_factory=list)
    variants: int = 5

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "ScenarioFamily":
        ranges = {k: (float(v[0]), float(v[1])) for k, v in data.get("ranges", {}).items()}
        return cls(
            id=data["id"],
            group=data["group"],
            name=data["name"],
            description=data.get("description", ""),
            tags=list(data.get("tags", [])),
            ranges=ranges,
            weather_presets=list(data.get("weather_presets", ["clear_day"])),
            times_of_day=list(data.get("times_of_day", ["day"])),
            actors=list(data.get("actors", [])),
            expected_events=list(data.get("expected_events", [])),
            variants=int(data.get("variants", 5)),
        )

    def _sample(self, rng: random.Random, key: str, default: float) -> float:
        lo, hi = self.ranges.get(key, (default, default))
        return round(rng.uniform(lo, hi), 4)

    def variant(self, index: int, base_seed: int = 0, vehicle_class: str = "truck") -> ScenarioTemplate:
        """Deterministically build variant ``index`` of this family.

        The same (family, index, base_seed) always yields the same template,
        which is what makes benchmark runs replayable.
        """
        seed = (base_seed * 1000003 + hash_str(self.id) * 7919 + index) % (2 ** 31)
        rng = random.Random(seed)
        # Cycle through weather and time combinations before sampling randomly,
        # so small variant counts still cover the family's declared conditions.
        combos = [(w, t) for w in self.weather_presets for t in self.times_of_day]
        weather_preset, time_of_day = combos[index % len(combos)]
        degradation = SensorDegradation(
            camera_blur=round(rng.uniform(0.0, 0.6) if index % 3 == 2 else 0.0, 3),
            gnss_noise_m=round(rng.choice([0.0, 0.5, 1.5, 3.0]), 3),
            imu_accel_noise=round(rng.choice([0.0, 0.05, 0.1]), 3),
            imu_gyro_noise=round(rng.choice([0.0, 0.002, 0.01]), 4),
            lidar_noise_m=round(rng.choice([0.0, 0.02, 0.05]), 3),
            dropout_probability=round(rng.choice([0.0, 0.0, 0.01, 0.05]), 3),
        )
        params = ScenarioParameters(
            weather_preset=weather_preset,
            time_of_day=time_of_day,
            traffic_density=self._sample(rng, "traffic_density", 0.2),
            lane_quality=self._sample(rng, "lane_quality", 0.7),
            road_curvature=self._sample(rng, "road_curvature", 0.0),
            grade_pct=self._sample(rng, "grade_pct", 0.0),
            speed_limit_kph=round(self._sample(rng, "speed_limit_kph", 40.0)),
            vehicle_class=vehicle_class,
            payload_kg=round(self._sample(rng, "payload_kg", 0.0)),
            lane_width_m=self._sample(rng, "lane_width_m", 3.3),
            duration_s=60.0,
            sensor_degradation=degradation,
        )
        actors = []
        for spec in self.actors:
            dist = spec.get("distance_m", [50.0, 50.0])
            lat = spec.get("lateral_m", [0.0, 0.0])
            actors.append(ActorSpec(
                role=spec["role"],
                lane=spec.get("lane", "ego"),
                distance_m=round(rng.uniform(float(dist[0]), float(dist[1])), 1),
                lateral_m=round(rng.uniform(float(lat[0]), float(lat[1])), 2),
                speed_mps=spec.get("speed_mps"),
            ))
        return ScenarioTemplate(
            id="bt-%s-%02d" % (self.id.replace("_", "-"), index + 1),
            family=self.id,
            group=self.group,
            name="%s (variant %d)" % (self.name, index + 1),
            description=self.description,
            tags=list(self.tags),
            params=params,
            actors=actors,
            expected_events=list(self.expected_events),
            seed=seed,
        )


def hash_str(value: str) -> int:
    """Stable string hash (Python's built-in hash is salted per process)."""
    acc = 0
    for ch in value:
        acc = (acc * 31 + ord(ch)) % (2 ** 31)
    return acc


@dataclass
class Taxonomy:
    version: str
    region: str
    description: str
    groups: Dict[str, str]
    families: List[ScenarioFamily]

    @classmethod
    def load(cls, path: Optional[str] = None) -> "Taxonomy":
        path = path or DEFAULT_TAXONOMY_PATH
        with open(path, "r", encoding="utf-8") as handle:
            data = json.load(handle)
        return cls.from_dict(data)

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "Taxonomy":
        families = [ScenarioFamily.from_dict(f) for f in data.get("families", [])]
        ids = [f.id for f in families]
        if len(ids) != len(set(ids)):
            raise ValueError("duplicate family ids in taxonomy")
        return cls(
            version=str(data.get("version", "0")),
            region=data.get("region", ""),
            description=data.get("description", ""),
            groups=dict(data.get("groups", {})),
            families=families,
        )

    def family(self, family_id: str) -> ScenarioFamily:
        for fam in self.families:
            if fam.id == family_id:
                return fam
        raise KeyError(family_id)

    def by_group(self, group: str) -> Sequence[ScenarioFamily]:
        return [f for f in self.families if f.group == group]
