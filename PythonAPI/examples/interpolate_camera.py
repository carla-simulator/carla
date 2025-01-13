#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smooth movement of a camera interpolating collection of keypoints provided in a xml file"""

import argparse

import numpy as np
from scipy.interpolate import interp1d

import xml.etree.ElementTree as ET
import carla

FPS = 30
DELTA_SECONDS = 1.0 / FPS
LIFETIME = 50

def parse_key_points(file, id):
    base_tree = ET.parse(file)
    interpolation_key_points = []
    max_order = 0
    for real_tree in base_tree.iter("interpolations"):
        for interpolation in real_tree.iter("interpolation"):
            if interpolation.attrib.get('id', None) != id:
                continue

            for point in interpolation.iter('point'):
                time = float(point.attrib.get('time', 0.0))

                x = float(point.attrib.get('x', 0.0))
                y = float(point.attrib.get('y', 0.0))
                z = float(point.attrib.get('z', 0.0))

                pitch = float(point.attrib.get('pitch', 0.0))
                yaw = float(point.attrib.get('yaw', 0.0))
                roll = float(point.attrib.get('roll', 0.0))

                interpolation_key_points.append([time, carla.Location(x=x, y=y, z=z), carla.Rotation(pitch=pitch, yaw=yaw, roll=roll)])
            max_order = int(interpolation.attrib.get('order', 3))

    if not interpolation_key_points:
        raise ValueError("Couldn't find the interpolation id")
    return interpolation_key_points, max_order

def interpolate_data(points, max_order):
    def interpolate_element(time_points, elem_points, time_disc):
        if len(time_points) > 3 and max_order >= 3:
            inter_kind = 'cubic'
        elif len(time_points) > 2 and max_order >= 2:
            inter_kind = 'quadratic'
        elif len(time_points) > 1 and max_order >= 1:
            inter_kind = 'linear'
        else:
            inter_kind = 'zero'

        f_elem = interp1d(time_points, elem_points, kind=inter_kind)
        elems = list(f_elem(time_disc))
        return elems

    def normalize_angles(angle_list):
        """Interpoaltion will fail between angle jumps, so add / substract 360 as needed"""
        for i in range(1, len(angle_list)):
            while abs(angle_list[i-1] - angle_list[i]) > 180:
                if angle_list[i-1] > angle_list[i]:
                    angle_list[i] += 360
                else:
                    angle_list[i] -= 360
        return angle_list

    def get_time_data(points):
        """The given 'time' is the duration, so add it all together to get the time"""
        time_list = []
        for item in points:
            time_list.append(item[0] + (time_list[-1] if time_list else 0))
        return time_list

    # Separate the data
    time_list = get_time_data(points)
    x_list = [item[1].x for item in points]
    y_list = [item[1].y for item in points]
    z_list = [item[1].z for item in points]
    roll_list = normalize_angles([item[2].roll for item in points])
    pitch_list = normalize_angles([item[2].pitch for item in points])
    yaw_list = normalize_angles([item[2].yaw for item in points])

    # Discretize the time
    time_disc = []
    for i in range(len(time_list) - 1):
        current_time = time_list[i]
        next_time = time_list[i+1]
        time_disc.extend(list(np.arange(current_time, next_time, step=1/FPS)))

    # Interpolate the data
    x_disc = interpolate_element(time_list, x_list, time_disc)
    y_disc = interpolate_element(time_list, y_list, time_disc)
    z_disc = interpolate_element(time_list, z_list, time_disc)
    roll_disc = interpolate_element(time_list, roll_list, time_disc)
    pitch_disc = interpolate_element(time_list, pitch_list, time_disc)
    yaw_disc = interpolate_element(time_list, yaw_list, time_disc)

    # Join the data
    data = []
    for i in range(len(time_disc)):
        data.append(carla.Transform(
            carla.Location(x=x_disc[i], y=y_disc[i], z=z_disc[i]),
            carla.Rotation(roll=roll_disc[i], pitch=pitch_disc[i], yaw=yaw_disc[i])
        ))

    return data

def main():

    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host', metavar='H', default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port', metavar='P', default=2000, type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-f', '--file', required=True,
        help='Interpolation file')
    argparser.add_argument(
        '--id', required=True,
        help='Id to be run part of the interpolation file.')
    argparser.add_argument(
        '--debug', action='store_true',
        help='Debug interpolation data.')
    argparser.add_argument(
        '-st', '--start-time',
        default=0, type=int, help='Starting keypoint of the interpolation.')
    argparser.add_argument(
        '--sync', action='store_true',
        help='Activates synchronous mode.')
    args = argparser.parse_args()

    client = carla.Client('localhost', 2000)
    world = client.get_world()

    try:
        if args.sync:
            original_settings = world.get_settings()
            settings = world.get_settings()
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = DELTA_SECONDS
            world.apply_settings(settings)

        key_points, max_order = parse_key_points(args.file, args.id)
        data = interpolate_data(key_points, max_order)

        try:
            for _ in range(int(args.start_time * FPS)):
                _ = data.pop(0)
        except Exception:
            pass

        if args.debug:

            # Draw the keypoints (and their data)
            for point in key_points:
                world.debug.draw_point(point[1], size=0.2, color=carla.Color(0, 0, 128), life_time=LIFETIME)
                text = f"Keypoint: {point[0]}, " \
                       f"{str(round(point[1].x, 2))}, " \
                       f"{str(round(point[1].y, 2))}, " \
                       f"{str(round(point[1].z, 2))}, " \
                       f"{str(round(point[2].roll, 2))}, " \
                       f"{str(round(point[2].yaw, 2))}, " \
                       f"{str(round(point[2].pitch, 2))}"
                world.debug.draw_string(point[1] + carla.Location(z=1), text, color=carla.Color(0, 128, 0), life_time=LIFETIME)

            # Calculate and draw the speed
            for i in range(1, len(key_points)):
                kp = key_points[i][1]
                prev_kp = key_points[i-1][1]
                middle_kp = carla.Location((kp.x + prev_kp.x)/2, (kp.y + prev_kp.y)/2, (kp.z + prev_kp.z)/2)
                speed = kp.distance(prev_kp) / float(key_points[i][0])
                world.debug.draw_string(middle_kp + carla.Location(z=1), "Speed: " + str(speed), color=carla.Color(128, 0, 0), life_time=LIFETIME)

            # Draw all points + arrows showing their rotation
            for i, transform in enumerate(data):
                world.debug.draw_point(transform.location, size=0.05, life_time=LIFETIME, color=carla.Color(128, 0, 128))
                forward_vector = transform.get_forward_vector()
                world.debug.draw_arrow(transform.location, transform.location + 2 * forward_vector,
                                       thickness=0.03, arrow_size=0.03, life_time=LIFETIME, color=carla.Color(128, 0, 128))
                if i % 200 == 0:
                    world.tick()

        else:
            spectator = world.get_spectator()
            while len(data) > 0:
                new_transform = data.pop(0)
                spectator.set_transform(new_transform)
                world.tick()

    except KeyboardInterrupt:
        pass

    finally:
        if args.sync:
            world.apply_settings(original_settings)

if __name__ == '__main__':
    main()