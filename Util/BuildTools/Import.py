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
import argparse

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

def generate_json_package(folder, package_name, use_carla_materials):
    """Generate a .json file with all the maps it founds on the folder
    and subfolders. A map is a .fbx and a .xodr with the same name.
    """
    json_files = []

    # search for all .fbx and .xodr pair of files
    maps = []
    for root, _, filenames in os.walk(folder):
        files = fnmatch.filter(filenames, "*.fbx")
        for file_name in files:
            fbx = file_name[:-4]
            # check if exist the .xodr file
            if os.path.exists("%s/%s.xodr" % (root, fbx)):
                maps.append([os.path.relpath(root, folder), fbx])

    # write the json
    if (len(maps) > 0):
        # build all the maps in .json format
        json_maps = []
        for map_name in maps:
            path = map_name[0].replace('\\', '/')
            name = map_name[1]
            json_maps.append({
                'name': name, 
                'source': '%s/%s.fbx'  % (path, name), 
                'xodr':   '%s/%s.xodr' % (path, name), 
                'use_carla_materials': use_carla_materials
                })
        # build and write the .json
        f = open("%s/%s.json" % (folder, package_name), "w")
        my_json = {'maps': json_maps, 'props': []}
        serialized = json.dumps(my_json, sort_keys=False, indent=3)
        f.write(serialized)
        f.close()
        # add
        json_files.append([folder, "%s.json" % package_name])

    return json_files


def invoke_commandlet(name, arguments):
    """Generic function for running a commandlet with its arguments."""
    ue4_path = os.environ["UE4_ROOT"]
    uproject_path = os.path.join(CARLA_ROOT_PATH, "Unreal", "CarlaUE4", "CarlaUE4.uproject")
    run = "-run=%s" % (name)

    if os.name == "nt":
        sys_name = "Win64"
        editor_path = "%s/Engine/Binaries/%s/UE4Editor" % (ue4_path, sys_name)
        command = [editor_path, uproject_path, run]
        command.extend(arguments)
        print("Commandlet:", command)
        subprocess.check_call(command, shell=True)
    elif os.name == "posix":
        sys_name = "Linux"
        editor_path = "%s/Engine/Binaries/%s/UE4Editor" % (ue4_path, sys_name)
        full_command = "%s %s %s %s" % (editor_path, uproject_path, run, " ".join(arguments))
        print("Commandlet:", full_command)
        subprocess.call([full_command], shell=True)




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
        import_settings = {
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
                "bCombineMeshes": 0,
                "bConvertSceneUnit": 1
            }
        }

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
    commandlet_arguments = ["-importSettings=\"%s\"" % import_setting_file, "-nosourcecontrol", "-replaceexisting"]
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
            maps = []
            props = []
            if "maps" in data:
                maps = data["maps"]
            if "props" in data:
                props = data["props"]

            package_name = filename.replace(".json", "")

            import_assets(package_name, dirname, props, maps)

            if not package_name:
                print("No Packages JSONs found, nothing to import. Skipping package.")
                continue

            # First we only move the meshes to the tagged folders for semantic
            # segmentation
            move_assets_commandlet(package_name, maps)

            # we need to build the binary file for navigation of pedestrians
            build_binary_for_navigation(package_name, dirname, maps)

            # We prepare only the maps for cooking after moving them. Props cooking will be done from Package.sh script.
            prepare_maps_commandlet_for_cooking(package_name, only_prepare_maps=True)


def prepare_maps_commandlet_for_cooking(package_name, only_prepare_maps):
    commandlet_name = "PrepareAssetsForCooking"
    commandlet_arguments = ["-PackageName=%s" % package_name]
    commandlet_arguments.append("-OnlyPrepareMaps=%d" % only_prepare_maps)
    invoke_commandlet(commandlet_name, commandlet_arguments)


def move_assets_commandlet(package_name, maps):
    commandlet_name = "MoveAssets"
    commandlet_arguments = ["-PackageName=%s" % package_name]

    umap_names = ""
    for umap in maps:
        umap_names += umap["name"] + " "
    commandlet_arguments.append("-Maps=%s" % umap_names)

    invoke_commandlet(commandlet_name, commandlet_arguments)

# build the binary file for navigation of pedestrians for that map
def build_binary_for_navigation(package_name, dirname, maps):
    folder = os.path.join(CARLA_ROOT_PATH, "Util", "DockerUtils", "dist")

    # process each map
    for umap in maps:

        # get the target name
        target_name = umap["name"]

        # copy the XODR file into docker utils folder
        if "xodr" in umap and umap["xodr"] and os.path.isfile(os.path.join(dirname, umap["xodr"])):
            # Make sure the `.xodr` file have the same name than the `.umap`
            xodr_path_source = os.path.abspath(os.path.join(dirname, umap["xodr"]))
            xodr_name = '.'.join([target_name, "xodr"])
            xodr_path_target = os.path.join(folder, xodr_name)
            # copy
            print('Copying "' + xodr_path_source + '" to "' + xodr_path_target + '"')
            shutil.copy2(xodr_path_source, xodr_path_target)

        # copy the FBX file into docker utils folder
        if "source" in umap and umap["source"] and os.path.isfile(os.path.join(dirname, umap["source"])):
            # Make sure the `.fbx` file have the same name than the `.umap`
            fbx_path_source = os.path.abspath(os.path.join(dirname, umap["source"]))
            fbx_name = '.'.join([target_name, "fbx"])
            fbx_path_target = os.path.join(folder, fbx_name)
            # copy
            print('Copying "' + fbx_path_source + '" to "' + fbx_path_target + '"')
            shutil.copy2(fbx_path_source, fbx_path_target)

        # make the conversion
        if os.name == "nt":
            subprocess.call(["build.bat", target_name], cwd=folder, shell=True)
        else:
            subprocess.call(["chmod +x build.sh"], cwd=folder, shell=True)
            subprocess.call("./build.sh %s" % target_name, cwd=folder, shell=True)

        # copy the binary file
        nav_folder_target = os.path.join(
            CARLA_ROOT_PATH,
            "Unreal",
            "CarlaUE4",
            "Content",
            package_name,
            "Maps",
            target_name,
            "Nav")

        nav_path_source = os.path.join(folder, "%s.bin" % target_name)
        if os.path.exists(nav_path_source):
            # Skip this step for maps that do not use ped navmesh
            if not os.path.exists(nav_folder_target):
                os.makedirs(nav_folder_target)
            nav_path_target = os.path.join(nav_folder_target, "%s.bin" % target_name)
            print('Copying "' + nav_path_source + '" to "' + nav_path_target + '"')
            shutil.copy2(nav_path_source, nav_path_target)

        # remove files
        os.remove(fbx_path_target)
        os.remove(xodr_path_target)

def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '--package',
        metavar='P',
        default='map_package',
        help='Name of the imported package')
    argparser.add_argument(
        '--no-carla-materials',
        action='store_false',
        help='user Carla materials')
    args = argparser.parse_known_args()[0]

    import_folder = os.path.join(CARLA_ROOT_PATH, "Import")
    json_list = get_packages_json_list(import_folder)
    if (len(json_list) == 0):
        json_list = generate_json_package(import_folder, args.package, args.no_carla_materials)
    import_assets_from_json_list(json_list)

if __name__ == '__main__':
    main()
