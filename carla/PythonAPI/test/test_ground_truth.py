#!/usr/bin/env python

# Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla
import numpy as np
import cv2
from pathlib import Path

from agents.navigation.behavior_agent import BehaviorAgent  # pylint: disable=import-error
from agents.navigation.basic_agent import BasicAgent  # pylint: disable=import-error
from agents.navigation.constant_velocity_agent import ConstantVelocityAgent  # pylint: disable=import-error

HERE = Path(__file__).parent

def main():
    from argparse import ArgumentParser
    print('Parsing command line')
    parser = ArgumentParser()
    parser.add_argument(
        '--host', metavar='H', default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    parser.add_argument(
        '-p', '--port', metavar='P', default=2000, type=int,
        help='TCP port to listen to (default: 2000)')
    parser.add_argument(
        '--res', metavar='WIDTHxHEIGHT', default='1920x1080',
        help='window resolution (default: 1920x1080)')
    parser.add_argument(
        '--filter', metavar='PATTERN', default='lincoln',
        help='actor filter (default: "lincoln")')
    parser.add_argument(
        '--sync', action='store_true',
        help='Activate synchronous mode execution')
    parser.add_argument(
        '--spawn-point', type=int, default=0,
        help='Vehicle spawn point index.')
    parser.add_argument(
        '--gamma', default=1.0, type=float,
        help='Gamma correction of the camera (default: 1.0)')
    parser.add_argument(
        '--sensor', type=str, default='sensor.camera.rgb',
        help='Sensor ID')
    parser.add_argument(
        '-a', '--agent', type=str, choices=['Behavior', 'Basic', 'Constant'], default='Behavior',
        help='select which agent to run')
    parser.add_argument(
        '-b', '--behavior', type=str, choices=["cautious", "normal", "aggressive"], default='normal',
        help='Choose one of the possible agent behaviors (default: normal)')
    parser.add_argument(
        '-s', '--seed', type=int, default=0xcdcdcdcd,
        help='PRNG seed value')
    parser.add_argument(
        '-n', '--frame-count', type=int, default=100,
        help='Number of frames to save')
    parser.add_argument(
        '-w', '--warmup-frame-count', type=int, default=10,
        help='Number of frames to save')
    parser.add_argument(
        '-g', '--generate',
        action='store_true',
        help='Whether to generate the ground truth file.')
    parser.add_argument(
        '--plot', action='store_true',
        help='Whether to plot results')
    argv = parser.parse_args()

    OUTPUT_DIR = HERE / '_ground_truth'
    OUTPUT_FILE = OUTPUT_DIR.with_suffix('.mkv')

    if not argv.generate:
        assert OUTPUT_FILE.exists()

    try:

        if OUTPUT_DIR.exists():
            import shutil
            shutil.rmtree(OUTPUT_DIR)
        OUTPUT_DIR.mkdir(exist_ok=True)
        
        print('Setting up client')
        underlying_prng = np.random.PCG64(argv.seed)
        prng = np.random.Generator(underlying_prng)
        client = carla.Client()
        client.set_timeout(10.0)
        world = client.get_world()
        world.reset_all_traffic_lights()
        traffic_manager = client.get_trafficmanager()
        map = world.get_map()
        settings = world.get_settings()
        blueprint_library = world.get_blueprint_library()
        sensor_bp = blueprint_library.find(argv.sensor)
        assert sensor_bp != None
        if argv.sync:
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            world.apply_settings(settings)
            traffic_manager.set_synchronous_mode(True)
        vehicle_bp = blueprint_library.filter(argv.filter)
        assert len(vehicle_bp) != 0
        vehicle_bp = vehicle_bp[0]
        spawn_points = map.get_spawn_points()
        spawn_point_index = argv.spawn_point

        print('Spawning vehicle')
        # Vehicle setup:
        vehicle = None
        while True:
            spawn_point = spawn_points[spawn_point_index]
            vehicle = world.try_spawn_actor(vehicle_bp, spawn_point)
            if vehicle != None:
                world.tick()
                break
            spawn_point_index += 1
            if spawn_point_index == 100:
                raise Exception('Failed to spawn vehicle after 100 retries.')
        print(f'Vehicle {vehicle} spawned at {spawn_point}')
        
        print('Spawning sensor')
        if argv.sensor.startswith('sensor.camera'):
            width, height = (float(e) for e in argv.res.split('x'))
            print(f'Image resolution: {width} x {height}')
            sensor_bp.set_attribute('image_size_x', str(width))
            sensor_bp.set_attribute('image_size_y', str(height))
            if sensor_bp.has_attribute('gamma'):
                sensor_bp.set_attribute('gamma', str(argv.gamma))
            ppp = 'Default'
            if 'Town10HD_Opt' in map.name:
                ppp = 'Town10HD_Opt'
            sensor_bp.set_attribute('post_process_profile', ppp)
            print(f'Image gamma: {argv.gamma}')
        bound_x = 0.5 + vehicle.bounding_box.extent.x
        bound_y = 0.5 + vehicle.bounding_box.extent.y
        bound_z = 0.5 + vehicle.bounding_box.extent.z
        sensor = world.spawn_actor(
            sensor_bp,
            carla.Transform(carla.Location(x=+0.8*bound_x, y=+0.0*bound_y, z=1.3*bound_z)),
            attach_to=vehicle,
            attachment_type=carla.AttachmentType.Rigid)
        global frame_count
        frame_count = 0
        def SaveImage(image):
            global frame_count
            image.convert(carla.ColorConverter.Raw)
            out_path = HERE / '_ground_truth' / f'{image.frame}.png'
            print(f'Saving frame {out_path.relative_to(HERE)}.', end='\r')
            image.save_to_disk(str(out_path))
            frame_count += 1
        sensor.listen(SaveImage)
        
        print('Configuring agent')
        agent = None
        if argv.agent == 'Basic':
            agent = BasicAgent(vehicle, 30)
            agent.follow_speed_limits(True)
        elif argv.agent == 'Constant':
            agent = ConstantVelocityAgent(vehicle, 30)
            ground_loc = world.world.ground_projection(vehicle.get_location(), 5)
            if ground_loc:
                world.player.set_location(ground_loc.location + carla.Location(z=0.01))
            agent.follow_speed_limits(True)
        elif argv.agent == 'Behavior':
            agent = BehaviorAgent(vehicle, behavior=argv.behavior)
        def GetNextDestination():
            return prng.choice(spawn_points).location
        agent.set_destination(GetNextDestination())

        print('Setup finished\nRecording')
        while True:
            if frame_count >= argv.frame_count + argv.warmup_frame_count:
                break
            if argv.sync:
                world.tick()
            else:
                world.wait_for_tick()
            if agent.done():
                agent.set_destination(GetNextDestination())
            control = agent.run_step()
            control.manual_gear_shift = False
            vehicle.apply_control(control)

    except Exception as e:

        print(f'Exception thrown: {e}')

    finally:

        if world != None:
            settings = world.get_settings()
            settings.synchronous_mode = False
            settings.fixed_delta_seconds = None
            world.apply_settings(settings)
            traffic_manager.set_synchronous_mode(True)

        def TryDestroy(target):
            if target == None:
                return
            target.destroy()
        
        TryDestroy(vehicle)
        TryDestroy(sensor)

        if frame_count != argv.frame_count + argv.warmup_frame_count:
            exit(1)

        frame_paths = [
            file
            for file in OUTPUT_DIR.iterdir()
            if file.name.endswith('.png')
        ]
        frame_paths = frame_paths[argv.warmup_frame_count:]
        
        if argv.generate:

            print(f'Generating .MKV video at {OUTPUT_FILE}')
            height, width, _ = cv2.imread(str(frame_paths[0])).shape
            fourcc = cv2.VideoWriter_fourcc(*"MJPG")
            video = cv2.VideoWriter(
                OUTPUT_FILE,
                fourcc,
                60,
                (width, height))
            for frame_path in frame_paths:
                video.write(cv2.imread(str(frame_path)))
            video.release()

        elif OUTPUT_FILE.is_file():

            print(f'Comparing recorded frames against {OUTPUT_FILE.relative_to(HERE)}')
            video = cv2.VideoCapture(OUTPUT_FILE)
            assert video.isOpened()
            correlation_results = []
            bhattacharyya_results = []

            for frame_path in frame_paths:
                print(f'Checking frame {frame_path.relative_to(HERE)}', end='\r')
                ok, reference = video.read()
                if not ok:
                    break
                frame = cv2.imread(str(frame_path))
                assert np.shape(reference) == np.shape(frame)
                reference = cv2.cvtColor(reference, cv2.COLOR_BGR2HSV)
                frame = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
                H_BINS = 50
                S_BINS = 60
                hist1 = cv2.calcHist([frame], [0, 1], None, [H_BINS,S_BINS], [0, 180, 0, 256]).astype(np.float32)
                hist1 = cv2.normalize(hist1, hist1).flatten()
                hist2 = cv2.calcHist([reference], [0, 1], None, [H_BINS,S_BINS], [0, 180, 0, 256]).astype(np.float32)
                hist2 = cv2.normalize(hist2, hist2).flatten()
                correlation_results.append(cv2.compareHist(hist1, hist2, cv2.HISTCMP_CORREL))
                bhattacharyya_results.append(cv2.compareHist(hist1, hist2, cv2.HISTCMP_BHATTACHARYYA))

            video.release()
            correlation_avg = np.mean(correlation_results)
            bhattacharyya_avg = np.mean(bhattacharyya_results)
            correlation_var = np.var(correlation_results)
            bhattacharyya_var = np.var(bhattacharyya_results)
            if argv.plot:
                from matplotlib import pyplot as plt
                plt.plot(
                    np.linspace(0, len(correlation_results), len(correlation_results)),
                    correlation_results)
                plt.plot(
                    np.linspace(0, len(bhattacharyya_results), len(bhattacharyya_results)),
                    bhattacharyya_results)
                plt.show()
            print(
                f'Final results:'
                f' - correlation_avg: {correlation_avg} (expected >=0.6)\n'
                f' - bhattacharyya_avg: {bhattacharyya_avg} (expected <=0.3)\n'
                f' - correlation_var: {correlation_var} (expected <=0.02)\n'
                f' - bhattacharyya_var: {bhattacharyya_var} (expected <=0.02)\n')
            exit_code = 0 if all([
                np.round(correlation_avg, 2) >= 0.6,
                np.round(bhattacharyya_avg, 2) <= 0.3,
                np.round(correlation_var, 2) <= 0.02,
                np.round(bhattacharyya_var, 2) <= 0.02
            ]) else 1
            print(f'Using exit_code={exit_code}')
            exit(exit_code)



if __name__ == '__main__':
    main()