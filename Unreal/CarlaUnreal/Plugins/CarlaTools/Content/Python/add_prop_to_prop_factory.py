#!/usr/bin/env python
"""
Script to add new props to the prop factory and json file
"""
import unreal
import argparse
import json

# convert PropSize to string
def size_to_str(prop_size):
    if prop_size == unreal.PropSize.TINY:
        return 'Tiny'
    elif prop_size == unreal.PropSize.SMALL:
        return 'Small'
    elif prop_size == unreal.PropSize.MEDIUM:
        return 'Medium'
    elif prop_size == unreal.PropSize.BIG:
        return 'Big'
    elif prop_size == unreal.PropSize.HUGE:
        return 'Huge'
    else:
        return 'Medium'

# convert string to PropSize
def str_to_size(prop_size):
    if prop_size == "tiny":
        return unreal.PropSize.TINY
    elif prop_size == "small":
        return unreal.PropSize.SMALL
    elif prop_size == "medium":
        return unreal.PropSize.MEDIUM
    elif prop_size == "big":
        return unreal.PropSize.BIG
    elif prop_size == "huge":
        return unreal.PropSize.HUGE
    else:
        return unreal.PropSize.MEDIUM

argparser = argparse.ArgumentParser()
argparser.add_argument(
    '-s', '--static_mesh_path',
    metavar='S',
    default='',
    type=str,
    help='Path to add to static mesh')
argparser.add_argument(
    '-n', '--name',
    metavar='N',
    default='',
    type=str,
    help='prop name')
argparser.add_argument(
    '--size',
    metavar='Z',
    default='',
    type=str,
    help='prop size')
args = argparser.parse_args()

# load prop and static mesh objects
prop_factory_path = '/Game/Carla/Blueprints/Props/PropFactory.PropFactory_C'
prop_factory_class = unreal.load_object(None, prop_factory_path)
prop_factory_default_object = unreal.get_default_object(prop_factory_class)
static_mesh = unreal.load_object(None, args.static_mesh_path)
definitions_map = prop_factory_default_object.get_editor_property("DefinitionsMap")

# generate the new field
new_prop_parameters = unreal.PropParameters()
new_prop_parameters.name = args.name
new_prop_parameters.mesh = static_mesh
new_prop_parameters.size = str_to_size(args.size)

prop_id = 'static.prop.' + args.name
if prop_id in definitions_map:
    print("The prop is already present in the DefinitionsMap")
else:
    # add new field
    definitions_map[prop_id] = new_prop_parameters
    unreal.EditorAssetLibrary.save_asset(prop_factory_path, False)
    # add prop to Default.Package.json file list
    prop_config_file_path = unreal.Paths.project_content_dir() + "Carla/Config/Default.Package.json"
    json_file = open(prop_config_file_path, 'r')
    config_json = json.load(json_file)
    json_file.close()
    prop_list = config_json['props']
    prop_list.append(
        {'name' : new_prop_parameters.name,
        'path' : args.static_mesh_path,
        'size' : size_to_str(new_prop_parameters.size)})
    json_file = open(prop_config_file_path, 'w')
    json_file.write(json.dumps(config_json, indent = 4, sort_keys=False))
    json_file.close()
