# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Bhutan weather and lighting presets.

Presets are plain dictionaries with ``carla.WeatherParameters`` field names so
they can be serialized into scenario templates and applied later with
:func:`to_carla`. Sun altitude is derived from ``time_of_day`` so a preset and a
time can be combined independently.
"""

from __future__ import annotations

import copy
from typing import Dict, Optional

# Field names exactly as exposed by carla.WeatherParameters on the UE5 branch.
WEATHER_FIELDS = (
    "cloudiness",
    "precipitation",
    "precipitation_deposits",
    "wind_intensity",
    "sun_azimuth_angle",
    "sun_altitude_angle",
    "fog_density",
    "fog_distance",
    "fog_falloff",
    "wetness",
    "scattering_intensity",
    "mie_scattering_scale",
    "rayleigh_scattering_scale",
    "dust_storm",
)

_BASE = {
    "cloudiness": 10.0,
    "precipitation": 0.0,
    "precipitation_deposits": 0.0,
    "wind_intensity": 5.0,
    "sun_azimuth_angle": 120.0,
    "sun_altitude_angle": 60.0,
    "fog_density": 0.0,
    "fog_distance": 0.0,
    "fog_falloff": 0.2,
    "wetness": 0.0,
    "scattering_intensity": 1.0,
    "mie_scattering_scale": 0.03,
    "rayleigh_scattering_scale": 0.0331,
    "dust_storm": 0.0,
}

PRESETS: Dict[str, Dict[str, float]] = {
    "clear_day": dict(_BASE),
    "overcast": dict(_BASE, cloudiness=85.0, wind_intensity=15.0),
    "light_rain": dict(_BASE, cloudiness=80.0, precipitation=30.0, precipitation_deposits=25.0, wetness=40.0, wind_intensity=20.0),
    "heavy_rain": dict(_BASE, cloudiness=95.0, precipitation=80.0, precipitation_deposits=70.0, wetness=85.0, wind_intensity=40.0, fog_density=8.0, fog_distance=40.0),
    "monsoon_storm": dict(_BASE, cloudiness=100.0, precipitation=100.0, precipitation_deposits=95.0, wetness=100.0, wind_intensity=70.0, fog_density=15.0, fog_distance=30.0),
    "wet_road": dict(_BASE, cloudiness=60.0, precipitation=0.0, precipitation_deposits=60.0, wetness=80.0),
    "valley_fog": dict(_BASE, cloudiness=70.0, fog_density=55.0, fog_distance=8.0, fog_falloff=0.6, wetness=30.0),
    "dense_fog": dict(_BASE, cloudiness=90.0, fog_density=90.0, fog_distance=3.0, fog_falloff=1.0, wetness=40.0),
    "low_sun": dict(_BASE, cloudiness=5.0, sun_azimuth_angle=0.0, sun_altitude_angle=6.0, scattering_intensity=1.5),
    "low_sun_haze": dict(_BASE, cloudiness=15.0, sun_azimuth_angle=0.0, sun_altitude_angle=4.0, fog_density=12.0, fog_distance=60.0, mie_scattering_scale=0.08, scattering_intensity=2.0),
    "clear_night": dict(_BASE, cloudiness=5.0, sun_altitude_angle=-30.0),
    "wet_night": dict(_BASE, cloudiness=80.0, sun_altitude_angle=-30.0, wetness=80.0, precipitation_deposits=50.0),
    "dust_haze": dict(_BASE, cloudiness=20.0, dust_storm=35.0, fog_density=10.0, fog_distance=80.0),
}

# Sun altitude by time of day. ``None`` keeps the preset's own value, which
# matters for low_sun and night presets that encode the sun position.
TIME_OF_DAY_ALTITUDE: Dict[str, Optional[float]] = {
    "dawn": 8.0,
    "day": 60.0,
    "dusk": 8.0,
    "night": -30.0,
}

TIME_OF_DAY_AZIMUTH: Dict[str, Optional[float]] = {
    "dawn": 90.0,
    "day": 150.0,
    "dusk": 270.0,
    "night": 0.0,
}

# Human-readable lighting class used for benchmark breakdowns.
LIGHTING_CLASS = {
    "dawn": "low_light",
    "day": "daylight",
    "dusk": "low_light",
    "night": "night",
}


def resolve(preset: str, time_of_day: str = "day") -> Dict[str, float]:
    """Return the weather dictionary for a preset at a time of day."""
    if preset not in PRESETS:
        raise KeyError("unknown weather preset '%s'; known: %s" % (preset, ", ".join(sorted(PRESETS))))
    if time_of_day not in TIME_OF_DAY_ALTITUDE:
        raise KeyError("unknown time_of_day '%s'" % time_of_day)
    weather = copy.deepcopy(PRESETS[preset])
    # Sun-position presets keep their own altitude; the others follow the clock.
    if not preset.startswith("low_sun") and not preset.endswith("night"):
        weather["sun_altitude_angle"] = TIME_OF_DAY_ALTITUDE[time_of_day]
        weather["sun_azimuth_angle"] = TIME_OF_DAY_AZIMUTH[time_of_day]
    elif preset.endswith("night"):
        weather["sun_altitude_angle"] = -30.0
    return weather


def to_carla(weather: Dict[str, float]):
    """Build a ``carla.WeatherParameters`` from a resolved weather dictionary."""
    import carla  # imported lazily so the rest of the package runs without CARLA

    params = carla.WeatherParameters()
    for name in WEATHER_FIELDS:
        if name in weather:
            setattr(params, name, float(weather[name]))
    return params


def visibility_class(weather: Dict[str, float]) -> str:
    """Coarse visibility class used for benchmark breakdowns."""
    if weather.get("fog_density", 0.0) >= 40.0:
        return "fog"
    if weather.get("precipitation", 0.0) >= 60.0:
        return "heavy_rain"
    if weather.get("precipitation", 0.0) > 0.0:
        return "rain"
    if weather.get("dust_storm", 0.0) > 0.0:
        return "dust"
    if weather.get("wetness", 0.0) >= 50.0:
        return "wet"
    return "clear"
