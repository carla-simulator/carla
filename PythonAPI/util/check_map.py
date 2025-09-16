#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""

Quick check script to get map stuff
"""

import carla


def main():
    # We start creating the client
    client = carla.Client('localhost', 2000)
    client.set_timeout(30.0)
    world = client.get_world()
    crosswalks = world.get_map().get_crosswalks()
    print(f"Crosswalks found: {len(crosswalks)}")
    for crosswalk in crosswalks:
        world.debug.draw_point(crosswalk, size=0.5, color=carla.Color(255, 0, 0), life_time=5000.0)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print(' - Exited by user.')
