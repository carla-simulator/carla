#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import random
import time


def main():
    #type here
    f = open("map.osm", 'r')
    osm_data = f.read()
    f.close()

    # Define the desired settings. In this case, default values.
    settings = carla.Osm2OdrSettings()

    # Convert to .xodr
    xodr_data = carla.Osm2Odr.convert(osm_data, settings)

    # save opendrive file
    f = open("map_opendrive", 'w')
    f.write(xodr_data)
    f.close()


if __name__ == '__main__':

    main()
