#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import argparse
import sys
import carla

OBJECT_LABELS = {
    'Buildings': carla.CityObjectLabel.Buildings,
    'Fences': carla.CityObjectLabel.Fences,
    'Other': carla.CityObjectLabel.Other,
    'Pedestrians': carla.CityObjectLabel.Pedestrians,
    'Poles': carla.CityObjectLabel.Poles,
    'RoadLines': carla.CityObjectLabel.RoadLines,
    'Roads': carla.CityObjectLabel.Roads,
    'Sidewalks': carla.CityObjectLabel.Sidewalks,
    'TrafficSigns': carla.CityObjectLabel.TrafficSigns,
    'Vegetation': carla.CityObjectLabel.Vegetation,
    'Car': carla.CityObjectLabel.Car,
    'Bus': carla.CityObjectLabel.Bus,
    'Truck': carla.CityObjectLabel.Truck,
    'Motorcycle': carla.CityObjectLabel.Motorcycle,
    'Bicycle': carla.CityObjectLabel.Bicycle,
    'Rider': carla.CityObjectLabel.Rider,
    'Train': carla.CityObjectLabel.Train,
    'Walls': carla.CityObjectLabel.Walls,
    'Sky': carla.CityObjectLabel.Sky,
    'Ground': carla.CityObjectLabel.Ground,
    'Bridge': carla.CityObjectLabel.Bridge,
    'RailTrack': carla.CityObjectLabel.RailTrack,
    'GuardRail': carla.CityObjectLabel.GuardRail,
    'TrafficLight': carla.CityObjectLabel.TrafficLight,
    'Static': carla.CityObjectLabel.Static,
    'Dynamic': carla.CityObjectLabel.Dynamic,
    'Water': carla.CityObjectLabel.Water,
    'Terrain': carla.CityObjectLabel.Terrain,
    'Any': carla.CityObjectLabel.Any,
    'NONE': carla.CityObjectLabel.NONE
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
        '-l', '--label', default='Any',
        help='Layer to manage')
    argparser.add_argument(
        '--id', default=None, type=int,
        help='ID to manage')
    argparser.add_argument(
        '--show', default=None, type=float,
        help='Duration of the visualization of objects data. 0 = disabled')
    argparser.add_argument(
        '--action', choices=["enable", "disable"],
        help='Action done to the selected layer')
    argparser.add_argument(
        '--summary', action='store_true',
        help='Print a summary of the amount of objects in each label')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    world = client.get_world()

    if args.id and args.label != 'Any':
        print("Cannot use '--id' and '--label' at the same time")
        print("Use '--id' to manage one specific object or '--label' for all the ones part of the label")

    if args.summary:
        print("Showcasing a summary of the labels and amount of object in them")
        for v in list(OBJECT_LABELS.values()):
            print(f"{v} - {len(world.get_environment_objects(v))}")
        sys.exit()

    # Manage one object
    if args.id:
        objects = list(filter(lambda k: k.id == args.id, world.get_environment_objects()))
        if not objects:
            print(f"Couldn't find an object with id '{args.id}'. Stopping")
            sys.exit()
        elif len(objects) > 1:
            print(f"Found more than one object with id '{args.id}'. Stopping")
            sys.exit()
        object = objects[0]

        if args.show:
            print(f"Showing the data of the environment object with id {args.id}. Stopping")
            obj_location = object.transform.location
            obj_bb = object.bounding_box
            text = f"[{object.id}] {object.name}"
            text_location = carla.Location(x=obj_bb.location.x, y=obj_bb.location.y, z=obj_bb.location.z)
            text_location += 1.2*obj_bb.extent.z * object.transform.get_up_vector()

            world.debug.draw_point(obj_location, life_time=args.show, size=0.05, color=carla.Color(0, 0, 200))
            world.debug.draw_box(obj_bb, obj_bb.rotation, life_time=args.show, thickness=0.05, color=carla.Color(0, 0, 200))
            world.debug.draw_string(text_location, text, life_time=args.show, color=carla.Color(0, 0, 200))

        if args.action == 'enable':
            print(f"Enabling the enviroment object with id {args.id}")
            world.enable_environment_objects([object.id], True)
        elif args.action == 'disable':
            print(f"Disabling the enviroment object with id {args.id}")
            world.enable_environment_objects([object.id], False)

    # Manage an object label
    else:
        try:
            objects = world.get_environment_objects(OBJECT_LABELS[args.label])
        except KeyError:
            print(f"CityObject label '{args.label}' doesn't exist. The possible labels are:")
            print(list(OBJECT_LABELS.keys()))
            sys.exit()

        if args.show:
            print(f"Showing all the data of the environment objects with label {args.label}")
            for obj in objects:
                obj_location = obj.transform.location
                obj_bb = obj.bounding_box
                text = f"[{obj.id}] {obj.name}"
                text_location = carla.Location(x=obj_bb.location.x, y=obj_bb.location.y, z=obj_bb.location.z)
                text_location += 1.2*obj_bb.extent.z * obj.transform.get_up_vector()

                world.debug.draw_point(obj_location, life_time=args.show, size=0.05, color=carla.Color(0, 0, 200))
                world.debug.draw_box(obj_bb, obj_bb.rotation, life_time=args.show, thickness=0.05, color=carla.Color(0, 0, 200))
                world.debug.draw_string(text_location, text, life_time=args.show, color=carla.Color(0, 0, 200))

        if args.action == 'enable':
            print(f"Enabling the enviroment objects with label {args.label}")
            world.enable_environment_objects([obj.id for obj in objects], True)
        elif args.action == 'disable':
            print(f"Disabling enviroment objects with label {args.label}")
            world.enable_environment_objects([obj.id for obj in objects], False)


if __name__ == '__main__':
    main()
