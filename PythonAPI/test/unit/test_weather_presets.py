# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.
"""Native weather preset values remain independent when callers customize them."""

import re
import unittest

import carla


PRESETS = (
    "Default", "ClearNoon", "CloudyNoon", "WetNoon", "WetCloudyNoon",
    "MidRainyNoon", "HardRainNoon", "SoftRainNoon", "ClearSunset",
    "CloudySunset", "WetSunset", "WetCloudySunset", "MidRainSunset",
    "HardRainSunset", "SoftRainSunset", "ClearNight", "CloudyNight",
    "WetNight", "WetCloudyNight", "SoftRainNight", "MidRainyNight",
    "HardRainNight", "DustStorm",
)


class TestWeatherPresets(unittest.TestCase):
    def test_reported_clear_sunset_mutation_does_not_change_the_preset(self):
        weather = carla.WeatherParameters.ClearSunset
        original = weather.wind_intensity
        try:
            weather.wind_intensity = 0.0
            self.assertEqual(carla.WeatherParameters.ClearSunset.wind_intensity, original)
            self.assertEqual(weather.wind_intensity, 0.0)
        finally:
            weather.wind_intensity = original

    def test_every_preset_access_returns_an_independent_equal_value(self):
        for name in PRESETS:
            with self.subTest(preset=name):
                first = getattr(carla.WeatherParameters, name)
                second = getattr(carla.WeatherParameters, name)
                self.assertIsInstance(first, carla.WeatherParameters)
                self.assertEqual(first, second)
                self.assertIsNot(first, second)

    def test_preset_access_through_an_instance_is_independent(self):
        instance = carla.WeatherParameters()
        weather = instance.ClearNoon
        original = weather.cloudiness
        try:
            weather.cloudiness = original + 1.0
            self.assertEqual(instance.ClearNoon.cloudiness, original)
            self.assertEqual(carla.WeatherParameters.ClearNoon.cloudiness, original)
        finally:
            weather.cloudiness = original

    def test_existing_preset_names_remain_discoverable(self):
        names = {name for name in dir(carla.WeatherParameters) if re.match("[A-Z]", name)}
        self.assertEqual(names, set(PRESETS))

    def test_representative_preset_values_are_unchanged(self):
        sunset = carla.WeatherParameters.ClearSunset
        self.assertEqual((sunset.cloudiness, sunset.wind_intensity, sunset.sun_altitude_angle),
                         (5.0, 10.0, 15.0))
        self.assertEqual(carla.WeatherParameters.DustStorm.dust_storm, 100.0)
        self.assertEqual(carla.WeatherParameters.Default.cloudiness, -1.0)
        self.assertEqual(carla.WeatherParameters.HardRainNight.precipitation, 100.0)

    def test_custom_weather_values_remain_mutable_and_compare_by_value(self):
        first = carla.WeatherParameters(cloudiness=12.0, wind_intensity=15.0)
        second = carla.WeatherParameters(cloudiness=12.0, wind_intensity=15.0)
        self.assertEqual(first, second)
        first.cloudiness = 42.0
        self.assertNotEqual(first, second)
        self.assertEqual(second.cloudiness, 12.0)
        self.assertEqual(carla.WeatherParameters().cloudiness, 0.0)


if __name__ == "__main__":
    unittest.main()
