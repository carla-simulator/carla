#!/usr/bin/env python

"""
Script to control weather parameters in simulations
"""

import glob
import os
import sys
import argparse

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

SUN_PRESETS = {
    'day': (45.0, 0.0),
    'night': (-90.0, 0.0),
    'sunset': (0.5, 0.0)}

WEATHER_PRESETS = {
    'clear': [10.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.2, 0.0, 0.0, 0.0, 0.0331],
    'overcast': [80.0, 0.0, 0.0, 50.0, 2.0, 0.75, 0.1, 10.0, 0.0, 0.03, 0.0331],
    'rain': [100.0, 80.0, 90.0, 100.0, 7.0, 0.75, 0.1, 100.0, 0.0, 0.03, 0.0331]}

CAR_LIGHTS = {
    'None' : [carla.VehicleLightState.NONE],
    'Position' : [carla.VehicleLightState.Position],
    'LowBeam' : [carla.VehicleLightState.LowBeam],
    'HighBeam' : [carla.VehicleLightState.HighBeam],
    'Brake' : [carla.VehicleLightState.Brake],
    'RightBlinker' : [carla.VehicleLightState.RightBlinker],
    'LeftBlinker' : [carla.VehicleLightState.LeftBlinker],
    'Reverse' : [carla.VehicleLightState.Reverse],
    'Fog' : [carla.VehicleLightState.Fog],
    'Interior' : [carla.VehicleLightState.Interior],
    'Special1' : [carla.VehicleLightState.Special1],
    'Special2' : [carla.VehicleLightState.Special2],
    'All' : [carla.VehicleLightState.All]}

LIGHT_GROUP = {
    'None' : [carla.LightGroup.NONE],
    # 'Vehicle' : [carla.LightGroup.Vehicle],
    'Street' : [carla.LightGroup.Street],
    'Building' : [carla.LightGroup.Building],
    'Other' : [carla.LightGroup.Other]}

def apply_sun_presets(args, weather):
    """Uses sun presets to set the sun position"""
    if args.sun is not None:
        if args.sun in SUN_PRESETS:
            weather.sun_altitude_angle = SUN_PRESETS[args.sun][0]
            weather.sun_azimuth_angle = SUN_PRESETS[args.sun][1]
        else:
            print("[ERROR]: Command [--sun | -s] '" + args.sun + "' not known")
            sys.exit(1)


def apply_weather_presets(args, weather):
    """Uses weather presets to set the weather parameters"""
    if args.weather is not None:
        if args.weather in WEATHER_PRESETS:
            weather.cloudiness = WEATHER_PRESETS[args.weather][0]
            weather.precipitation = WEATHER_PRESETS[args.weather][1]
            weather.precipitation_deposits = WEATHER_PRESETS[args.weather][2]
            weather.wind_intensity = WEATHER_PRESETS[args.weather][3]
            weather.fog_density = WEATHER_PRESETS[args.weather][4]
            weather.fog_distance = WEATHER_PRESETS[args.weather][5]
            weather.fog_falloff = WEATHER_PRESETS[args.weather][6]
            weather.wetness = WEATHER_PRESETS[args.weather][7]
            weather.scattering_intensity = WEATHER_PRESETS[args.weather][8]
            weather.mie_scattering_scale = WEATHER_PRESETS[args.weather][9]
            weather.rayleigh_scattering_scale = WEATHER_PRESETS[args.weather][10]
        else:
            print("[ERROR]: Command [--weather | -w] '" + args.weather + "' not known")
            sys.exit(1)


def apply_weather_values(args, weather):
    """Set weather values individually"""
    if args.azimuth is not None:
        weather.sun_azimuth_angle = args.azimuth
    if args.altitude is not None:
        weather.sun_altitude_angle = args.altitude
    if args.clouds is not None:
        weather.cloudiness = args.clouds
    if args.rain is not None:
        weather.precipitation = args.rain
    if args.puddles is not None:
        weather.precipitation_deposits = args.puddles
    if args.wind is not None:
        weather.wind_intensity = args.wind
    if args.fog is not None:
        weather.fog_density = args.fog
    if args.fogdist is not None:
        weather.fog_distance = args.fogdist
    if args.fogfalloff is not None:
        weather.fog_falloff = args.fogfalloff
    if args.wetness is not None:
        weather.wetness = args.wetness
    if args.scatteringintensity is not None:
        weather.scattering_intensity = args.scatteringintensity
    if args.miescatteringscale is not None:
        weather.mie_scattering_scale = args.miescatteringscale
    if args.rayleighscatteringscale is not None:
        weather.rayleigh_scattering_scale = args.rayleighscatteringscale


def apply_lights_to_cars(args, world):
    if args.cars is None:
        return

    light_mask = carla.VehicleLightState.NONE
    for option in args.cars:
        light_mask |= CAR_LIGHTS[option][0]

    # Get all cars in level
    all_vehicles = world.get_actors()
    for ve in all_vehicles:
        if "vehicle." in ve.type_id:
            ve.set_light_state(carla.VehicleLightState(light_mask))

def apply_lights_manager(args, light_manager):
    if args.lights is None:
        return

    light_group = 'None'
    if args.lightgroup is not None:
        light_group = args.lightgroup

    # filter by group
    lights = light_manager.get_all_lights(LIGHT_GROUP[light_group][0]) # light_group

    i = 0
    while (i < len(args.lights)):
        option = args.lights[i]

        if option == "on":
            light_manager.turn_on(lights)
        elif option == "off":
            light_manager.turn_off(lights)
        elif option == "intensity":
            light_manager.set_intensity(lights, int(args.lights[i + 1]))
            i += 1
        elif option == "color":
            r = int(args.lights[i + 1])
            g = int(args.lights[i + 2])
            b = int(args.lights[i + 3])
            light_manager.set_color(lights, carla.Color(r, g, b))
            i += 3

        i += 1


def main():
    """Start function"""
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '--sun',
        default=None,
        type=str,
        help='Sun position presets [' + ' | '.join([i for i in SUN_PRESETS]) + ']')
    argparser.add_argument(
        '--weather',
        default=None,
        type=str,
        help='Weather condition presets [' + ' | '.join([i for i in WEATHER_PRESETS]) + ']')
    argparser.add_argument(
        '--altitude', '-alt',
        metavar='A',
        default=None,
        type=float,
        help='Sun altitude [-90.0, 90.0]')
    argparser.add_argument(
        '--azimuth', '-azm',
        metavar='A',
        default=None,
        type=float,
        help='Sun azimuth [0.0, 360.0]')
    argparser.add_argument(
        '--clouds', '-c',
        metavar='C',
        default=None,
        type=float,
        help='Clouds amount [0.0, 100.0]')
    argparser.add_argument(
        '--rain', '-r',
        metavar='R',
        default=None,
        type=float,
        help='Rain amount [0.0, 100.0]')
    argparser.add_argument(
        '--puddles', '-pd',
        metavar='Pd',
        default=None,
        type=float,
        help='Puddles amount [0.0, 100.0]')
    argparser.add_argument(
        '--wind', '-w',
        metavar='W',
        default=None,
        type=float,
        help='Wind intensity [0.0, 100.0]')
    argparser.add_argument(
        '--fog', '-f',
        metavar='F',
        default=None,
        type=float,
        help='Fog intensity [0.0, 100.0]')
    argparser.add_argument(
        '--fogdist', '-fd',
        metavar='Fd',
        default=None,
        type=float,
        help='Fog Distance [0.0, 100.0)')
    argparser.add_argument(
        '--fogfalloff', '-fo',
        metavar='Fo',
        default=None,
        type=float,
        help='Fog Falloff [0.0, inf)')
    argparser.add_argument(
        '--wetness', '-wet',
        metavar='Wet',
        default=None,
        type=float,
        help='Wetness intensity [0.0, 100.0]')
    argparser.add_argument(
        '--scatteringintensity', '-si',
        metavar='si',
        default=None,
        type=float,
        help='Scattering intensity [0.0, inf]')
    argparser.add_argument(
        '--rayleighscatteringscale', '-rss',
        metavar='rss',
        default=None,
        type=float,
        help='Rayleigh scattering scale [0.0, 2.0]')
    argparser.add_argument(
        '--miescatteringscale', '-mss',
        metavar='mss',
        default=None,
        type=float,
        help='Mie scattering scale [0.0, 5.0]')
    argparser.add_argument(
        '--cars',
        metavar='Cars',
        default=None,
        type=str,
        nargs='+',
        help='Light Cars [' + ' | '.join([i for i in CAR_LIGHTS]) + ']')
    argparser.add_argument(
        '--lights', '-l',
        metavar='Lights',
        default=None,
        type=str,
        nargs='+',
        help='Street Lights []')
    argparser.add_argument(
        '--lightgroup', '-lg',
        metavar='LightGroup',
        default=None,
        type=str,
        help='Light Group [' + ' | '.join([i for i in LIGHT_GROUP]) + ']')
    args = argparser.parse_args()

    # since all the arguments are None by default
    # (except for the first 2, host and port)
    # we can check if all the arguments have been provided
    arg_values = [v for _, v in args.__dict__.items()][2:]
    if all(i is (None and False) for i in arg_values):
        argparser.error('No arguments provided.')

    client = carla.Client(args.host, args.port)
    client.set_timeout(2.0)
    world = client.get_world()

    weather = world.get_weather()

    # apply presets
    apply_sun_presets(args, weather)
    apply_weather_presets(args, weather)

    # apply weather values individually
    apply_weather_values(args, weather)

    world.set_weather(weather)

    # apply car light changes
    apply_lights_to_cars(args, world)

    apply_lights_manager(args, world.get_lightmanager())

    world.wait_for_tick()


if __name__ == '__main__':
    main()
