#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Set or inspect the weather of a running CARLA Simulator instance.

Start from a preset and/or override individual parameters:

    python3 set_weather.py --preset ClearNoon --cloudiness 40 --wind-intensity 20
    python3 set_weather.py --sun-altitude-angle -5 --sun-azimuth-angle 210
    python3 set_weather.py --show
    python3 set_weather.py --list-presets
"""

import argparse

import carla


# (flag name, WeatherParameters attribute) -- flag names use '-' for the
# CLI, argparse maps them back to '_' automatically.
PARAMETERS = [
    'cloudiness',
    'precipitation',
    'precipitation_deposits',
    'wind_intensity',
    'sun_azimuth_angle',
    'sun_altitude_angle',
    'fog_density',
    'fog_distance',
    'fog_falloff',
    'wetness',
    'scattering_intensity',
    'mie_scattering_scale',
    'rayleigh_scattering_scale',
    'dust_storm',
]


def list_presets():
    return sorted(
        name for name in dir(carla.WeatherParameters)
        if name[0].isupper()
        and isinstance(getattr(carla.WeatherParameters, name), carla.WeatherParameters))


def print_weather(label, wp):
    print(label)
    for name in PARAMETERS:
        print('    %-28s %s' % (name, getattr(wp, name)))


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                      formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('-p', '--port', type=int, default=2000)
    parser.add_argument('--preset', metavar='NAME',
                         help='start from a named preset (see --list-presets), '
                              'then apply any parameter overrides below on top')
    parser.add_argument('--list-presets', action='store_true',
                         help='print available preset names and exit')
    parser.add_argument('--show', action='store_true',
                         help='print the current weather and exit (no changes made)')
    for name in PARAMETERS:
        flag = '--' + name.replace('_', '-')
        parser.add_argument(flag, type=float, default=None,
                             help='override WeatherParameters.%s' % name)
    args = parser.parse_args()

    if args.list_presets:
        print('\n'.join(list_presets()))
        return

    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    world = client.get_world()

    if args.show:
        print_weather('current weather:', world.get_weather())
        return

    if args.preset:
        if not hasattr(carla.WeatherParameters, args.preset):
            parser.error('unknown preset %r -- see --list-presets' % args.preset)
        wp = getattr(carla.WeatherParameters, args.preset)
    else:
        wp = world.get_weather()

    for name in PARAMETERS:
        value = getattr(args, name)
        if value is not None:
            setattr(wp, name, value)

    world.set_weather(wp)
    print_weather('weather set to:', world.get_weather())


if __name__ == '__main__':
    main()
