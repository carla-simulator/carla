#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
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
from queue import Queue
from queue import Empty
import re
import time
import platform
import subprocess

def find_weather_presets():
    rgx = re.compile('.+?(?:(?<=[a-z])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])|$)')
    name = lambda x: ' '.join(m.group(0) for m in rgx.finditer(x))
    presets = [x for x in dir(carla.WeatherParameters) if re.match('[A-Z].+', x)]
    return [(getattr(carla.WeatherParameters, x), name(x)) for x in presets]

def open_file(path):
    if platform.system() == "Windows":
        os.startfile(path)
    else:
        subprocess.Popen(["xdg-open", path])

def main():
    cc = carla.ColorConverter.Raw

    client = carla.Client('127.0.0.1', 2000)
    client.set_timeout(2.0)

    world = client.get_world()

    vehicle = None
    camera = None

    try:
        image_queue = Queue()

        blueprint_library = world.get_blueprint_library()

        camera_bp = blueprint_library.find('sensor.camera.rgb')
        camera_bp.set_attribute('image_size_x', '1920')
        camera_bp.set_attribute('image_size_y', '1080')

        world.apply_settings(carla.WorldSettings(
            synchronous_mode=True,
            no_rendering_mode=False,
            fixed_delta_seconds=1.0/20.0))
        world.tick()

        vehicle_transform = [
            carla.Transform(carla.Location(x=105.66784,y=67.80252,z=-0.0), carla.Rotation(pitch=0.0,yaw=180.0,roll=0.0))
        ]

        vehicle_bp = blueprint_library.find('sensor.other.imu') # will be a car in the future
        vehicle = world.spawn_actor(vehicle_bp, vehicle_transform[0])

        calibrator_bp = blueprint_library.find('static.prop.calibrator')

        camera_settings = [
            (carla.Transform(carla.Location(x=3.35405273,y=-3.03557617,z=1.85515884), carla.Rotation(pitch=-20.397671,yaw=135.063766,roll=0.341087)), 90),
            (carla.Transform(carla.Location(x=-3.20292633,y=-2.32813751,z=1.79877197), carla.Rotation(pitch=-23.255402,yaw=43.908443,roll=0.649716)), 90),
            (carla.Transform(carla.Location(x=-2.11163025,y=3.18936646,z=1.88755127), carla.Rotation(pitch=-23.255121,yaw=-61.601181,roll=0.649744)), 90),
            (carla.Transform(carla.Location(x=3.06805023,y=2.81499054,z=1.88755127), carla.Rotation(pitch=-23.254951,yaw=-133.622498,roll=0.649759)), 90),
            (carla.Transform(carla.Location(x=4.33030243,y=-0.15888763,z=1.47493744), carla.Rotation(pitch=-15.581820,yaw=-184.579498,roll=1.427924)), 90),
            (carla.Transform(carla.Location(x=-0.04058146,y=0.03086796,z=5.19841309), carla.Rotation(pitch=-90.000000,yaw=-4.763644,roll=-85.236633)), 90),
            (carla.Transform(carla.Location(x=-3.84447327,y=0.03088558,z=1.48876389), carla.Rotation(pitch=-19.999920,yaw=0.000751,roll=0.000082)), 90),
            (carla.Transform(carla.Location(x=-2.26887268,y=-3.15259216,z=1.33794464), carla.Rotation(pitch=-14.060455,yaw=59.264236,roll=-1.144938)), 90),
            (carla.Transform(carla.Location(x=-0.02598816,y=-4.77382904,z=1.43991119), carla.Rotation(pitch=-10.000031,yaw=90.000092,roll=-0.000031)), 90),
            (carla.Transform(carla.Location(x=-0.67070068,y=-0.03987844,z=1.04761536), carla.Rotation(pitch=-9.776250,yaw=0.000000,roll=0.000000)), 90)
        ]

        calibrator_settings = [
            carla.Transform(carla.Location(x=2.49233734,y=-0.66457535,z=1.75697784),  carla.Rotation(pitch=2.846715,yaw=198.618912,roll=98.639015)),
            carla.Transform(carla.Location(x=-0.92982712,y=-1.85779770,z=0.61223732),  carla.Rotation(pitch=-2.537846,yaw=114.272583,roll=66.822098)),
            carla.Transform(carla.Location(x=-2.09941238,y=0.05091389,z=1.57293243),   carla.Rotation(pitch=-0.239929,yaw=-1.360830,roll=80.002808)),
            carla.Transform(carla.Location(x=0.84898468,y=2.30053986,z=0.61111752),    carla.Rotation(pitch=-2.032418,yaw=-51.959133,roll=66.007568)),
            carla.Transform(carla.Location(x=2.18879135,y=1.07872337,z=1.53271454),  carla.Rotation(pitch=0.000000,yaw=-110.000183,roll=90.000000)),
            carla.Transform(carla.Location(x=-0.16348768,y=0.84804077,z=3.03298218),   carla.Rotation(pitch=-0.603857,yaw=0.603880,roll=-0.000055)),
            carla.Transform(carla.Location(x=-1.29074249,y=1.40817368,z=1.43738678), carla.Rotation(pitch=-0.000018,yaw=110.000160,roll=90.000122)),
            carla.Transform(carla.Location(x=-0.89705482,y=-2.26339279,z=0.51912533),  carla.Rotation(pitch=1.696204,yaw=139.442917,roll=75.933411)),
            carla.Transform(carla.Location(x=0.01915392,y=-3.01262543,z=0.49601734),    carla.Rotation(pitch=-0.206512,yaw=180.036469,roll=70.000946)),
            carla.Transform(carla.Location(x=2.54500671,y=1.35720947,z=1.22803589),  carla.Rotation(pitch=0.603741,yaw=110.603867,roll=89.999962))
        ]

        weathers = find_weather_presets()

        print("Capturing", len(weathers), "weathers")

        # Set weather
        weather = weathers[0]
        # if weather is not None:
        weathers = [weather]
        for weather in weathers:

            world.set_weather(weather[0])
            world.tick()

            while not image_queue.empty():
                frame = image_queue.get() # discard

            camera_index = 0

            print(weather[1] + "... " + str(camera_index) + "/" + str(len(camera_settings)), end="\r")
            # Move Camera
            for i in range(0, len(camera_settings)):
                camera_transform = camera_settings[i][0]
                calibrator_transform = calibrator_settings[i]
                fov = camera_settings[i][1]

                # Spawn Camera
                camera_bp.set_attribute('fov', str(fov))
                camera = world.spawn_actor(camera_bp, camera_transform, attach_to=vehicle)
                camera.listen(lambda image: image_queue.put(image))

                # Set calibrator
                # calibrator.set_transform(calibrator_transform)
                calibrator = world.spawn_actor(calibrator_bp, calibrator_transform, attach_to=vehicle)

                # Avoiding weird artifacts in the first frames
                world.tick()
                frame = image_queue.get() # discard
                world.tick()
                frame = image_queue.get() # discard

                world.tick()
                frame = image_queue.get()

                frame.save_to_disk('screenshots/%s_%02d.png' % (weather[1], camera_index), cc)

                camera_index = camera_index + 1

                # Destroy Camera
                camera.destroy()

                # Destroy Calibrator
                calibrator.destroy()

                print(weather[1] + "... " + str(camera_index) + "/" + str(len(camera_settings)), end="\r")
            print(weather[1] + " Done      ")

        print("Finished! :D")


    finally:
        open_file("screenshots")

        world.apply_settings(carla.WorldSettings(
            synchronous_mode=False,
            no_rendering_mode=False,
            fixed_delta_seconds=0.0))

        # Destroy calibrator
        if calibrator is not None:
          calibrator.destroy()

        # Destroy Vehicle
        if vehicle is not None:
            vehicle.destroy()

        # Destroy Camera
        if camera is not None:
            camera.destroy()

if __name__ == "__main__":
    main()