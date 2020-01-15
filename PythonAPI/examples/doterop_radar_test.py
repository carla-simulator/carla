#!/usr/bin/env python

# Copyright (c) 2019 Marc G Puig. All rights reserved.
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

import argparse
import math
import time
import weakref
import random


def get_correct_type(attr):
    if attr.type == carla.ActorAttributeType.Bool:
        return attr.as_bool()
    if attr.type == carla.ActorAttributeType.Int:
        return attr.as_int()
    if attr.type == carla.ActorAttributeType.Float:
        return attr.as_float()
    if attr.type == carla.ActorAttributeType.String:
        return attr.as_str()
    if attr.type == carla.ActorAttributeType.RGBColor:
        return attr.as_color()


def main():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '--fps',
        metavar='N',
        default=30,
        type=int,
        help='set fixed FPS, zero for variable FPS')
    argparser.add_argument(
        '--vertical-fov',
        metavar='N',
        default=30.0,
        type=float,
        help='radar\'s vertical FOV (default: 30.0)')
    argparser.add_argument(
        '--horizontal-fov',
        metavar='N',
        default=30.0,
        type=float,
        help='radar\'s horizontal FOV (default: 30.0)')
    argparser.add_argument(
        '--points-per-second',
        metavar='N',
        default=1500,
        type=int,
        help='radar\'s total points per second (default: 1500)')
    argparser.add_argument(
        '--range',
        metavar='D',
        default=100.0,
        type=float,
        help='radar\'s maximum range in meters (default: 100.0)')
    argparser.add_argument(
        '--time',
        metavar='S',
        default=10,
        type=float,
        help='time to run in seconds (default: 10)')
    args = argparser.parse_args()

    start_timestamp = int(time.time())

    try:

        client = carla.Client(args.host, args.port)
        client.set_timeout(2.0)
        world = client.get_world()

        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = (1.0 / args.fps) if args.fps > 0.0 else 0.0
        world.apply_settings(settings)

        bp_lb = world.get_blueprint_library()

        start_transform = random.choice(world.get_map().get_spawn_points())
        sensors_transform = carla.Transform(carla.Location(x=1.6, z=1.7))

        # Car
        mustang_bp = bp_lb.filter('vehicle.ford.mustang')[0]
        mustang = world.spawn_actor(mustang_bp, start_transform)
        # mustang.set_autopilot(True)

        # Radar
        radar_bp = bp_lb.filter('sensor.other.radar')[0]
        # Set radar attributes
        radar_bp.set_attribute('horizontal_fov', str(args.horizontal_fov)) # degrees
        radar_bp.set_attribute('vertical_fov', str(args.vertical_fov)) # degrees
        radar_bp.set_attribute('points_per_second', str(args.points_per_second))
        radar_bp.set_attribute('range', str(args.range)) # meters
        # Spawn it
        radar = world.spawn_actor(radar_bp, sensors_transform, mustang)

        # Display Radar attributes
        print(f"{'- Radar Information ':-<45}")
        print(f" - {'ID':<23}{radar_bp.id}")
        for attr in radar_bp:
            print(f" - {attr.id+':':<23}{get_correct_type(attr)}")
        print(f"{'':-<45}")

        # Camera
        camera_bp = bp_lb.filter('sensor.camera.rgb')[0]
        # Set camera attributes
        image_size = (1920, 1080)
        camera_bp.set_attribute('image_size_x', str(image_size[0]))
        camera_bp.set_attribute('image_size_y', str(image_size[1]))
        # Spawn it
        camera = world.spawn_actor(camera_bp, sensors_transform, mustang)

        # Ploting variables
        p_depth = []
        p_azimuth = []
        p_altitude = []
        p_velocity = []
        p_images = []

        def radar_callback(weak_radar, sensor):
            self = weak_radar()
            if not self:
                return
            current_depth = []
            current_azimuth = []
            current_altitude = []
            current_velocity = []
            for i in sensor:
                current_depth.append(i.depth)
                current_azimuth.append(i.azimuth)
                current_altitude.append(i.altitude)
                current_velocity.append(i.velocity)
            p_depth.append(current_depth)
            p_azimuth.append(current_azimuth)
            p_altitude.append(current_altitude)
            p_velocity.append(current_velocity)
        weak_radar = weakref.ref(radar)
        radar.listen(lambda sensor: radar_callback(weak_radar, sensor))

        def camera_callback(weak_camera, image):
            self = weak_camera()
            if not self:
                return
            # p_images.append(image)
            # image.save_to_disk(f'_out_{start_timestamp}/{image.frame:08}')
        weak_camera = weakref.ref(camera)
        camera.listen(lambda image: camera_callback(weak_camera, image))

        time_to_run = args.time # in seconds
        initial_time = time.time()
        close_time = initial_time + time_to_run

        while time.time() < close_time:
            sys.stdout.write(f"\rElapsed time: {time.time() - initial_time:4.1f}/{time_to_run} s")
            sys.stdout.flush()
            world.tick()
            # time.sleep(1)

    finally:
        print('')
        try:
            settings.synchronous_mode = False
            settings.fixed_delta_seconds = None
            world.apply_settings(settings)
        except NameError:
            pass

        try:
            radar.destroy()
        except NameError:
            pass

        try:
            camera.destroy()
        except NameError:
            pass

        try:
            mustang.destroy()
        except NameError:
            pass

        import numpy as np
        import matplotlib.pyplot as plt
        from matplotlib import animation

        p_depth = np.array([np.array(i) for i in p_depth])
        p_azimuth = np.array([np.array(i) for i in p_azimuth])
        p_altitude = np.array([np.array(i) for i in p_altitude])
        p_velocity = np.array([np.array(i) for i in p_velocity])
        p_images = np.array([np.array(i.raw_data) for i in p_images])

        # im = np.frombuffer(p_images[5], dtype=np.dtype("uint8"))
        # im = im.reshape((image_size[1], image_size[0], 4))
        # im = im[:, :, :3]
        # im = im[:, :, ::-1]
        # plt.imshow(im)
        # plt.show()
        # sys.exit(0)

        # Animation ##################################################
        fig, (ax_depth, ax_vel) = plt.subplots(nrows=1, ncols=2, figsize=(14, 6))
        half_hor_fov_to_deg = math.radians(args.horizontal_fov) / 2.0
        half_ver_fov_to_deg = math.radians(args.vertical_fov) / 2.0
        horizontal_axes_limits = (-half_hor_fov_to_deg, half_hor_fov_to_deg)
        vertical_axes_limits = (-half_ver_fov_to_deg, half_ver_fov_to_deg)

        # - Depth Axes ----------------------------------
        ax_depth.set(xlim=horizontal_axes_limits, ylim=vertical_axes_limits)
        ax_depth.set_title('Depth (m)')
        ax_depth.set_xlabel('Azimuth (rad)')
        ax_depth.set_ylabel('Altitude (rad)')

        # Initialize the depth scater plot without data
        scat_depth = ax_depth.scatter(
            np.array([]),
            np.array([]),
            c=np.array([]),  # dot intensity/color
            s=20,            # dot size
            vmin=0.0,        # colorize from 0 m
            vmax=args.range, # to Radar's maximum disance (far)
            cmap='viridis')  # viridis, plasma, gray...
        fig.colorbar(scat_depth, ax=ax_depth, extend='max', shrink=1.0, pad=0.01)

        # - Velocity Axes -------------------------------
        ax_vel.set(xlim=horizontal_axes_limits, ylim=vertical_axes_limits)
        ax_vel.set_title('Velocity (m/s)')
        ax_vel.set_xlabel('Azimuth (rad)')
        ax_vel.set_ylabel('Altitude (rad)')

        # Initialize the velocity scater plot without data
        scat_vel = ax_vel.scatter(
            np.array([]),
            np.array([]),
            c=np.array([]), # dot intensity/color
            s=20,           # dot size
            vmin=-10.0,     # colorize from -10 m/s
            vmax=10.0,      # to +10 m/s
            cmap='RdBu')    # RdBu, Spectral, coolwarm...
        fig.colorbar(scat_vel, ax=ax_vel, extend='both', shrink=1.0, pad=0.01)

        # Animation initialization callback
        def acc_init():
            scat_depth.set_array(p_depth[0])
            scat_vel.set_array(p_velocity[0])
            scat_depth.set_offsets(np.c_[p_azimuth[0], p_altitude[0]])
            scat_vel.set_offsets(np.c_[p_azimuth[0], p_altitude[0]])
            return scat_depth, scat_vel

        # Animation update callback
        def radar_anim_update(i):
            scat_depth.set_array(p_depth[i])
            scat_vel.set_array(p_velocity[i])
            scat_depth.set_offsets(np.c_[p_azimuth[i], p_altitude[i]])
            scat_vel.set_offsets(np.c_[p_azimuth[i], p_altitude[i]])
            return scat_depth, scat_vel

        # Do the animation
        radar_animation = animation.FuncAnimation(
            fig,
            radar_anim_update,
            init_func=acc_init,
            frames=len(p_azimuth),
            interval=(1.0 / args.fps) * 1000 if args.fps > 0.0 else 0.0,
            blit=True,
            repeat=True)

        plt.show()
        # Store the animation as mp4
        # radar_animation.save(
        #     f"_out_{start_timestamp}/radar_plot.mp4",
        #     fps=args.fps,
        #     extra_args=["-vcodec", "libx264"])
        ##############################################################

if __name__ == '__main__':
    main()
