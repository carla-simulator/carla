#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Raycast sensor profiler

This script can be used to test, visualize and profile the raycast sensors,
LiDARs and Radar (radar visualization not available, sorry).

By default, the script render one RGB Camera and three LiDARS whose output
can be visualized in a window just running:
  python raycast_sensor_testing.py

For profiling, you can choose the number of LiDARs and Radars and then use
the profiling option to run a series of simulations for points from 100k
to 1.5M per second. In this mode we do not render anything but processing
of the data is done.
For example for profiling one lidar:
  python raycast_sensor_testing.py -ln 1 --profiling
For example for profiling one semantic lidar:
  python raycast_sensor_testing.py -sln 1 --profiling
And for profiling one radar:
  python raycast_sensor_testing.py -rn 1 --profiling

"""

import carla
import argparse
import random
import time
import numpy as np


try:
    import pygame
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_q
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

class CustomTimer:
    def __init__(self):
        try:
            self.timer = time.perf_counter
        except AttributeError:
            self.timer = time.time

    def time(self):
        return self.timer()

class DisplayManager:
    def __init__(self, grid_size, window_size, show_window=True):
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
        self.timer = CustomTimer()

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
            camera.listen(self.save_rgb_image)

            return camera

        elif sensor_type == 'LiDAR':
            lidar_bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast')
            lidar_bp.set_attribute('range', '100')
            lidar_bp.set_attribute('dropoff_general_rate', lidar_bp.get_attribute('dropoff_general_rate').recommended_values[0])
            lidar_bp.set_attribute('dropoff_intensity_limit', lidar_bp.get_attribute('dropoff_intensity_limit').recommended_values[0])
            lidar_bp.set_attribute('dropoff_zero_intensity', lidar_bp.get_attribute('dropoff_zero_intensity').recommended_values[0])

            for key in sensor_options:
                lidar_bp.set_attribute(key, sensor_options[key])


            lidar = self.world.spawn_actor(lidar_bp, transform, attach_to=attached)

            lidar.listen(self.save_lidar_image)

            return lidar
        elif sensor_type == 'SemanticLiDAR':
            lidar_bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast_semantic')
            lidar_bp.set_attribute('range', '100')

            for key in sensor_options:
                lidar_bp.set_attribute(key, sensor_options[key])

            lidar = self.world.spawn_actor(lidar_bp, transform, attach_to=attached)

            lidar.listen(self.save_semanticlidar_image)

            return lidar
        elif sensor_type == "Radar":
            radar_bp = self.world.get_blueprint_library().find('sensor.other.radar')
            for key in sensor_options:
                radar_bp.set_attribute(key, sensor_options[key])

            radar = self.world.spawn_actor(radar_bp, transform, attach_to=attached)
            radar.listen(self.save_radar_image)

            return radar
        else:
            return None

    def get_sensor(self):
        return self.sensor

    def save_rgb_image(self, image):
        t_start = self.timer.time()

        image.convert(carla.ColorConverter.Raw)
        array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        array = np.reshape(array, (image.height, image.width, 4))
        array = array[:, :, :3]
        array = array[:, :, ::-1]

        if self.display_man.render_enabled():
            self.surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))

        t_end = self.timer.time()
        self.time_processing += (t_end-t_start)
        self.tics_processing += 1

    def save_lidar_image(self, image):
        t_start = self.timer.time()

        disp_size = self.display_man.get_display_size()
        lidar_range = 2.0*float(self.sensor_options['range'])

        points = np.frombuffer(image.raw_data, dtype=np.dtype('f4'))
        points = np.reshape(points, (int(points.shape[0] / 4), 4))
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

        t_end = self.timer.time()
        self.time_processing += (t_end-t_start)
        self.tics_processing += 1

    def save_semanticlidar_image(self, image):
        t_start = self.timer.time()

        disp_size = self.display_man.get_display_size()
        lidar_range = 2.0*float(self.sensor_options['range'])

        points = np.frombuffer(image.raw_data, dtype=np.dtype('f4'))
        points = np.reshape(points, (int(points.shape[0] / 6), 6))
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

        t_end = self.timer.time()
        self.time_processing += (t_end-t_start)
        self.tics_processing += 1

    def save_radar_image(self, radar_data):
        t_start = self.timer.time()
        #print("Hola, saving Radar data!!")
        # To get a numpy [[vel, altitude, azimuth, depth],...[,,,]]:
        points = np.frombuffer(radar_data.raw_data, dtype=np.dtype('f4'))
        points = np.reshape(points, (len(radar_data), 4))

        t_end = self.timer.time()
        self.time_processing += (t_end-t_start)
        self.tics_processing += 1

    def render(self):
        if self.surface is not None:
            offset = self.display_man.get_display_offset(self.display_pos)
            self.display_man.display.blit(self.surface, offset)

    def destroy(self):
        self.sensor.destroy()

def one_run(args, client):
    """This function performed one test run using the args parameters
    and connecting to the carla client passed.
    """

    display_manager = None
    vehicle = None
    vehicle_list = []
    prof_str = ""
    timer = CustomTimer()

    try:

        # Getting the world and
        world = client.get_world()

        if args.sync:
            traffic_manager = client.get_trafficmanager(8000)
            settings = world.get_settings()
            traffic_manager.set_synchronous_mode(True)
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            world.apply_settings(settings)

        if args.profiling:
            settings.no_rendering_mode = True
            world.apply_settings(settings)


        # Instanciating the vehicle to which we attached the sensors
        bp = world.get_blueprint_library().filter('vehicle')[0]

        if bp.has_attribute('color'):
            color = random.choice(bp.get_attribute('color').recommended_values)
            bp.set_attribute('color', color)

        vehicle = world.spawn_actor(bp, world.get_map().get_spawn_points()[0])
        vehicle_list.append(vehicle)

        vehicle.set_autopilot(True)


        # Display Manager organize all the sensors an its display in a window
        display_manager = DisplayManager(grid_size=[2, 2], window_size=[args.width, args.height], show_window=args.render_window)


        # If require, we instanciate the RGB camera
        if args.render_cam:
            SensorManager(world, display_manager, 'RGBCamera', carla.Transform(carla.Location(x=1.5, z=2.4)), vehicle, {}, [0, 0])


        # If any, we instanciate the required lidars
        lidar_points_per_second = args.lidar_points

        if args.lidar_number >= 3:
            SensorManager(world, display_manager, 'LiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '50',  'points_per_second': lidar_points_per_second, 'rotation_frequency': '20'}, [1, 0])

        if args.lidar_number >= 2:
            SensorManager(world, display_manager, 'LiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '100', 'points_per_second': lidar_points_per_second, 'rotation_frequency': '20'}, [0, 1])

        if args.lidar_number >= 1:
            SensorManager(world, display_manager, 'LiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '200', 'points_per_second': lidar_points_per_second, 'rotation_frequency': '20'}, [1, 1])


        # If any, we instanciate the required semantic lidars
        semanticlidar_points_per_second = args.semanticlidar_points

        if args.semanticlidar_number >= 3:
            SensorManager(world, display_manager, 'SemanticLiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '50', 'points_per_second': semanticlidar_points_per_second, 'rotation_frequency': '20'}, [1, 0])

        if args.semanticlidar_number >= 2:
            SensorManager(world, display_manager, 'SemanticLiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '100', 'points_per_second': semanticlidar_points_per_second, 'rotation_frequency': '20'}, [0, 1])

        if args.semanticlidar_number >= 1:
            SensorManager(world, display_manager, 'SemanticLiDAR', carla.Transform(carla.Location(x=0, z=2.4)), vehicle, {'channels' : '64', 'range' : '200', 'points_per_second': semanticlidar_points_per_second, 'rotation_frequency': '20'}, [1, 1])


        # If any, we instanciate the required radars
        radar_points_per_second = args.radar_points

        if args.radar_number >= 3:
            SensorManager(world, display_manager, 'Radar', carla.Transform(carla.Location(x=0, z=2.4), carla.Rotation(pitch=5, yaw=90)), vehicle, {'points_per_second': radar_points_per_second}, [2, 2])

        if args.radar_number >= 2:
            SensorManager(world, display_manager, 'Radar', carla.Transform(carla.Location(x=0, z=2.4), carla.Rotation(pitch=5, yaw=-90)), vehicle, {'points_per_second': radar_points_per_second}, [2, 2])

        if args.radar_number >= 1:
            SensorManager(world, display_manager, 'Radar', carla.Transform(carla.Location(x=0, z=2.4), carla.Rotation(pitch=5)), vehicle, {'points_per_second': radar_points_per_second}, [2, 2])


        call_exit = False
        time_init_sim = timer.time()

        frame = 0
        time0 = timer.time()

        while True:
            frame += 1

            # Carla Tick
            if args.sync:
                world.tick()
            else:
                world.wait_for_tick()

            # Render received data
            display_manager.render()

            # Time measurement for profiling or to output
            if not args.profiling:
                if frame == 30:
                    time_frames = timer.time() - time0
                    time_procc = 0
                    for sensor in display_manager.sensor_list:
                        time_procc += sensor.time_processing

                    print("FPS: %.3f %.3f %.3f" % (time_frames, 1.0/time_frames * 30, time_procc/time_frames))
                    frame = 0
                    for sensor in display_manager.sensor_list:
                        sensor.time_processing = 0
                        sensor.tics_processing = 0
                    time0 = timer.time()

                if args.render_window:
                    for event in pygame.event.get():
                        if event.type == pygame.QUIT:
                            call_exit = True
                        elif event.type == pygame.KEYDOWN:
                            if event.key == K_ESCAPE or event.key == K_q:
                                call_exit = True
                                break
            else:
                if (timer.time() - time_init_sim) < 5.0:
                    frame = 0
                    for sensor in display_manager.sensor_list:
                        sensor.time_processing = 0
                        sensor.tics_processing = 0
                    time0 = timer.time()

                if (timer.time() - time0) > 10.0:
                    time_frames = timer.time() - time0

                    time_procc = 0
                    for sensor in display_manager.sensor_list:
                        time_procc += sensor.time_processing
                    prof_str = "%-10s %-9s  %-9s %-15s %-7.2f %-20.3f" % (args.lidar_number, args.semanticlidar_number, args.radar_number, lidar_points_per_second, float(frame) / time_frames, time_procc/time_frames)
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

        if args.profiling:
            settings.no_rendering_mode = False
            world.apply_settings(settings)


    return prof_str



def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Sensor tutorial')
    argparser.add_argument(
        '--host', metavar='H', default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port', metavar='P', default=2000, type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '--sync', action='store_true',
        help='Synchronous mode execution')
    argparser.add_argument(
        '--async', dest='sync', action='store_false',
        help='Asynchronous mode execution')
    argparser.set_defaults(sync=True)
    argparser.add_argument(
        '--res', metavar='WIDTHxHEIGHT', default='1280x720',
        help='window resolution (default: 1280x720)')
    argparser.add_argument(
        '-lp', '--lidar_points', metavar='LP', default='100000',
        help='lidar points per second (default: "100000")')
    argparser.add_argument(
        '-ln', '--lidar_number', metavar='LN', default=3, type=int,
        choices=range(0, 4),
        help='Number of lidars to render (from zero to three)')
    argparser.add_argument(
        '-slp', '--semanticlidar_points', metavar='SLP', default='100000',
        help='semantic lidar points per second (default: "100000")')
    argparser.add_argument(
        '-sln', '--semanticlidar_number', metavar='SLN', default=0, type=int, choices=range(0, 4),
        help='Number of semantic lidars to render (from zero to three)')
    argparser.add_argument(
        '-rp', '--radar_points', metavar='RP',  default='100000',
        help='radar points per second (default: "100000")')
    argparser.add_argument(
        '-rn', '--radar_number', metavar='LN', default=0, type=int, choices=range(0, 4),
        help='Number of radars to render (from zero to three)')
    argparser.add_argument(
        '--camera', dest='render_cam', action='store_true',
        help='render also RGB camera (camera enable by default)')
    argparser.add_argument(
        '--no-camera', dest='render_cam', action='store_false', default=True,
        help='no render RGB camera (camera disable by default)')
    argparser.add_argument(
        '--profiling', action='store_true', default=False,
        help='Use the script in profiling mode. It measures the performance of \
              the lidar for different number of points.')
    argparser.add_argument(
        '--no-render-window', action='store_false', dest='render_window', default=True,
        help='Render visualization window.')


    args = argparser.parse_args()
    args.width, args.height = [int(x) for x in args.res.split('x')]

    try:
        client = carla.Client(args.host, args.port)
        client.set_timeout(5.0)

        if args.profiling:
            print("-------------------------------------------------------")
            print("# Running profiling with %s lidars, %s semantic lidars and %s radars." % (args.lidar_number, args.semanticlidar_number, args.radar_number))
            args.render_cam = False
            args.render_window = False
            runs_output = []

            points_range = ['100000', '200000', '300000', '400000', '500000',
                            '600000', '700000', '800000', '900000', '1000000',
                            '1100000', '1200000', '1300000', '1400000', '1500000']
            for points in points_range:
                args.lidar_points = points
                args.semanticlidar_points = points
                args.radar_points = points
                run_str = one_run(args, client)
                runs_output.append(run_str)

            print("-------------------------------------------------------")
            print("# Profiling of parallel raycast sensors (LiDAR and Radar)")
            try:
                import multiprocessing
                print("#Number of cores: %d" % multiprocessing.cpu_count())
            except ImportError:
                print("#Hardware information not available, please install the " \
                    "multiprocessing module")

            print("#NumLidars NumSemLids NumRadars PointsPerSecond FPS     PercentageProcessing")
            for o  in runs_output:
                print(o)

        else:
            one_run(args, client)

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')


if __name__ == '__main__':

    main()
