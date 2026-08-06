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
    help='vehicle name (the Model half of the blueprint id)')
argparser.add_argument(
    '--make',
    default='',
    type=str,
    help='manufacturer (the Make half of the blueprint id); '
         'defaults to the folder the blueprint lives in')
argparser.add_argument(
    '--base-type',
    dest='base_type',
    default='car',
    type=str,
    help='car / truck / van / bus / motorcycle / bicycle')
argparser.add_argument(
    '--object-type',
    dest='object_type',
    default='',
    type=str,
    help='FVehicleParameters.ObjectType')
argparser.add_argument(
    '--special-type',
    dest='special_type',
    default='',
    type=str,
    help='electric / emergency / taxi ...')
argparser.add_argument(
    '--generation',
    default=2,
    type=int,
    help='asset generation attribute')
argparser.add_argument(
    '--number-of-wheels',
    dest='number_of_wheels',
    default=4,
    type=int,
    help='wheel count')
argparser.add_argument(
    '--no-lights',
    dest='has_lights',
    action='store_false',
    help='clear HasLights (set by default)')
argparser.add_argument(
    '--dynamic-doors',
    dest='has_dynamic_doors',
    action='store_true',
    help='set HasDynamicDoors')
argparser.add_argument(
    '--result',
    default='',
    type=str,
    help='optional path to write a JSON summary to')
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
# Make defaults to the folder the blueprint sits in, which is the convention for the
# shipped vehicles (Blueprints/Vehicles/<Make>/BP_<Model>), but any of these can be
# given explicitly: the blueprint id is vehicle.<make>.<model>, and BaseType is what
# Traffic Manager and most example scripts filter on.
new_vehicle_parameters.make = args.make or unreal.Paths.get_base_filename(
    unreal.Paths.get_path(args.vehicle_blueprint_path))
new_vehicle_parameters.model = args.name
new_vehicle_parameters.class_ = vehicle_blueprint
new_vehicle_parameters.generation = args.generation
new_vehicle_parameters.number_of_wheels = args.number_of_wheels
new_vehicle_parameters.base_type = args.base_type
new_vehicle_parameters.object_type = args.object_type
new_vehicle_parameters.special_type = args.special_type
new_vehicle_parameters.has_lights = args.has_lights
new_vehicle_parameters.has_dynamic_doors = args.has_dynamic_doors

# Re-registering the same blueprint updates its entry instead of adding a second one
# that shadows it.
replaced = None
for index, entry in enumerate(vehicle_list):
    existing = entry.get_editor_property('class_')
    if existing is not None and str(existing) == str(vehicle_blueprint):
        replaced = index
        break

if replaced is None:
    vehicle_list.append(new_vehicle_parameters)
    action = 'appended'
else:
    vehicle_list[replaced] = new_vehicle_parameters
    action = 'updated'

# Write the list back: get_editor_property returns a copy, so mutating it alone does
# not reach the CDO.
vehicle_factory_default_object.set_editor_property("Vehicles", vehicle_list)

# save vehicle factory
saved = unreal.EditorAssetLibrary.save_asset(vehicle_factory_path, False)

blueprint_id = 'vehicle.%s.%s' % (new_vehicle_parameters.make.lower(),
                                  new_vehicle_parameters.model.lower())
print('%s %s in %s (%d entries)' % (
    action, blueprint_id, vehicle_factory_path,
    len(vehicle_factory_default_object.get_editor_property("Vehicles"))))

if args.result:
    with open(args.result, 'w') as handle:
        json.dump({'ok': bool(saved), 'action': action, 'id': blueprint_id,
                   'make': new_vehicle_parameters.make,
                   'model': new_vehicle_parameters.model,
                   'base_type': args.base_type, 'generation': args.generation,
                   'entries_after': len(
                       vehicle_factory_default_object.get_editor_property("Vehicles"))},
                  handle, indent=2)