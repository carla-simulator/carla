#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Import Assets to Carla"""

from __future__ import print_function

from contextlib import contextmanager
import fnmatch
import json
import ntpath
import os
import shutil
import subprocess

# Global variables
IMPORT_SETTING_FILENAME = "importsetting.json"


@contextmanager
def pushd(directory):
    """Context manager to temporally change working directory."""
    cwd = os.getcwd()
    try:
        os.chdir(directory)
        yield
    finally:
        os.chdir(cwd)


def get_packages_json_list(folder):
    """Returns a list with the paths of each package's json
    files that has been found recursively in the input folder.
    """
    json_files = []

    for root, dirnames, filenames in os.walk(folder):
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
    uproject_path = os.path.join(os.getcwd(), "..", "Unreal", "CarlaUE4", "CarlaUE4.uproject")
    full_command = "%s %s -run=%s %s" % (editor_path, uproject_path, name, arguments)
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

        for carla_map in maps:
            maps_dest = "/" + "/".join(["Game", package_name, "Maps", carla_map["name"]])
            print(maps_dest)

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


def generate_package_file(package_name, props, maps):
    """Creates the PackageName.Package.json file for the package."""
    output_json = {}

    output_json["props"] = []
    for prop in props:
        name = prop["name"]
        size = prop["size"]

        fbx_name = ntpath.basename(prop["source"]).replace(".fbx", "")
        path = "/" + "/".join(["Game", package_name, "Static", prop["tag"], prop["name"]])

        output_json["props"].append({
            "name": name,
            "path": path,
            "size": size,
        })

    output_json["maps"] = []
    for map in maps:
        fbx_name = ntpath.basename(map["source"]).replace(".fbx", "")
        path = "/" + "/".join(["Game", package_name, "Maps", map["name"], fbx_name])
        use_carla_materials = map["use_carla_materials"] if "use_carla_materials" in map else False
        output_json["maps"].append({
            "name": map["name"],
            "path": path,
            "use_carla_materials": use_carla_materials
        })

    package_config_path = os.path.join(os.getcwd(), "..", "Unreal", "CarlaUE4", "Content", package_name, "Config")
    if not os.path.exists(package_config_path):
        try:
            os.makedirs(package_config_path)
        except OSError as exc:
            if exc.errno != errno.EEXISTS:
                raise

    with open(os.path.join(package_config_path, package_name + ".Package.json"), "w+") as fh:
        json.dump(output_json, fh)


def import_assets(package_name, json_dirname, props, maps):
    """Same commandlet is used for importing assets and also maps."""
    commandlet_name = "ImportAssets"

    # Import Props
    import_setting_file = generate_import_setting_file(package_name, json_dirname, props, maps)
    commandlet_arguments = "-importSettings=\"%s\" -AllowCommandletRendering -nosourcecontrol -replaceexisting" % import_setting_file
    invoke_commandlet(commandlet_name, commandlet_arguments)
    os.remove(import_setting_file)

    # Create package file
    generate_package_file(package_name, props, maps)

    # Move maps XODR files if any
    for umap in maps:
        # Make sure XODR info is full and the file exists
        if "xodr" in umap and umap["xodr"] and os.path.isfile(umap["xodr"]):
            # Make sure the xodr file have the same name than the umap
            xodr_path = umap["xodr"]
            umap_name = os.path.basename(umap["source"]).split('.')[:-1]
            xodr_name = '.'.join([umap_name, 'xodr'])

            # Get Carla's root folder
            current_script_folder = os.path.dirname(os.path.realpath(__file__))
            carla_root_path = ""
            with pushd(current_script_folder):
                os.chdir("..")
                carla_root_path = os.getcwd()

            xodr_path_destin = os.path.join(carla_root_path, "Unreal", "CarlaUE4", "Content", xodr_name)
            shutil.copy2(xodr_path, xodr_path_destin)


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

    prepare_cook_commandlet(package_name)


def prepare_cook_commandlet(package_name):
    commandlet_name = "CookAssets"
    commandlet_arguments = "-PackageName=%s" % package_name
    invoke_commandlet(commandlet_name, commandlet_arguments)


def main():
    import_folder = os.path.join(os.getcwd(), "..", "Import")
    json_list = get_packages_json_list(import_folder)
    import_assets_from_json_list(json_list)


if __name__ == '__main__':
    main()
