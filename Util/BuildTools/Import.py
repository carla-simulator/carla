#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Import Assets to Carla"""

from __future__ import print_function

import errno
import fnmatch
import json
import os
import shutil
import subprocess

# Global variables
IMPORT_SETTING_FILENAME = "importsetting.json"
SCRIPT_NAME = os.path.basename(__file__)
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
# Go two directories above the current script
CARLA_ROOT_PATH = os.path.normpath(SCRIPT_DIR + '/../..')


def get_packages_json_list(folder):
    """Returns a list with the paths of each package's json
    files that has been found recursively in the input folder.
    """
    json_files = []

    for root, _, filenames in os.walk(folder):
        for filename in fnmatch.filter(filenames, "*.json"):
            json_files.append([root, filename])

    return json_files


def invoke_commandlet(name, arguments):
    """Generic function for running a commandlet with its arguments."""
    if os.name == "nt":
        sys_name = "Win64"
    elif os.name == "posix":
        sys_name = "Linux"
    ue4_path = os.environ["UE4_ROOT"]
    editor_path = "%s/Engine/Binaries/%s/UE4Editor" % (ue4_path, sys_name)
    uproject_path = os.path.join(CARLA_ROOT_PATH, "Unreal", "CarlaUE4", "CarlaUE4.uproject")
    full_command = "%s %s -run=%s %s" % (editor_path, uproject_path, name, arguments)
    print("\n[" + str(SCRIPT_NAME) + "] Running command:\n$ " + full_command + '\n')
    subprocess.check_call([full_command], shell=True)


def generate_import_setting_file(package_name, json_dirname, props, maps):
    """Creates the PROPS and MAPS import_setting.json file needed
    as an argument for using the ImportAssets commandlet
    """
    importfile = os.path.join(os.getcwd(), IMPORT_SETTING_FILENAME)
    if os.path.exists(importfile):
        os.remove(importfile)

    with open(importfile, "w+") as fh:
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

        for prop in props:
            props_dest = "/" + "/".join(["Game", package_name, "Static", prop["tag"], prop["name"]])

            file_names = [os.path.join(json_dirname, prop["source"])]
            import_groups.append({
                "ImportSettings": import_settings,
                "FactoryName": "FbxFactory",
                "DestinationPath": props_dest,
                "bReplaceExisting": "true",
                "FileNames": file_names
            })

        for umap in maps:
            maps_dest = "/" + "/".join(["Game", package_name, "Maps", umap["name"]])

            file_names = [os.path.join(json_dirname, umap["source"])]
            import_groups.append({
                "ImportSettings": import_settings,
                "FactoryName": "FbxFactory",
                "DestinationPath": maps_dest,
                "bReplaceExisting": "true",
                "FileNames": file_names
            })

        fh.write(json.dumps({"ImportGroups": import_groups}))
        fh.close()
    return importfile


def generate_package_file(package_name, props, maps):
    """Creates the PackageName.Package.json file for the package."""
    output_json = {}

    output_json["props"] = []
    for prop in props:
        name = prop["name"]
        size = prop["size"]
        source_name = os.path.basename(prop["source"]).split('.')
        if len(source_name) < 2:
            print("[Warning] File name '" + prop["source"] + "' contains multiple dots ('.')")

        source_name = '.'.join([source_name[0], source_name[0]])

        path = "/" + "/".join(["Game", package_name, "Static", prop["tag"], prop["name"], source_name])

        output_json["props"].append({
            "name": name,
            "path": path,
            "size": size,
        })

    output_json["maps"] = []
    for umap in maps:
        path = "/" + "/".join(["Game", package_name, "Maps", umap["name"]])
        use_carla_materials = umap["use_carla_materials"] if "use_carla_materials" in umap else False
        output_json["maps"].append({
            "name": umap["name"],
            "path": path,
            "use_carla_materials": use_carla_materials
        })

    package_config_path = os.path.join(CARLA_ROOT_PATH, "Unreal", "CarlaUE4", "Content", package_name, "Config")
    if not os.path.exists(package_config_path):
        try:
            os.makedirs(package_config_path)
        except OSError as exc:
            if exc.errno != errno.EEXIST:
                raise

    with open(os.path.join(package_config_path, package_name + ".Package.json"), "w+") as fh:
        json.dump(output_json, fh, indent=4)


def import_assets(package_name, json_dirname, props, maps):
    """Same commandlet is used for importing assets and also maps."""
    commandlet_name = "ImportAssets"

    # Import Props
    import_setting_file = generate_import_setting_file(package_name, json_dirname, props, maps)
    commandlet_arguments = "-importSettings=\"%s\" -nosourcecontrol -replaceexisting" % import_setting_file
    invoke_commandlet(commandlet_name, commandlet_arguments)
    os.remove(import_setting_file)

    # Move maps XODR files if any
    for umap in maps:
        # Make sure XODR info is full and the file exists
        if "xodr" in umap and umap["xodr"] and os.path.isfile(os.path.join(json_dirname, umap["xodr"])):
            # Make sure the `.xodr` file have the same name than the `.umap`
            xodr_path = os.path.abspath(os.path.join(json_dirname, umap["xodr"]))
            umap_name = umap["name"]
            xodr_name = '.'.join([umap_name, "xodr"])

            xodr_folder_destin = os.path.join(
                CARLA_ROOT_PATH,
                "Unreal",
                "CarlaUE4",
                "Content",
                package_name,
                "Maps",
                umap_name,
                "OpenDrive")

            if not os.path.exists(xodr_folder_destin):
                os.makedirs(xodr_folder_destin)

            xodr_path_destin = os.path.join(
                xodr_folder_destin,
                xodr_name)

            print('Copying "' + xodr_path + '" to "' + xodr_path_destin + '"')
            shutil.copy2(xodr_path, xodr_path_destin)

    # Create package file
    generate_package_file(package_name, props, maps)


def import_assets_from_json_list(json_list):
    maps = []
    package_name = ""
    for dirname, filename in json_list:
        # Read json file
        with open(os.path.join(dirname, filename)) as json_file:
            data = json.load(json_file)
            # Take all the fbx registered in the provided json files
            # and place it inside unreal in the provided path (by the json file)
            maps = data["maps"]
            props = data["props"]
            package_name = filename.replace(".json", "")

            import_assets(package_name, dirname, props, maps)
            move_uassets(package_name, maps)

            if not package_name:
                print("No Packages JSONs found, nothing to import. Skipping package.")
                continue
            prepare_maps_commandlet_for_cooking(package_name)


def move_uassets(package_name, maps):
    for umap in maps:
        origin_path = os.path.join(CARLA_ROOT_PATH, "Unreal", "CarlaUE4", "Content", package_name, "Maps", umap["name"])
        dest_base_path = os.path.join(CARLA_ROOT_PATH, "Unreal", "CarlaUE4", "Content", package_name, "Static")

        # Create the 3 posible destination folder path
        marking_dir = os.path.join(dest_base_path, "MarkingNode", umap["name"])
        road_dir = os.path.join(dest_base_path, "RoadNode", umap["name"])
        terrain_dir = os.path.join(dest_base_path, "TerrainNode", umap["name"])

        # Create folders if they do not exist
        if not os.path.exists(marking_dir):
            os.makedirs(marking_dir)
        if not os.path.exists(road_dir):
            os.makedirs(road_dir)
        if not os.path.exists(terrain_dir):
            os.makedirs(terrain_dir)

        # Move uassets to corresponding folder
        for filename in os.listdir(origin_path):
            if "MarkingNode" in filename:
                shutil.move(os.path.join(origin_path, filename), os.path.join(marking_dir, filename))
            if "RoadNode" in filename:
                shutil.move(os.path.join(origin_path, filename), os.path.join(road_dir, filename))
            if "TerrainNode" in filename:
                shutil.move(os.path.join(origin_path, filename), os.path.join(terrain_dir, filename))

def prepare_maps_commandlet_for_cooking(package_name):
    commandlet_name = "PrepareAssetsForCooking"
    commandlet_arguments = "-PackageName=%s" % package_name
    commandlet_arguments += " -OnlyPrepareMaps=true"
    invoke_commandlet(commandlet_name, commandlet_arguments)

def main():
    import_folder = os.path.join(CARLA_ROOT_PATH, "Import")
    json_list = get_packages_json_list(import_folder)
    import_assets_from_json_list(json_list)


if __name__ == '__main__':
    main()
