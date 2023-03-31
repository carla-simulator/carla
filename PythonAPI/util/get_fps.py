#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys
import time
import threading

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
    try:
        # connect several clients
        client = carla.Client('localhost', 2000)
        client.set_timeout(15.0)

        # get the world
        world = client.get_world()

        # mode synchronous
        settings = world.get_settings()
        # settings.synchronous_mode = True
        # settings.fixed_delta_seconds = 1 / 20
        # world.apply_settings(settings)

        # f = open("carla.txt", "tw")
        prev = 0
        prevTime = 0
        refresh = 0
        mean = 0
        total_frames  = 0
        while 1:
            # if settings.synchronous_mode:
            #     world.tick()
            #     snapshot = world.get_snapshot()
            # else:
            snapshot = world.wait_for_tick()
            # f.writelines("%d) : %d\n" % (snapshot.frame, (1/(snapshot.platform_timestamp - prevTime))))

            if (prev > 0 and prev != snapshot.frame - 1):
                print("Frame error:", prev, snapshot.frame)
            #     f.writelines("Frame error:%d , %d\n" % (prev, snapshot.frame))

            if snapshot.platform_timestamp - prevTime <= 1:
                mean += snapshot.platform_timestamp - prevTime
                total_frames+=1

                # print(int(1/(snapshot.platform_timestamp - prevTime)), "(%f)" % (mean/total_frames))
            if (refresh >= 0.5):
                print(int(1/(snapshot.platform_timestamp - prevTime)), "(%f)" % (1/float(mean/float(total_frames))))
                refresh -= 0.5
            refresh += snapshot.delta_seconds
            prevTime = snapshot.platform_timestamp

    finally:
        # f.close()
        # mode synchronous
        # settings = world.get_settings()
        # settings.synchronous_mode = False
        # settings.fixed_delta_seconds = 0
        # world.apply_settings(settings)
        pass

if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
