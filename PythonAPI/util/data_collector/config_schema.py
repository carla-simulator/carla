# Copyright (c) 2021 Intel Labs.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from dataclasses import dataclass, field
from typing import Optional
from omegaconf import MISSING
from typing import List, Dict
import sys

@dataclass
class Sync:
    fps: float = 0.2
    timeout: float = 2.0

@dataclass
class Carla:
    host: str = "127.0.0.1"
    port: int = 2000
    timeout: float = 5.0
    sync: Sync = Sync()
    seed: int = 42
    traffic_manager_port: int = 8000
    respawn: bool = True
    townmap: Optional[str] = MISSING

@dataclass
class Location:
    x: float = 0.0
    y: float = 0.0
    z: float = 0.0

@dataclass
class Rotation:
    roll: float = 0.0
    pitch: float = 0.0
    yaw: float = 0.0

@dataclass
class Weather:
    cloudiness: float = 0.0
    precipitation: float = 0.0
    precipitation_deposits: float = 0.0
    wind_intensity: float = 0.0
    sun_azimuth_angle: float = 0.0
    sun_altitude_angle: float = 0.0
    fog_density: float = 0.0
    fog_distance: float = 0.0
    wetness: float = 0.0

@dataclass
class Transform:
    location: Location = Location()
    rotation: Rotation = Rotation()

@dataclass
class Blueprint:
    name: str = MISSING
    attr: Optional[Dict[str, str]] = field(default_factory=lambda: {"image_size_x": 800, "image_size_y": 600, "gamma": 2.2})

@dataclass
class Actor:
    blueprint: Blueprint = Blueprint()
    transform: Transform = Transform()
    attach_to: Optional[str] = MISSING
    attachment: str = "rigid"

@dataclass
class SpawnActors(Actor):
    sensors: Optional[List[Actor]] = MISSING

@dataclass
class ConfigSchema:
    carla: Carla = Carla()
    weather: Weather = Weather()
    output_dir: str = "_out"
    max_frames: int = sys.maxsize
    spawn_actors: List[SpawnActors] = MISSING
