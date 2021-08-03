"""
This file is used to utilise ray library.
"""

import ray

from util.classes.sensor_data import (
    SensorDataHandler as SDH,
    RadarSensorDataHandler as RSDH,
    ObstacleSensorDataHandler as OSDH
)

@ray.remote
class RadarSensorDataHandler(RSDH):
    pass


@ray.remote
class ObstacleSensorDataHandler(OSDH):
    pass


@ray.remote
class SensorDataHandler(SDH):
    pass
