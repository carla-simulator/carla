#!/usr/bin/env python

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Generate map from FBX"""

import os
import sys
import json
import subprocess
import glob
import shutil
import argparse


if os.name == 'nt':
    sys_name = 'Win64'
elif os.name == 'posix':
    sys_name = 'Linux'


def main():
    if(args.force):
        generate_all_maps_but_list([])
    else:
        maps = get_map_names()
        generate_all_maps_but_list(maps)


def get_map_names():
    maps = []
    dirname = os.getcwd()
    map_place = os.path.join(dirname, "..", "Unreal", "CarlaUE4", "Content", "Carla", "Maps")
    for filename in os.listdir(map_place):
        if filename.endswith('.umap'):
            maps.append(filename)
    return maps

def generate_all_maps_but_list(existent_maps):
    map_name = ""
    dirname = os.getcwd()
    fbx_place = os.path.join(dirname, "..", "RoadRunnerFiles")
    for x in os.walk(fbx_place):
            map_name = os.path.basename(x[0])
            if map_name != "RoadRunnerFiles":
                if not any(ext in "%s.umap" % map_name for ext in existent_maps):
                    print("Found map in fbx folder: %s" % map_name)
                    import_assets_commandlet(map_name)
                    #move_uassets(map_name)
                    print("Generating map asset for %s" % map_name)
                    generate_map(map_name)
                    print("Cleaning up directories")
                    cleanup_assets(map_name)
                    print("Finished %s" % map_name)
                else:
                    print("Found %s map in Content folder, skipping. Use \"--force\" to override" % map_name)

def parse_arguments():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '-f', '--force',
        action='store_true',
        help='Force import. Will override maps with the same name')
    argparser.add_argument(
        '-m', '--map',
        metavar='M',
        type=str,
        help='Map to import. If empty, all maps in the folder will be loaded')
    argparser.add_argument(
        '--usecarlamats',
        action='store_true',
        help='Avoid using RoadRunner materials. Use materials provided by Carla instead')
    return argparser.parse_args()


def cleanup_assets(map_name):
    dirname = os.getcwd()
    content_folder = os.path.join(dirname, "..", "Unreal", "CarlaUE4" , "Content", "Carla")
    origin_folder = os.path.join(content_folder, "Static", "Imported", map_name)
    for filename in os.listdir(origin_folder):
        if map_name in filename:
            removal_path = os.path.join(origin_folder, filename)
            os.remove(removal_path)


def import_assets_commandlet(map_name):
    generate_json(map_name, "importsetting.json")
    dirname = os.getcwd()
    commandlet_name = "ImportAssets"
    import_settings = os.path.join(dirname, "importsetting.json")
    commandlet_arguments = "-importSettings=\"%s\" -AllowCommandletRendering -nosourcecontrol -replaceexisting" % import_settings

    file_xodr_origin = os.path.join(dirname, "..", "RoadRunnerFiles", map_name, "%s.xodr" % map_name)
    file_xodr_dest = os.path.join(dirname, "..", "Unreal", "CarlaUE4", "Content", "Carla", "Maps", "OpenDrive", "%s.xodr" % map_name)

    shutil.copy2(file_xodr_origin, file_xodr_dest)
    invoke_commandlet(commandlet_name, commandlet_arguments)
    #Clean up
    os.remove("importsetting.json")

def generate_map(map_name):
    dirname = os.getcwd()
    commandlet_name = "MapProcess"
    commandlet_arguments = "-mapname=\"%s\"" % map_name
    if args.usecarlamats:
        commandlet_arguments += " -use-carla-materials"
    invoke_commandlet(commandlet_name, commandlet_arguments)

#This line might be needed if Epic tells us anything about the current way of doing the movement. It shouldn't but just in case...
def move_uassets(map_name):
    dirname = os.getcwd()
    content_folder = os.path.join(dirname, "..", "Unreal", "CarlaUE4" , "Content", "Carla")
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


def generate_json(map_name, json_file):
    fh = open("importsetting.json", "a+")
    import_groups = []
    file_names = []
    import_settings = []
    fbx_path = os.path.join("..", "..", "RoadRunnerFiles", map_name, "%s.fbx" % map_name)
    file_names.append(fbx_path)

    import_settings.append({
      "bImportMesh": 1,
      "bConvertSceneUnit": 1,
      "bConvertScene": 1,
      "bCombineMeshes": 1,
      "bImportTextures": 1,
      "bImportMaterials": 1,
      "bRemoveDegenerates":1,
      "bCombineMeshes":0,
      "AnimSequenceImportData": {},
      "SkeletalMeshImportData": {},
      "TextureImportData": {},
      "StaticMeshImportData": {
          "bRemoveDegenerates": 1,
          "bAutoGenerateCollision": 0,
          "bRemoveDegenerates":1,
          "bCombineMeshes":0
        }
      })
    dest_path = "/Game/Carla/Static/Imported/%s" % map_name
    import_groups.append({
    "ImportSettings": import_settings,
    "FactoryName": "FbxFactory",
    "DestinationPath": dest_path,
    "bReplaceExisting": "true",
    "FileNames": file_names
    })
    fh.write(json.dumps({"ImportGroups": import_groups}))
    fh.close()

args = parse_arguments()

if __name__ == '__main__':

    try:
        main()
    finally:
        print('\ndone.')