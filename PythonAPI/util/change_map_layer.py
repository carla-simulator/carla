#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import argparse
import sys
import carla


MAP_LAYERS = {
    'Buildings': carla.MapLayer.Buildings,
    'Decals':  carla.MapLayer.Decals,
    'Foliage':  carla.MapLayer.Foliage,
    'Ground':  carla.MapLayer.Ground,
    'ParkedVehicles':  carla.MapLayer.ParkedVehicles,
    'Particles':  carla.MapLayer.Particles,
    'Props':  carla.MapLayer.Props,
    'StreetLights':  carla.MapLayer.StreetLights,
    'Walls':  carla.MapLayer.Walls,
    'All':  carla.MapLayer.All
}


def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client')
    argparser.add_argument(
        '--host', metavar='H', default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port', metavar='P', default=2000, type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-l', '--layer',required=True,
        help='Layer to load / unload')
    argparser.add_argument(
        '--action', choices=["load", "unload"],
        help='Layer to load / unload')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    world = client.get_world()
    try:
        layer = MAP_LAYERS[args.layer]
    except KeyError:
        print(f"Map layer '{args.layer}' doesn't exist. The possible map layers are:")
        print(list(MAP_LAYERS.keys()))
        sys.exit()

    if args.action == 'load':
        print(f"Loading layer {layer}")
        world.load_map_layer(layer)
    elif args.action == 'unload':
        print(f"Unloading layer {layer}")
        world.unload_map_layer(layer)

if __name__ == '__main__':
    main()
