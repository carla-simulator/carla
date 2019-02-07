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


if os.name == 'nt':
    sys_name = 'Windows'
    print('We are on Windows.')
elif os.name == 'posix':
    sys_name = 'Linux'
    print('We are on Linux')

def main():
    print("Hi")
    generate_json("Test_ToolRoadRunner")
    import_assets_commandlet()

def import_assets_commandlet():
    ue4_path = os.environ['UE4_ROOT']
    editor_path = "%s/Engine/Binaries/%s/UE4Editor" % (ue4_path, sys_name)
    uproject_path = "/home/dnovillo/carla/Unreal/CarlaUE4/CarlaUE4.uproject"
    commandlet_name = "ImportAssets"
    commandlet_arguments = "-importSettings=\"/home/dnovillo/carla/PythonAPI/importsetting.json\" -AllowCommandletRendering -nosourcecontrol -replaceexisting"
    full_command = "%s %s -run=%s %s" % (editor_path, uproject_path, commandlet_name, commandlet_arguments)
    subprocess.check_call([full_command], shell=True)
    os.remove("importsetting.json")

def generate_json(map_name):
    print("Hi")
    fh = open("importsetting.json", "a+")
    import_groups = []
    file_names = []
    import_settings = []

    file_names.append("..\\..\\RoadRunnerFiles\\%s.fbx" % map_name)

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
    import_groups.append({
    "ImportSettings": import_settings,
    "FactoryName": "FbxFactory",
    "DestinationPath": "/Game/Carla/%s" % map_name,
    "bReplaceExisting": "true",
    "FileNames": file_names
    })
    fh.write(json.dumps({"ImportGroups": import_groups}))
    fh.close()



if __name__ == '__main__':

    try:
        main()
    finally:
        print('\ndone.')