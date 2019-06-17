#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Bulk Import FBX"""

import os
import json
import subprocess
import shutil
import argparse
import re


if os.name == 'nt':
    sys_name = 'Win64'
elif os.name == 'posix':
    sys_name = 'Linux'


def main():
    try:
        args = parse_arguments()
        folder_list = []
        import_all_fbx_in_folder(args.folder, folder_list)
    finally:
        print('\ndone.')


def import_all_fbx_in_folder(fbx_folder, folder_list):
    dirname = os.getcwd()
    fbx_place = os.path.join(dirname, "..", fbx_folder)
    for file in os.listdir(fbx_place):
        if file.endswith(".PropRegistry.json"):
            registry_name = file.replace(".PropRegistry.json", "")
            with open(os.path.join(dirname, "..", fbx_folder, file)) as json_file:
                data = json.load(json_file)
                # This will take all the fbx registerd in the provided json files
                # and place it inside unreal in the provided path (by the json file)
                import_assets_commandlet(data, fbx_folder, registry_name, folder_list)
    # This part will just gather all the folders in which an fbx has been placed
    if(len(folder_list) > 0):
        final_list = ""
        for folder in folder_list:
            if folder not in final_list:
                final_list += folder + "+"
        final_list = final_list[:-1]
        # Destination map (the one that will be cooked)
        dest_map_path = "/Game/Carla/Maps/TestMaps"
        dest_map_name = "TEMPMAP"
        # This should be a folder, because the commandlet will take anything inside.
        # It is better if there is only one map inside
        src_map_folder = "/Game/Carla/Maps/TestMaps"
        prepare_cook_commandlet(final_list, src_map_folder, dest_map_path, dest_map_name)
        launch_bash_script("BuildTools/ExportFBX.sh", "--maps=%s/%s" % (dest_map_path, dest_map_name))


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


def prepare_cook_commandlet(folder_list, source_map, dest_map_path, dest_map_name):
    commandlet_name = "FBXExporterPreparator"
    commandlet_arguments = "-MeshesDir=%s -SourceMap=%s -DestMapPath=%s -DestMapName=%s" % (
        folder_list, source_map, dest_map_path, dest_map_name)
    invoke_commandlet(commandlet_name, commandlet_arguments)


def import_assets_commandlet(json_data, fbx_folder, registry_name, folder_list):
    importfile = "importsetting.json"
    if os.path.exists(importfile):
        os.remove(importfile)
    populate_json_and_data(json_data, fbx_folder, importfile, registry_name, folder_list)
    generate_prop_registry_file_for_unreal(json_data, registry_name)
    dirname = os.getcwd()
    commandlet_name = "ImportAssets"
    import_settings = os.path.join(dirname, importfile)
    commandlet_arguments = "-importSettings=\"%s\" -AllowCommandletRendering -nosourcecontrol -replaceexisting" % import_settings
    invoke_commandlet(commandlet_name, commandlet_arguments)
    # Clean up
    os.remove(importfile)


def invoke_commandlet(name, arguments):
    ue4_path = os.environ['UE4_ROOT']
    dirname = os.getcwd()
    editor_path = "%s/Engine/Binaries/%s/UE4Editor" % (ue4_path, sys_name)
    uproject_path = os.path.join(dirname, "..", "Unreal", "CarlaUE4", "CarlaUE4.uproject")
    full_command = "%s %s -run=%s %s" % (editor_path, uproject_path, name, arguments)
    subprocess.check_call([full_command], shell=True)


def launch_bash_script(script_path, arguments):
    dirname = os.getcwd()
    full_command = "%s %s" % (os.path.join(dirname, script_path), arguments)
    print("Executing: " + full_command)
    subprocess.check_call([full_command], shell=True)


def populate_json_and_data(json_data, fbx_folder, json_file, registry_name, folder_list):
    with open(json_file, "w+") as fh:
        import_groups = []
        file_names = []
        import_settings = []
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
            destination_path = "/Game/" + registry_name + "/Static/" + prop["tag"] + "/" + prop["name"]

            file_names = []
            fbx_path = os.path.join("..", "..", fbx_folder, "%s" % prop["source"])
            file_names.append(fbx_path)
            import_groups.append({
                "ImportSettings": import_settings,
                "FactoryName": "FbxFactory",
                "DestinationPath": destination_path,
                "bReplaceExisting": "true",
                "FileNames": file_names
            })
            folder_list.append(destination_path)
        fh.write(json.dumps({"ImportGroups": import_groups}))
        fh.close()


def generate_prop_registry_file_for_unreal(json_data, registry_name):
    data = {}
    data["definitions"] = []
    for prop in json_data['definitions']:
        name = prop["name"]
        size = prop["size"]

        fbx_name = prop["source"].replace(".fbx", "")
        path = "/Game/" + registry_name + "/Static/" + prop["tag"] + "/" + prop["name"] + "/" + fbx_name

        data['definitions'].append({
            "name": name,
            "size": size,
            "path": path
        })

    registry_path = os.path.join("../Unreal/CarlaUE4/Content/" + registry_name + "/Config/")
    if not os.path.exists(registry_path):
        try:
            os.makedirs(registry_path)
        except OSError as exc:
            if exc.errno != errno.EEXIST:
                raise

    with open(registry_path + registry_name + "_generated" + ".PropRegistry.json", 'w+') as fh:
        json.dump(data, fh)


if __name__ == '__main__':

    main()
