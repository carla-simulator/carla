import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
from math import sin
import argparse

# -------------------------------------------------

def main():
    argparser = argparse.ArgumentParser(
        description="CARLA weather example")
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
        help='Sun position presets [day | night | sunset]')
    argparser.add_argument(
        '--weather',
        default=None,
        type=str,
        help='Weather condition presets [clear | overcast | rain]')
    argparser.add_argument(
        '--altitude', '-alt',
        metavar='A',
        default=None,
        type=float,
        help='Sun altitude [-90 to 90]')
    argparser.add_argument(
        '--azimuth', '-azm',
        metavar='A',
        default=None,
        type=float,
        help='Sun azimuth [0 to 360]')
    argparser.add_argument(
        '--clouds', '-c',
        metavar='C',
        default=None,
        type=float,
        help='Clouds amount [0.0 to 100.0]')
    argparser.add_argument(
        '--rain', '-r',
        default=None,
        type=float,
        help='Rain amount [0.0 to 100.0]')
    argparser.add_argument(
        '--puddles', '-pd',
        default=None,
        type=float,
        help='Puddles amount [0.0 to 100.0]')
    argparser.add_argument(
        '--wind', '-w',
        default=None,
        type=float,
        help='Wind intensity [0.0 to 100.0]')
    argparser.add_argument(
        '--fog', '-f',
        default=None,
        type=float,
        help='Fog intensity [0.0 to 100.0]')
    argparser.add_argument(
        '--fogdist', '-fd',
        default=None,
        type=float,
        help='Fog Distance')
    argparser.add_argument(
        '--wetness', '-wet',
        default=None,
        type=float,
        help='Wetness')
    argparser.add_argument(
        '-s',
        default=1.0,
        type=float,
        help='Dynamic weather time factor (default: 1.0)')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(2.0)

    world = client.get_world()

    weather = world.get_weather()

    run_dynamic_simulation = True

    ####   PRESETS   ###
    if args.sun is not None:
        ##  Sun Position  ##
        if args.sun == 'day':
            weather.sun_altitude_angle = 90
            weather.sun_azimuth_angle = 220.0
            run_dynamic_simulation = False
        elif args.sun == 'night':
            weather.sun_altitude_angle = -90
            weather.sun_azimuth_angle = 220.0
            run_dynamic_simulation = False
        elif args.sun == 'sunset':
            weather.sun_altitude_angle = 181
            weather.sun_azimuth_angle = 100
            run_dynamic_simulation = False
        else:
            print("[ERROR]: Command [--sun | -s] ´" + args.sun + "´ not known")
            sys.exit(1)

    ##  Weather  ##
    if args.weather is not None:
        if args.weather == 'clear':
            weather.cloudiness = 7.0
            weather.precipitation = 0.0
            weather.precipitation_deposits = 0.0
            weather.wind_intensity = 5.0
            run_dynamic_simulation = False
        elif args.weather == 'overcast':
            weather.cloudiness = 80.0
            weather.precipitation = 0.0
            weather.precipitation_deposits = 0.0
            weather.wind_intensity = 50.0
            run_dynamic_simulation = False
        elif args.weather == 'rain':
            weather.cloudiness = 100.0
            weather.precipitation = 80.0
            weather.precipitation_deposits = 90.0
            weather.wind_intensity = 100.0
            weather.fog_density = 20.0
            weather.fog_distance = 30.0
            weather.wetness = 100.0
            run_dynamic_simulation = False
        else:
            print("[ERROR]: Command [--weather | -w] ´" + args.weather + "´ not known")
            sys.exit(1)




    if args.azimuth is not None:
        weather.sun_azimuth_angle = args.azimuth
        run_dynamic_simulation = False

    if args.altitude is not None:
        weather.sun_altitude_angle = args.altitude
        run_dynamic_simulation = False

    if args.clouds is not None:
        weather.cloudiness = args.clouds
        run_dynamic_simulation = False

    if args.rain is not None:
        weather.precipitation = args.rain
        run_dynamic_simulation = False

    if args.puddles is not None:
        weather.precipitation_deposits = args.puddles
        run_dynamic_simulation = False

    if args.wind is not None:
        weather.wind_intensity = args.wind
        run_dynamic_simulation = False

    if args.fog is not None:
        weather.fog_density = args.fog
        run_dynamic_simulation = False

    if args.fogdist is not None:
        weather.fog_distance = args.fogdist
        run_dynamic_simulation = False

    if args.wetness is not None:
        weather.wetness = args.wetness
        run_dynamic_simulation = False

    if run_dynamic_simulation:
        speed_factor = args.s
        update_freq = 0.000001 / speed_factor
        elapsed_time = 0.0
        total_time = 0.0

        while True:
            timestamp = world.wait_for_tick(seconds=30.0).timestamp
            elapsed_time += timestamp.delta_seconds
            if elapsed_time > update_freq:                
                # tick -> speed_factor * elapsed_time
                delta_time = speed_factor * elapsed_time
                total_time += delta_time
                world.set_weather(weather)




                weather.sun_altitude_angle += delta_time * 15.0

                weather.sun_azimuth_angle += delta_time * 0.5
                weather.sun_azimuth_angle %= 360.0

                weather.cloudiness = max(0, sin(total_time/10) * 100)

                weather.precipitation = max(0, sin(total_time/20) * 150 - 50)
                weather.precipitation_deposits = max(0, sin(total_time/20) * 130 - 30)
                weather.wetness = max(0, sin(total_time/15) * 150 - 50)
                weather.wind_intensity = max(30, sin(total_time/20) * 150 - 50)




                info_str = '\r' + str('Altitude: {:6.2f} Azimuth: {:6.2f} Clouds: {:4.2f} Wind: {:4.2f} Rain: {:4.2f} Puddles: {:4.2f} Wetness: {:4.2f} FogDensity: {:4.2f}'.format(
                    weather.sun_altitude_angle,
                    weather.sun_azimuth_angle,
                    weather.cloudiness,
                    weather.wind_intensity,
                    weather.precipitation,
                    weather.precipitation_deposits,
                    weather.wetness,
                    weather.fog_density)) + 5 * ' '

                sys.stdout.write(info_str)
                sys.stdout.flush()

                elapsed_time = 0.0

    else:
        world.set_weather(weather)











if __name__ == '__main__':
    main()