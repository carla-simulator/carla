#!/usr/bin/env python
"""
Script to add new vehicles to the vehicle factory and json file
"""
import unreal
import argparse
import json

argparser = argparse.ArgumentParser()
argparser.add_argument(
    '-v', '--vehicle_blueprint_path',
    metavar='V',
    default='',
    type=str,
    help='Path to add to the vehicle blueprint')
argparser.add_argument(
    '-n', '--name',
    metavar='N',
    default='',
    type=str,
    help='vehicle name')
args = argparser.parse_args()

# load vehicle and factory
vehicle_factory_path = '/Game/Carla/Blueprints/Vehicles/VehicleFactory.VehicleFactory_C'
vehicle_factory_class = unreal.load_object(None, vehicle_factory_path)
vehicle_factory_default_object = unreal.get_default_object(vehicle_factory_class)
vehicle_blueprint_path = args.vehicle_blueprint_path + '_C'
vehicle_blueprint = unreal.load_object(None, vehicle_blueprint_path)
vehicle_list = vehicle_factory_default_object.get_editor_property("Vehicles")

# generate the new field
new_vehicle_parameters = unreal.VehicleParameters()
new_vehicle_parameters.make = 'usd'
new_vehicle_parameters.model = args.name
new_vehicle_parameters.class_ = vehicle_blueprint
new_vehicle_parameters.generation = 2
new_vehicle_parameters.base_type = 'car'

# add to the vehicle list
vehicle_list.append(new_vehicle_parameters)
