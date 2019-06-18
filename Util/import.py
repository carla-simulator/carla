#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

""" Import Assets to Carla"""

import os
import fnmatch
import json
import ntpath
import subprocess

# Returns a list with the paths of each package's json files that has been found recursively in the input folder
def get_packages_json_list(folder):
    json_files = []

    for root, dirnames, filenames in os.walk(folder):
        for filename in fnmatch.filter(filenames, "*.json"):
            json_files.append([root, filename])

    return json_files

# Generic function for running a commandlet with its arguments
def invoke_commandlet(name, arguments):
    if os.name == "nt":
        sys_name = "Win64"
    elif os.name == "posix":
        sys_name = "Linux"
    ue4_path = os.environ["UE4_ROOT"]
    editor_path = "%s/Engine/Binaries/%s/UE4Editor" % (ue4_path, sys_name)
    uproject_path = os.path.join(os.getcwd(), "..", "Unreal", "CarlaUE4", "CarlaUE4.uproject")
    full_command = "%s %s -run=%s %s" % (editor_path, uproject_path, name, arguments)
    subprocess.check_call([full_command], shell=True)

# Creates the PROPS and MAPS import_setting.json file needed as an argument for using the ImportAssets commandlet
def generate_import_setting_file(package_name, json_dirname, props, maps):
    importfile = os.path.join(os.getcwd(), "importsetting.json")
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

        for carla_map in maps:
            maps_dest = "/" + "/".join(["Game", package_name, "Maps", carla_map["name"]])
            print (maps_dest)

            file_names = [os.path.join(json_dirname, carla_map["source"])]
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

# Creates the PackageName.Props.json file for the package
def generate_prop_file(package_name, props):
    output_json = {}
    output_json["props"] = []
    for prop in props:
        name = prop["name"]
        size = prop["size"]

        fbx_name = ntpath.basename(prop["source"]).replace(".fbx", "")
        path = os.path.join("Game", package_name, "Static", prop["tag"], prop["name"], fbx_name)

        output_json["props"].append({
            "name": name,
            "size": size,
            "path": path
        })

    prop_file_path = os.path.join(os.getcwd(), "..", "Unreal", "CarlaUE4", "Content", package_name, "Config")
    if not os.path.exists(prop_file_path):
        try:
            os.makedirs(prop_file_path)
        except OSError as exc:
            if exc.errno != errno.EEXIST:
                raise

    with open(os.path.join(prop_file_path, package_name + ".Props.json"), 'w+') as fh:
        json.dump(output_json, fh)

# Creates the PackageName.Package.json file for the package
def generate_package_file(package_name, props):
    output_json = {}
    output_json["maps"] = []
    output_json["props"] = []

    prop_json_dir = os.path.join(os.getcwd(), "..", "Unreal", "CarlaUE4", "Content", package_name, "Config")

    output_json["props"].append(os.path.join(prop_json_dir, package_name + ".Props.json"))

    with open(os.path.join(prop_json_dir, package_name + ".Package.json"), 'w+') as fh:
        json.dump(output_json, fh)


def import_assets(package_name, json_dirname, props, maps):
    # Same commandlet is used for importing assets and also maps
    commandlet_name = "ImportAssets"

    # Import Props
    import_setting_file = generate_import_setting_file(package_name, json_dirname, props, maps)
    commandlet_arguments = "-importSettings=\"%s\" -AllowCommandletRendering -nosourcecontrol -replaceexisting" % import_setting_file
    # invoke_commandlet(commandlet_name, commandlet_arguments)
    os.remove(import_setting_file)

    # TODO: Move maps XODR files if any

    # Create files
    generate_prop_file (package_name, props)
    generate_package_file (package_name, props)

    # Clean up


def import_assets_from_json_list(json_list):
    for dirname, filename in json_list:
        # Read json file
        with open(os.path.join(dirname, filename)) as json_file:
            data = json.load(json_file)
            # Take all the fbx registerd in the provided json files
            # and place it inside unreal in the provided path (by the json file)
            maps = data["maps"]
            props = data["props"]
            package_name = filename.replace(".json", "")

            import_assets(package_name, dirname, props, maps)


def main():
    import_folder = os.path.join(os.getcwd(), "..", "Import")
    json_list = get_packages_json_list(import_folder)
    import_assets_from_json_list(json_list)

if __name__ == '__main__':
    main()
