#!/usr/bin/env python

# Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys
import argparse
import json

def main():
    """Edits the uproject file to enable and disable the SimReady plugin
    """
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        '-f', '--file',
        metavar='F',
        default="",
        type=str,
        help='Path to the uproject file')
    argparser.add_argument(
        '-p', '--plugin',
        metavar='P',
        default="SimReady",
        type=str,
        help='Plugin name')
    argparser.add_argument(
        '-e', '--enable',
        action='store_true',
        help='enable plugin')
    args = argparser.parse_args()

    # Read uproject (json) file
    uproject_file = open(args.file, 'r')
    uproject_json = json.load(uproject_file)
    uproject_file.close()

    # Get the plugin list
    plugin_list = uproject_json["Plugins"]

    # Edit plugin
    should_do_changes = False
    found = False
    for plugin in plugin_list:
        if plugin['Name'] == args.plugin:
            found = True
            if args.enable:
                if not plugin['Enabled']:
                    should_do_changes = True
                    plugin['Enabled'] = True
            else:
                if plugin['Enabled']:
                    should_do_changes = True
                    plugin['Enabled'] = False
    if not found:
        should_do_changes = True
        plugin_list.append({
            "Name": args.plugin,
            "Enabled": True if args.enable else False
        })

    # Save file if there are changes to do
    if should_do_changes:
        uproject_file = open(args.file, 'w')
        uproject_file.write(json.dumps(uproject_json, indent = 4))
        uproject_file.close()

if __name__ == '__main__':
    main()
