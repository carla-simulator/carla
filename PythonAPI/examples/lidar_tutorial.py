#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Spawn NPCs into the simulation"""

import glob
import os
import sys
import argparse

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

import numpy as np

try:
    import pygame
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_q
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')


class DisplayManager:
    def __init__(self, grid_size=[1, 1], window_size=[1280, 720], show_window=True):
        if show_window:
            pygame.init()
            pygame.font.init()
            self.display = pygame.display.set_mode(window_size, pygame.HWSURFACE | pygame.DOUBLEBUF)
        else:
            self.display = None

        self.grid_size = grid_size
        self.window_size = window_size
        self.sensor_list = []

    def get_window_size(self):
        return [int(self.window_size[0]), int(self.window_size[1])]

    def get_display_size(self):
        return [int(self.window_size[0]/self.grid_size[0]), int(self.window_size[1]/self.grid_size[1])]

    def get_display_offset(self, gridPos):
        dis_size = self.get_display_size()
        return [int(gridPos[0] * dis_size[0]), int(gridPos[1] * dis_size[1])]

    def add_sensor(self, sensor):
        self.sensor_list.append(sensor)

    def get_sensor_list(self):
        return self.sensor_list

    def render(self):
        if not self.render_enabled():
            return

        for s in self.sensor_list:
            s.render()

        pygame.display.flip()

    def destroy(self):
        for s in self.sensor_list:
            s.destroy()

    def render_enabled(self):
        return self.display != None

class SensorManager:
    def __init__(self, world, display_man, sensor_type, transform, attached, sensor_options, display_pos):
        self.surface = None
        self.world = world
        self.display_man = display_man
        self.display_pos = display_pos
        self.sensor = self.init_sensor(sensor_type, transform, attached, sensor_options)
        self.sensor_options = sensor_options

        self.time_processing = 0.0
        self.tics_processing = 0

        self.display_man.add_sensor(self)

    def init_sensor(self, sensor_type, transform, attached, sensor_options):
        if sensor_type == 'RGBCamera':
            camera_bp = self.world.get_blueprint_library().find('sensor.camera.rgb')
            disp_size = self.display_man.get_display_size()
            camera_bp.set_attribute('image_size_x', str(disp_size[0]))
            camera_bp.set_attribute('image_size_y', str(disp_size[1]))

            for key in sensor_options:
              camera_bp.set_attribute(key, sensor_options[key])

            camera = self.world.spawn_actor(camera_bp, transform, attach_to=attached)
            camera.listen(lambda image: self.save_rgb_image(image))

            return camera

        elif sensor_type == 'LiDAR':
            lidar_bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast')
            lidar_bp.set_attribute('range', '100')

            for key in sensor_options:
              lidar_bp.set_attribute(key, sensor_options[key])

            lidar = self.world.spawn_actor(lidar_bp, transform, attach_to=attached)

            lidar.listen(lambda image: self.save_lidar_image(image))

            return lidar

        else:
            return None

    def get_sensor(self):
        return self.sensor


    def save_rgb_image(self, image):
        t_start = time.perf_counter()

        image.convert(carla.ColorConverter.Raw)
        array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        array = np.reshape(array, (image.height, image.width, 4))
        array = array[:, :, :3]
        array = array[:, :, ::-1]

        if self.display_man.render_enabled():
            self.surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))

        t_end = time.perf_counter()
        self.time_processing += (t_end-t_start)
        self.tics_processing += 1

    def save_lidar_image(self, image):
        t_start = time.perf_counter()

        disp_size = self.display_man.get_display_size()
        lidar_range = 2.0*float(self.sensor_options['range'])

        points = np.frombuffer(image.raw_data, dtype=np.dtype('f4'))
        points = np.reshape(points, (int(points.shape[0] / 3), 3))
        lidar_data = np.array(points[:, :2])
        lidar_data *= min(disp_size) / lidar_range
        lidar_data += (0.5 * disp_size[0], 0.5 * disp_size[1])
        lidar_data = np.fabs(lidar_data)  # pylint: disable=E1111
        lidar_data = lidar_data.astype(np.int32)
        lidar_data = np.reshape(lidar_data, (-1, 2))
        lidar_img_size = (disp_size[0], disp_size[1], 3)
        lidar_img = np.zeros((lidar_img_size), dtype=np.uint8)
        lidar_img[tuple(lidar_data.T)] = (255, 255, 255)

        if self.display_man.render_enabled():
            self.surface = pygame.surfarray.make_surface(lidar_img)

        t_end = time.perf_counter()
        self.time_processing += (t_end-t_start)
        self.tics_processing += 1

    def render(self):
        if self.surface is not None:
            offset = self.display_man.get_display_offset(self.display_pos)
            self.display_man.display.blit(self.surface, offset)

    def destroy(self):
        self.sensor.destroy()


def one_run(args):
    display_manager = None
    vehicle = None
    vehicle_list = []

    # In this tutorial script, we are going to add a vehicle to the simulation
    # and let it drive in autopilot. We will also create a camera attached to
    # that vehicle, and save all the images generated by the camera to disk.

    try:
        # First of all, we need to create the client that will send the requests
        # to the simulator. Here we'll assume the simulator is accepting
        # requests in the localhost at port 2000.
        client = carla.Client(args.host, args.port)
        client.set_timeout(2.0)

        # Once we have a client we can retrieve the world that is currently
        # running.
        world = client.get_world()


        if args.sync:
            traffic_manager = client.get_trafficmanager(8000)
            settings = world.get_settings()
            traffic_manager.set_synchronous_mode(True)
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            world.apply_settings(settings)



        # The world contains the list blueprints that we can use for adding new
        # actors into the simulation.
        blueprint_library = world.get_blueprint_library()

        # Now let's filter all the blueprints of type 'vehicle' and choose one
        # at random.
        #bp = random.choice(blueprint_library.filter('vehicle'))
        bp = blueprint_library.filter('vehicle')[0]

        # A blueprint contains the list of attributes that define a vehicle's
        # instance, we can read them and modify some of them. For instance,
        # let's randomize its color.
        if bp.has_attribute('color'):
            color = random.choice(bp.get_attribute('color').recommended_values)
            bp.set_attribute('color', color)

        # Now we need to give an initial transform to the vehicle. We choose a
        # random transform from the list of recommended spawn points of the map.
        transform = world.get_map().get_spawn_points()[0]

        # So let's tell the world to spawn the vehicle.
        vehicle = world.spawn_actor(bp, transform)
        vehicle_list.append(vehicle)

        # Let's put the vehicle to drive around.
        vehicle.set_autopilot(True)

        # Let's add now a "depth" camera attached to the vehicle. Note that the
        # transform we give here is now relative to the vehicle.

        # Now we register the function that will be called each time the sensor
        # receives an image. In this example we are saving the image to disk
        # converting the pixels to gray-scale.

        display_manager = DisplayManager(grid_size=[2, 2], show_window=args.render_window)

        if args.render_cam:
            SensorManager(world, display_manager, 'RGBCamera', carla.Transform(carla.Location(x=1.5, z=2.4)), vehicle, {}, [0, 0])

        lidar_points_per_second = args.lidar_points

        if args.lidar_number >= 3:
            SensorManager(world, display_manager, 'LiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '50',  'points_per_second': lidar_points_per_second}, [1,0])

        if args.lidar_number >= 2:
            SensorManager(world, display_manager, 'LiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '100', 'points_per_second': lidar_points_per_second}, [0,1])

        if args.lidar_number >= 1:
            SensorManager(world, display_manager, 'LiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '200', 'points_per_second': lidar_points_per_second}, [1,1])


        call_exit = False
        time_init_sim = time.perf_counter()

        frame = 0
        time0 = time.perf_counter()

        while True:
            frame += 1

            if args.sync:
                world.tick()
            else:
                world.wait_for_tick()

            display_manager.render()


            if not args.profiling:
                if frame == 30:
                    time_frames = time.perf_counter() - time0
                    time_procc = 0
                    for sensor in display_manager.sensor_list:
                        time_procc += sensor.time_processing

                    print("%.3f %.3f %.3f" % (time_frames, 1/time_frames * 30, time_procc/time_frames))
                    frame = 0
                    for sensor in display_manager.sensor_list:
                        sensor.time_processing = 0
                        sensor.tics_processing = 0
                    time0 = time.perf_counter()

                if args.render_window:
                    for event in pygame.event.get():
                        if event.type == pygame.QUIT:
                            call_exit = True
                        elif event.type == pygame.KEYDOWN:
                            if event.key == K_ESCAPE or event.key == K_q:
                                call_exit = True
                                break
            else:
                if (time.perf_counter() - time_init_sim) < 5.0:
                    frame = 0
                    for sensor in display_manager.sensor_list:
                        sensor.time_processing = 0
                        sensor.tics_processing = 0
                    time0 = time.perf_counter()

                if (time.perf_counter() - time0) > 10.0:
                    time_frames = time.perf_counter() - time0

                    time_procc = 0
                    for sensor in display_manager.sensor_list:
                        time_procc += sensor.time_processing
                    print("%s %s %.3f %.3f" % (args.lidar_number, lidar_points_per_second, float(frame) / time_frames, time_procc/time_frames))
                    break
            if call_exit:
                break

    finally:
        if display_manager:
            display_manager.destroy()

        client.apply_batch([carla.command.DestroyActor(x) for x in vehicle_list])

        if args.sync:
            settings = world.get_settings()
            settings.synchronous_mode = False
            settings.fixed_delta_seconds = None
            world.apply_settings(settings)



def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Sensor tutorial')
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
        '--sync',
        action='store_true',
        help='Synchronous mode execution')
    argparser.add_argument(
        '--async',
        dest='sync',
        action='store_false',
        help='Asynchronous mode execution')
    argparser.set_defaults(sync=True)
    argparser.add_argument(
        '--res',
        metavar='WIDTHxHEIGHT',
        default='1280x720',
        help='window resolution (default: 1280x720)')
    argparser.add_argument(
        '-lp', '--lidar_points',
        metavar='LD',
        default='100000',
        help='lidar points per second (default: "100000")')
    argparser.add_argument(
        '-ln', '--lidar_number',
        metavar='LN',
        default=3,
        type=int,
        choices=range(0, 4),
        help='Number of lidars to render')
    argparser.add_argument(
        '--camera',
        dest='render_cam', action='store_true',
        help='render also RGB camera (camera enable by default)')
    argparser.add_argument('--no-camera',
        dest='render_cam', action='store_false',
        help='no render RGB camera (camera disable by default)')
    argparser.set_defaults(render_cam=True)
    argparser.add_argument(
        '--profiling',
        action='store_true',
        help='Use the script in profiling mode. It measures the performance of \
         the lidar for different number of points.')
    argparser.set_defaults(profiling=False)
    argparser.add_argument(
        '--no-render-window',
        action='store_false',
        dest='render_window',
        help='Render visualization window.')
    argparser.set_defaults(render_window=True)

    args = argparser.parse_args()

    args.width, args.height = [int(x) for x in args.res.split('x')]

    if args.profiling:
        args.render_cam = False
        args.render_window = False

        points_range = ['100000', '200000', '300000', '400000', '500000', '600000', '650000', '700000', '750000', '800000', '850000', '900000', '950000', '1000000']
        print("#NumLidars PointsPerSecond FPS PercentageProcessing")
        for points in points_range:
            args.lidar_points = points
            one_run(args)
    else:
        one_run(args)



if __name__ == '__main__':

    main()
