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
import threading
import copy

# Global variables
IMPORT_SETTING_FILENAME = "importsetting.json"
SCRIPT_NAME = os.path.basename(__file__)
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
# Go two directories above the current script
CARLA_ROOT_PATH = os.path.normpath(SCRIPT_DIR + '/../..')

import carla


def get_packages_json_list(folder):
    """Returns a list with the paths of each package's json
    files that has been found recursively in the input folder.
    """
    json_files = []

    for root, _, filenames in os.walk(folder):
        for filename in fnmatch.filter(filenames, "*.json"):
            if filename != "roadpainter_decals.json":
                json_files.append([root, filename])

    return json_files

def get_decals_json_file(folder):

    for root, _, filenames in os.walk(folder):
        for filename in fnmatch.filter(filenames, "roadpainter_decals.json"):
            return filename

    return ""

def generate_json_package(folder, package_name, use_carla_materials):
    """Generate a .json file with all the maps it founds on the folder
    and subfolders. A map is a .fbx and a .xodr with the same name.
    """
    json_files = []

    # search for all .fbx and .xodr pair of files
    maps = []
    for root, _, filenames in os.walk(folder):
        files = fnmatch.filter(filenames, "*.xodr")
        for file_name in files:
            xodr = file_name[:-5]
            # check if exist the .fbx file
            if os.path.exists("%s/%s.fbx" % (root, xodr)):
                maps.append([os.path.relpath(root, folder), xodr, ["%s.fbx" % xodr]])
            else:
                # check if exist the map by tiles
                tiles = fnmatch.filter(filenames, "*_Tile_*.fbx")
                if (len(tiles) > 0):
                    maps.append([os.path.relpath(root, folder), xodr, tiles])

    # write the json
    if (len(maps) > 0):
        # build all the maps in .json format
        json_maps = []
        for map_name in maps:
            path = map_name[0].replace('\\', '/')
            name = map_name[1]
            tiles = map_name[2]
            tiles = ["%s/%s" % (path, x) for x in tiles]
            map_dict = {
                'name': name,
                'xodr':   '%s/%s.xodr' % (path, name),
                'use_carla_materials': use_carla_materials
            }
            # check for only one 'source' or map in 'tiles'
            if (len(tiles) == 1):
                map_dict['source'] = tiles[0]
            else:
                map_dict['tile_size'] = 2000
                map_dict['tiles'] = tiles

            # write
            json_maps.append(map_dict)
        # build and write the .json
        f = open("%s/%s.json" % (folder, package_name), "w")
        my_json = {'maps': json_maps, 'props': []}
        serialized = json.dumps(my_json, sort_keys=False, indent=3)
        f.write(serialized)
        f.close()
        # add
        json_files.append([folder, "%s.json" % package_name])

    return json_files

def generate_decals_file(folder):

    # search for all .fbx and .xodr pair of files
    maps = []
    for root, _, filenames in os.walk(folder):
        files = fnmatch.filter(filenames, "*.xodr")
        for file_name in files:
            xodr = file_name[:-5]
            # check if exist the .fbx file
            if os.path.exists("%s/%s.fbx" % (root, xodr)):
                maps.append([os.path.relpath(root, folder), xodr, ["%s.fbx" % xodr]])
            else:
                # check if exist the map by tiles
                tiles = fnmatch.filter(filenames, "*_Tile_*.fbx")
                if (len(tiles) > 0):
                    maps.append([os.path.relpath(root, folder), xodr, tiles])

    if (len(maps) > 0):
        # build all the maps in .json format
        json_decals = []
        for map_name in maps:

            name = map_name[1]

            #create the decals default config file
            json_decals.append({
                'map_name' : name,
                'drip1': '10',
                'drip3': '10',
                'dirt1': '10',
                'dirt3' : '10',
                'dirt4' : '10',
                'dirt5': '10',
                'roadline1': '20',
                'roadline5': '20',
                'tiremark1': '20',
                'tiremark3': '20',
                'tarsnake1': '10',
                'tarsnake3': '20',
                'tarsnake4': '10',
                'tarsnake5': '20',
                'tarsnake11': '20',
                'cracksbig1': '10',
                'cracksbig3': '10',
                'cracksbig5': '10',
                'cracksbig8': '10',
                'mud1' : '10',
                'mud5' : '10',
                'oilsplat1' : '20',
                'oilsplat2' : '20',
                'oilsplat3' : '20',
                'oilsplat4' : '20',
                'oilsplat5' : '20',
                'gum' : '30',
                'crack1': '10',
                'crack3' : '10',
                'crack4' : '10',
                'crack5' : '10',
                'crack8': '10',
                'decal_scale' : {
                'x_axis' : '1.0',
                'y_axis' : '1.0',
                'z_axis' : '1.0'},
                'fixed_decal_offset': {
                'x_axis' : '15.0',
                'y_axis' : '15.0',
                'z_axis' : '0.0'},
                'decal_min_scale' : '0.3',
                'decal_max_scale' : '0.7',
                'decal_random_yaw' : '360.0',
                'random_offset' : '50.0'
            });

        # build and write the .json
        f = open("%s/%s.json" % (folder, 'roadpainter_decals'), "w")
        my_json = {'decals': json_decals}
        serialized = json.dumps(my_json, sort_keys=False, indent=3)
        f.write(serialized)
        f.close()

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


def generate_import_setting_file(package_name, json_dirname, props, maps, do_tiles, tile_size):
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
                "bAutoGenerateCollision": 1,
                "bCombineMeshes": 0,
                "bConvertSceneUnit": 1,
                "bForceVerticesRelativeToTile": do_tiles,
                "TileSize": tile_size
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

            if "source" in umap:
                tiles = [os.path.join(json_dirname, umap["source"])]
            else:
                tiles = ["%s" % (os.path.join(json_dirname, x)) for x in umap["tiles"]]
            import_groups.append({
                "ImportSettings": import_settings,
                "FactoryName": "FbxFactory",
                "DestinationPath": maps_dest,
                "bReplaceExisting": "true",
                "FileNames": tiles
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


def copy_roadpainter_config_files(package_name):
    """Copies roadpainter configuration files into Unreal content folder"""

    two_directories_up = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
    final_path = os.path.join(two_directories_up, "Import", "roadpainter_decals.json")
    if os.path.exists(final_path):
        package_config_path = os.path.join(CARLA_ROOT_PATH, "Unreal", "CarlaUE4", "Content", package_name, "Config")
        if not os.path.exists(package_config_path):
            try:
                os.makedirs(package_config_path)
            except OSError as exc:
                if exc.errno != errno.EEXIST:
                    raise
        shutil.copy(final_path, package_config_path)


def import_assets(package_name, json_dirname, props, maps, do_tiles, tile_size, batch_size):
    """Same commandlet is used for importing assets and also maps."""
    commandlet_name = "ImportAssets"

    if do_tiles:
        for umap in maps:
            # import groups of tiles to prevent unreal from using too much memory
            map_template = {}
            for key, value in iter(umap.items()):
                if key is not 'tiles':
                    map_template[key] = value
            map_template['tiles'] = []
            tiles = umap['tiles']
            tiles.sort()
            total_tiles = len(tiles)
            num_batches = int(total_tiles / batch_size)
            current_tile = 0
            current_batch = 0
            current_batch_size = 0
            current_batch_map = copy.deepcopy(map_template)
            # get groups of tiles
            while current_tile < total_tiles:
                current_batch_map['tiles'].append(tiles[current_tile])
                file_path = os.path.join(json_dirname, tiles[current_tile])
                current_batch_size += os.path.getsize(file_path)/1000000.0
                current_tile += 1
                current_batch += 1
                # import when the size of the group of tiles surpasses the specified size in MB
                if current_batch_size >= batch_size:
                    import_setting_file = generate_import_setting_file(package_name, json_dirname, props, [current_batch_map], do_tiles, tile_size)
                    commandlet_arguments = ["-importSettings=\"%s\"" % import_setting_file, "-nosourcecontrol", "-replaceexisting"]
                    invoke_commandlet(commandlet_name, commandlet_arguments)
                    os.remove(import_setting_file)
                    current_batch_map = copy.deepcopy(map_template)
                    current_batch = 0
                    current_batch_size = 0
            # import remaining tiles
            if current_batch > 0:
                import_setting_file = generate_import_setting_file(package_name, json_dirname, props, [current_batch_map], do_tiles, tile_size)
                commandlet_arguments = ["-importSettings=\"%s\"" % import_setting_file, "-nosourcecontrol", "-replaceexisting"]
                invoke_commandlet(commandlet_name, commandlet_arguments)
                os.remove(import_setting_file)
    else:
        # Import Props
        import_setting_file = generate_import_setting_file(package_name, json_dirname, props, maps, do_tiles, tile_size)
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


def import_assets_from_json_list(json_list, batch_size):
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
            tile_size = 2000
            if "maps" in data:
                maps = data["maps"]
                if len(maps) > 0 and "tile_size" in maps[0]:
                    tile_size = maps[0]["tile_size"]
            if "props" in data:
                props = data["props"]

            package_name = filename.replace(".json", "")

            # we need to build the binary file for navigation of pedestrians
            thr = threading.Thread(target=build_binary_for_navigation, args=(package_name, dirname, maps,))
            thr.start()

            if (len(maps) > 0 and "tiles" in maps[0]):
                import_assets(package_name, dirname, props, maps, 1, tile_size, batch_size)
            else:
                import_assets(package_name, dirname, props, maps, 0, 0, 0)

            if not package_name:
                print("No Packages JSONs found, nothing to import. Skipping package.")
                continue

            # First we only move the meshes to the tagged folders for semantic segmentation
            move_assets_commandlet(package_name, maps)

            # We prepare only the maps for cooking after moving them. Props cooking will be done from Package.sh script.
            if len(maps) > 0:
                prepare_maps_commandlet_for_cooking(package_name, only_prepare_maps=True)
                load_asset_materials_commandlet(package_name)
                build_binary_for_tm(package_name, dirname, maps)
            thr.join()


def load_asset_materials_commandlet(package_name):
    commandlet_name = "LoadAssetMaterials"
    commandlet_arguments = ["-PackageName=%s" % package_name]
    invoke_commandlet(commandlet_name, commandlet_arguments)

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

        # get the sources for the map (single or tiles)
        if ("source" in umap):
            tiles = [umap["source"]]
        # disabled until we have a new Recast adapted to work with tiles
        # elif ("tiles" in umap):
        #     tiles = umap["tiles"]
        else:
            continue

        # get the target name
        target_name = umap["name"]
        xodr_filename = os.path.basename(umap["xodr"])
        xodr_path_target = ""

        # copy the XODR file into docker utils folder
        if "xodr" in umap and umap["xodr"] and os.path.isfile(os.path.join(dirname, umap["xodr"])):
            # Make sure the `.xodr` file have the same name than the `.umap`
            xodr_path_source = os.path.abspath(os.path.join(dirname, umap["xodr"]))
            xodr_path_target = os.path.join(folder, xodr_filename)
            # copy
            print('Copying "' + xodr_path_source + '" to "' + xodr_path_target + '"')
            shutil.copy2(xodr_path_source, xodr_path_target)

        for tile in tiles:

            fbx_filename = os.path.basename(tile)
            fbx_name_no_ext = os.path.splitext(fbx_filename)[0]
            fbx_path_target = ""

            # copy the FBX file into docker utils folder
            if os.path.isfile(os.path.join(dirname, tile)):
                # Make sure the `.fbx` file have the same name than the `.umap`
                fbx_path_source = os.path.abspath(os.path.join(dirname, tile))
                fbx_path_target = os.path.join(folder, fbx_filename)
                # copy
                print('Copying "' + fbx_path_source + '" to "' + fbx_path_target + '"')
                shutil.copy2(fbx_path_source, fbx_path_target)

            # rename the xodr with the same name of the source/tile
            # os.rename(os.path.join(folder, xodr_filename), os.path.join(folder, "%s.xodr" % fbx_name_no_ext))

            # make the conversion
            if os.name == "nt":
                subprocess.call(["build.bat", fbx_name_no_ext, xodr_filename], cwd=folder, shell=True)
            else:
                subprocess.call(["chmod +x build.sh"], cwd=folder, shell=True)
                subprocess.call("./build.sh %s %s" % (fbx_name_no_ext, xodr_filename), cwd=folder, shell=True)

            # rename the xodr with the original name
            # os.rename(os.path.join(folder, "%s.xodr" % fbx_name_no_ext), os.path.join(folder, xodr_filename))

            # copy the binary file
            nav_path_source = os.path.join(folder, "%s.bin" % fbx_name_no_ext)
            nav_folder_target = os.path.join(CARLA_ROOT_PATH, "Unreal", "CarlaUE4", "Content", package_name, "Maps", target_name, "Nav")
            if os.path.exists(nav_path_source):
                if not os.path.exists(nav_folder_target):
                    os.makedirs(nav_folder_target)
                nav_path_target = os.path.join(nav_folder_target, "%s.bin" % fbx_name_no_ext)
                print('Copying "' + nav_path_source + '" to "' + nav_path_target + '"')
                shutil.copy2(nav_path_source, nav_path_target)

            # remove files
            if os.path.exists(nav_path_source):
                os.remove(nav_path_source)

            if os.path.exists(fbx_path_target):
                os.remove(fbx_path_target)

        if os.path.exists(xodr_path_target):
            os.remove(xodr_path_target)


def build_binary_for_tm(package_name, dirname, maps):

    xodrs = set(
        (map["name"], map["xodr"]) for map in maps if "xodr" in map)

    for target_name, xodr in xodrs:
        with open(os.path.join(dirname, xodr), "rt") as f:
            data = f.read()

        # copy the binary file
        tm_folder_target = os.path.join(
            CARLA_ROOT_PATH,
            "Unreal",
            "CarlaUE4",
            "Content",
            package_name,
            "Maps",
            target_name,
            "TM")

        if not os.path.exists(tm_folder_target):
            os.makedirs(tm_folder_target)

        m = carla.Map(str(target_name), data)
        m.cook_in_memory_map(str(os.path.join(tm_folder_target, "%s.bin" % target_name)))


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
    argparser.add_argument(
        '--json-only',
        action='store_true',
        help='Create JSON files only')
    argparser.add_argument(
        '--batch-size',
        type=float,
        default=300,
        help='Max batch size in MB')
    args = argparser.parse_known_args()[0]

    import_folder = os.path.join(CARLA_ROOT_PATH, "Import")
    json_list = get_packages_json_list(import_folder)
    decals_json = get_decals_json_file(import_folder)

    if len(json_list) < 1:
        json_list = generate_json_package(import_folder, args.package, args.no_carla_materials)

    if len(decals_json) == 0:
        decals_json_file = generate_decals_file(import_folder)

    if args.json_only == False:
        copy_roadpainter_config_files(args.package)
        import_assets_from_json_list(json_list, args.batch_size)

if __name__ == '__main__':
    main()
