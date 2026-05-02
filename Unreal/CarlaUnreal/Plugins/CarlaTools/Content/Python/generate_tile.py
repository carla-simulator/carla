#!/usr/bin/env python
import unreal
import argparse
import json
import os
import subprocess


"""Generic function for running a commandlet with its arguments."""
ue4_path = os.environ["UE4_ROOT"]
uproject_path = unreal.Paths.project_dir() + ("CarlaUnreal.uproject")
run = "-run=%s" % ("GenerateTileCommandlet")

print("Before any Commandlet:")

argparser = argparse.ArgumentParser()

argparser.add_argument(
    '-s', '--paramstring',
    metavar='S',
    default='',
    type=str,
    help='String to put as arguments')
args = argparser.parse_args()

arguments = args.paramstring

if os.name == "nt":
    sys_name = "Win64"
    editor_path = "%s/Engine/Binaries/%s/UE4Editor" % (ue4_path, sys_name)
    command = [editor_path, uproject_path, run]
    command.extend(arguments)
    print("Commandlet:", command)
    print("Arguments:", arguments)
    subprocess.check_call(command, shell=True)
elif os.name == "posix":
    sys_name = "Linux"
    editor_path = "%s/Engine/Binaries/%s/UE4Editor" % (ue4_path, sys_name)
    full_command = editor_path + " " + uproject_path + " " + run + " " + arguments
    print("Commandlet:", full_command)
    print("Arguments:", arguments)
    subprocess.call([full_command], shell=True)

