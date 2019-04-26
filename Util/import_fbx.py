#!/usr/bin/env python

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Generate map from FBX"""

import os
import json
import subprocess
import shutil
import argparse


if os.name == 'nt':
    sys_name = 'Win64'
elif os.name == 'posix':
    sys_name = 'Linux'


def main():
    try:
        args = parse_arguments()
        import_all_fbx_in_folder(args.folder)
    finally:
        print('\ndone.')


def import_all_fbx_in_folder(fbx_folder):
    dirname = os.getcwd()
    fbx_place = os.path.join(dirname, "..", fbx_folder)
    for file in os.listdir(fbx_place):
        if file.endswith(".PropRegistry.json"):
            with open(os.path.join(dirname, "..", fbx_folder, file)) as json_file:
                data = json.load(json_file)
                for prop in data['definitions']:
                    print("Name: " + prop["name"])
                    print("Size: " + prop["size"])
                    print("Source: " + prop["source"])
                    print("Path: " + prop["path"])
                    #import_assets_commandlet(prop["source"], fbx_folder, prop["path"])
                import_assets_commandlet(data, fbx_folder)


def parse_arguments():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '-f', '--folder',
        metavar='F',
        type=str,
        default="FBXImporter",
        help='FBX containing folder')
    return argparser.parse_args()


def cleanup_assets(map_name):
    dirname = os.getcwd()
    content_folder = os.path.join(dirname, "..", "Unreal", "CarlaUE4", "Content", "Carla")
    origin_folder = os.path.join(content_folder, "Static", "Imported", map_name)
    for filename in os.listdir(origin_folder):
        if map_name in filename:
            removal_path = os.path.join(origin_folder, filename)
            os.remove(removal_path)


def import_assets_commandlet(json_data, fbx_folder):
    importfile = "importsetting.json"
    if os.path.exists(importfile):
        os.remove(importfile)
    generate_json(json_data, fbx_folder, importfile)
    dirname = os.getcwd()
    commandlet_name = "ImportAssets"
    import_settings = os.path.join(dirname, importfile)
    commandlet_arguments = "-importSettings=\"%s\" -AllowCommandletRendering -nosourcecontrol -replaceexisting" % import_settings
    invoke_commandlet(commandlet_name, commandlet_arguments)
    # Clean up
    os.remove(importfile)


def generate_map(map_name, args):
    commandlet_name = "MapProcess"
    commandlet_arguments = "-mapname=\"%s\"" % map_name
    if args.usecarlamats:
        commandlet_arguments += " -use-carla-materials"
    invoke_commandlet(commandlet_name, commandlet_arguments)
# This line might be needed if Epic tells us anything about the current
# way of doing the movement. It shouldn't but just in case...


def move_uassets(map_name):
    dirname = os.getcwd()
    content_folder = os.path.join(dirname, "..", "Unreal", "CarlaUE4", "Content", "Carla")
    origin_folder = os.path.join(content_folder, "Static", map_name)
    dest_path = ""
    src_path = ""
    marking_dir = os.path.join(content_folder, "Static", "RoadLines", "%sLaneMarking" % map_name)
    road_dir = os.path.join(content_folder, "Static", "Road", "Roads%s" % map_name)
    terrain_dir = os.path.join(content_folder, "Static", "Terrain", "%sTerrain" % map_name)
    if not os.path.exists(marking_dir):
        os.makedirs(marking_dir)
    if not os.path.exists(road_dir):
        os.makedirs(road_dir)
    if not os.path.exists(terrain_dir):
        os.makedirs(terrain_dir)
    for filename in os.listdir(origin_folder):
        if "MarkingNode" in filename:
            dest_path = os.path.join(marking_dir, filename)
        if "RoadNode" in filename:
            dest_path = os.path.join(road_dir, filename)
        if "TerrainNode" in filename:
            dest_path = os.path.join(terrain_dir, filename)
        src_path = os.path.join(content_folder, "Static", map_name, filename)
        os.rename(src_path, dest_path)


def invoke_commandlet(name, arguments):
    ue4_path = os.environ['UE4_ROOT']
    dirname = os.getcwd()
    editor_path = "%s/Engine/Binaries/%s/UE4Editor" % (ue4_path, sys_name)
    uproject_path = os.path.join(dirname, "..", "Unreal", "CarlaUE4", "CarlaUE4.uproject")
    full_command = "%s %s -run=%s %s" % (editor_path, uproject_path, name, arguments)
    subprocess.check_call([full_command], shell=True)


def generate_json(json_data, fbx_folder, json_file):
    with open(json_file, "w+") as fh:
        import_groups = []
        file_names = []
        import_settings = []
        #print("Name: " + prop["name"])
        #print("Size: " + prop["size"])
        #print("Source: " + prop["source"])
        #print("Path: " + prop["path"])
        import_settings.append({
            "bImportMesh": 1,
            "bConvertSceneUnit": 1,
            "bConvertScene": 1,
            "bCombineMeshes": 1,
            "bImportTextures": 1,
            "bImportMaterials": 1,
            "bRemoveDegenerates": 1,
            "AnimSequenceImportData": {},
            "SkeletalMeshImportData": {},
            "TextureImportData": {},
            "StaticMeshImportData": {
                "bRemoveDegenerates": 1,
                "bAutoGenerateCollision": 0,
                "bCombineMeshes": 0
            }
        })

        for prop in json_data['definitions']:
            file_names = []
            fbx_path = os.path.join("..", "..", fbx_folder, "%s" % prop["source"])
            file_names.append(fbx_path)
            import_groups.append({
                "ImportSettings": import_settings,
                "FactoryName": "FbxFactory",
                "DestinationPath": os.path.dirname(prop["path"]),
                "bReplaceExisting": "true",
                "FileNames": file_names
            })
        fh.write(json.dumps({"ImportGroups": import_groups}))
        fh.close()


if __name__ == '__main__':

    main()
