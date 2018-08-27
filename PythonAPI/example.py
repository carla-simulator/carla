#!/usr/bin/env python

import sys

sys.path.append(
    'PythonAPI/carla-0.9.0-py%d.%d-linux-x86_64.egg' % (sys.version_info.major,
                                                        sys.version_info.minor))

import carla

import os
import random
import time

# This function is here because this functionality haven't been ported to the
# new API yet.
def save_to_disk(image):
    """Save this image to disk (requires PIL installed)."""

    filename = '_images/{:0>6d}_{:s}.png'.format(image.frame_number, image.type)

    try:
        from PIL import Image as PImage
    except ImportError:
        raise RuntimeError(
            'cannot import PIL, make sure pillow package is installed')

    image = PImage.frombytes(
        mode='RGBA',
        size=(image.width, image.height),
        data=image.raw_data,
        decoder_name='raw')
    color = image.split()
    image = PImage.merge("RGB", color[2::-1])

    folder = os.path.dirname(filename)
    if not os.path.isdir(folder):
        os.makedirs(folder)
    image.save(filename)


def main(add_a_camera, enable_autopilot):
    client = carla.Client('localhost', 2000)
    client.set_timeout(2000)

    print('client version: %s' % client.get_client_version())
    print('server version: %s' % client.get_server_version())

    world = client.get_world()

    blueprint_library = world.get_blueprint_library()

    vehicle_blueprints = blueprint_library.filter('vehicle')


    actor_list = []

    try:

        while True:

            bp = random.choice(vehicle_blueprints)

            if bp.contains_attribute('number_of_wheels'):
                n = bp.get_attribute('number_of_wheels')
                print('spawning vehicle %r with %d wheels' % (bp.id, n))

            color = random.choice(bp.get_attribute('color').recommended_values)
            bp.set_attribute('color', color)

            transform = carla.Transform(
                carla.Location(x=180.0, y=199.0, z=40.0),
                carla.Rotation(yaw=0.0))

            vehicle = world.try_spawn_actor(bp, transform)

            if vehicle is None:
                continue

            actor_list.append(vehicle)

            print(vehicle)

            if add_a_camera:
                add_a_camera = False

                camera_bp = blueprint_library.find('sensor.camera')
                # camera_bp.set_attribute('post_processing', 'Depth')
                camera_transform = carla.Transform(carla.Location(x=0.4, y=0.0, z=1.4))
                camera = world.spawn_actor(camera_bp, camera_transform, attach_to=vehicle)
                camera.listen(save_to_disk)

            if enable_autopilot:
                vehicle.set_autopilot()
            else:
                vehicle.apply_control(carla.VehicleControl(throttle=1.0, steer=-1.0))

            time.sleep(3)

            print('vehicle at %s' % vehicle.get_location())
            vehicle.set_location(carla.Location(x=220, y=199, z=38))
            print('is now at %s' % vehicle.get_location())

            time.sleep(2)

    finally:

        for actor in actor_list:
            actor.destroy()


if __name__ == '__main__':

    main(add_a_camera=False, enable_autopilot=True)
