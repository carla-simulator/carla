#!/usr/bin/env python

# Copyright (c) 2019 Aptiv
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Carla API mock client-side data recorder.

Should be used together with `manual_control.py` or any other controlling script
Default configuration requires `hero` actor present in an episode
Example configuration file can be found as `example_recording_config.ini`

For the list of possible arguments execute this file with --help
"""

from __future__ import print_function

import glob
import os
import sys

# ==============================================================================
# -- find carla module ---------------------------------------------------------
# ==============================================================================

try:
    sys.path.append(glob.glob('../../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================


import carla

import argparse
import logging
import time

try:
    import configparser
except ImportError:
    raise RuntimeError('cannot import configparser, make sure configparser package is installed')


# ==============================================================================
# -- Data recording ------------------------------------------------------------
# ==============================================================================

def create_dir(dir):
    if not os.path.exists(os.path.dirname(dir)):
        os.makedirs(os.path.dirname(dir))
        logging.info("created dir: %s" % dir)


def has_actor_role_filter(role):
    def has_actor_role(actor):
        return 'role_name' in actor.attributes and actor.attributes['role_name'] == role
    return has_actor_role


def set_generic_camera_config_properties(transform, config, defaults=None):
    if defaults is None:
        defaults = {}
    if 'x' in config:
        transform.location.x = float(config['x'])
    elif 'x' in defaults:
        transform.location.x = defaults['x']
    if 'y' in config:
        transform.location.y = float(config['y'])
    elif 'y' in defaults:
        transform.location.y = defaults['y']
    if 'z' in config:
        transform.location.z = float(config['z'])
    elif 'z' in defaults:
        transform.location.z = defaults['z']
    if 'pitch' in config:
        transform.rotation.pitch = float(config['pitch'])
    elif 'pitch' in defaults:
        transform.rotation.pitch = defaults['pitch']
    if 'yaw' in config:
        transform.rotation.yaw = float(config['yaw'])
    elif 'yaw' in defaults:
        transform.rotation.yaw = defaults['yaw']
    if 'roll' in config:
        transform.rotation.roll = float(config['roll'])
    elif 'roll' in defaults:
        transform.rotation.roll = defaults['roll']


class DataRecorder:
    def __init__(self, config, world, output_dir):
        self.actors = []
        self.sensors = []
        self.world = world
        world_callbacks = []
        self.save_start(output_dir)
        for record_config in config:
            record_type = record_config['type']
            output_dir_for_type = "%s/%s/" % (output_dir, record_config['name'])
            create_dir(output_dir_for_type)
            if record_type.startswith('sensor'):
                self.set_sensor(record_config, output_dir, output_dir_for_type)
            elif record_type.startswith('agent') or record_type == 'world':
                world_callbacks.append(self.get_world_callback(record_type, output_dir_for_type))
            else:
                logging.warning("Unknown record type %s" % record_type)
        self.world.on_tick(lambda timestamp: on_tick(timestamp, world_callbacks))

    def save_start(self, output_dir):
        file_path = "%s/start.dict" % output_dir
        create_dir(file_path)
        with open(file_path, 'w') as file:
            print({'timestamp': time.time()}, file=file)
        logging.info("saved starting timestamp in %s" % file_path)

    def set_sensor(self, sensor_config, root_dir, save_dir):
        sensor_type = sensor_config['type']
        location = carla.Location()
        rotation = carla.Rotation()
        transform = carla.Transform(location, rotation)
        data = {
            'name': sensor_config['name'],
            'type': sensor_config['type'],
        }
        bp = self.world.get_blueprint_library().find(sensor_type)

        if 'attach.to.role' in sensor_config:
            following_role = sensor_config['attach.to.role']
            agents_with_given_role = list(filter(has_actor_role_filter(following_role), self.world.get_actors()))
            if not agents_with_given_role:
                logging.error("No actor with role %s found for sensor %s" % (following_role, sensor_config['name']))
                return
            if len(agents_with_given_role) > 1:
                logging.error("Multiple (%d) actors with role %s found for sensor %s"
                              % (len(agents_with_given_role), following_role, sensor_config['name']))
                return
            parent = agents_with_given_role[0]
            data['attach_to_role'] = following_role
            bp.set_attribute('role_name', sensor_config['attach.to.role'] + '_following')
        else:
            parent = None
            data['attach_to_role'] = None

        if sensor_type.startswith('sensor.camera'):
            bp.set_attribute('image_size_x', str(sensor_config['width']))
            bp.set_attribute('image_size_y', str(sensor_config['height']))
        elif sensor_type.startswith('sensor.lidar'):
            bp.set_attribute('range', str(sensor_config['range']))

        if sensor_type.startswith('sensor.camera') or sensor_type.startswith('sensor.lidar'):
            defaults = {'x': -5.5, 'z': 2.5, 'pitch': 8.0}
        elif sensor_type.startswith('sensor.other.gnss'):
            defaults = {'x': 1.0, 'z': 2.8}
        elif sensor_type.startswith('sensor.other.lane_invasion') or sensor_type.startswith('sensor.other.collision'):
            defaults = {}
        else:
            logging.warning('Sensor %s not recognized' % sensor_type)
            return
        set_generic_camera_config_properties(transform, sensor_config, defaults)
        if parent:
            attachment = carla.AttachmentType
            if 'attachment.type' in sensor_config:
                if sensor_config['attachment.type'] == 'Rigid':
                    attachment_type = attachment.Rigid
                else:
                    attachment_type = attachment.SpringArm
                sensor = self.world.spawn_actor(bp, transform, attach_to=parent, attachment_type=attachment_type)
            else:
                sensor = self.world.spawn_actor(bp, transform, attach_to=parent)
        else:
            sensor = self.world.spawn_actor(bp, transform)

        data['transform'] = transform_data(transform)
        save_data("%s/%s.json" % (root_dir, sensor_config['name']), data)
        sensor.listen(get_sensor_callback(sensor_type, save_dir))
        self.sensors.append(sensor)

    def get_world_callback(self, record_type, output_dir):

        def callback_all(timestamp):
            save_data("%s/%s" % (output_dir, timestamp.frame_count), {
                'actors': list(map(lambda actor: actor_data(actor), self.world.get_actors())),
                'timestamp': time.time()
            })

        def callback_world(timestamp):
            save_data("%s/%s" % (output_dir, timestamp.frame_count),
                      world_data(timestamp, self.world))

        callbacks = {
            'agent.all': callback_all,
            'world': callback_world
        }
        return callbacks[record_type]

    def destroy(self):
        for sensor in self.sensors:
            sensor.destroy()


def on_tick(timestamp, callbacks):
    if callbacks:
        logging.debug("recording frame %d" % timestamp.frame_count)
        for callback in callbacks:
            callback(timestamp)
        logging.debug("finished recording frame %d" % timestamp.frame_count)


def world_data(timestamp, world):
    return {
        'timestamp': time.time(),
        'elapsed_seconds': timestamp.elapsed_seconds,
        'map_name': world.get_map().name,
        'frame_count': timestamp.frame_count
    }


def actor_data(data):
    type_id = data.type_id
    properties = {
        'timestamp': time.time(),
        'id': data.id,
        'type_id': data.type_id,
        'is_alive': data.is_alive,
        'attributes': data.attributes,
        'location': vector_data(data.get_location()),
        'transform': transform_data(data.get_transform()),
        'velocity': vector_data(data.get_velocity()),
        'acceleration': vector_data(data.get_acceleration())
    }
    if type_id.startswith('vehicle.'):
        detailed_properties = {
            'bounding_box': bounding_box_data(data.bounding_box),
            'speed_limit': data.get_speed_limit(),
            'traffic_light_state': data.get_traffic_light_state(),
            'is_at_traffic_light': data.is_at_traffic_light(),
            'control': vehicle_control_data(data.get_control())
        }
    elif type_id.startswith('walker.'):
        detailed_properties = {'bounding_box': bounding_box_data(data.bounding_box)}
    elif type_id.startswith('traffic.traffic_light'):
        detailed_properties = {'state': data.state}
    elif type_id.startswith('sensor.'):
        detailed_properties = {'is_listening': data.is_listening}
    else:
        detailed_properties = {}
    properties.update(detailed_properties)
    return properties


def vector_data(vector):
    return {'x': vector.x, 'y': vector.y, 'z': vector.z}


def rotation_data(rotation):
    return {'pitch': rotation.pitch, 'yaw': rotation.yaw, 'roll': rotation.roll}


def transform_data(transform):
    return {
        'location': vector_data(transform.location),
        'rotation': rotation_data(transform.rotation),
        'forward_vector': vector_data(transform.get_forward_vector())
    }


def bounding_box_data(bounding_box):
    return {'location': vector_data(bounding_box.location), 'extent': vector_data(bounding_box.extent)}


def vehicle_control_data(vehicle_control):
    return {
        'throttle': vehicle_control.throttle,
        'steer': vehicle_control.steer,
        'brake': vehicle_control.brake,
        'hand_brake': vehicle_control.hand_brake,
        'reverse': vehicle_control.reverse,
        'gear': vehicle_control.gear,
        'manual_gear_shift': vehicle_control.manual_gear_shift
    }


def lane_markings_data(lane_markings):
    return list(map(lane_marking_data, lane_markings))


def lane_marking_data(lane_marking):
    return {
        'type': lane_marking.type,
        'color': lane_marking.color,
        'lane_change': lane_marking.lane_change,
        'width': lane_marking.width
    }


def save_data(path, data):
    with open(path, 'w') as file:
        print(data, file=file)


def save_binary_data(path, data):
    with open(path, 'wb') as file:
        file.write(data)


def get_sensor_callback(sensor_name, dir):
    def callback_image(data):
        save_data('%s/%s' % (dir, data.frame_number),
                  {
                      'width': data.width,
                      'height': data.height,
                      'fov': data.fov,
                      'timestamp': time.time()
                  })
        save_binary_data('%s/%s-raw-data' % (dir, data.frame_number), data.raw_data)

    def callback_lidar(data):
        save_data('%s/%s' % (dir, data.frame_number),
                  {
                      'horizontal_angle': data.horizontal_angle,
                      'channels': data.channels,
                      'timestamp': time.time()
                  })
        save_binary_data('%s/%s-raw-data' % (dir, data.frame_number), data.raw_data)

    def callback_gnss(data):
        save_data('%s/%s' % (dir, data.frame_number),
                  {
                      'latitude': data.latitude,
                      'longitude': data.longitude,
                      'altitude': data.altitude,
                      'timestamp': time.time()
                  })

    def callback_lane_detector(data):
        save_data('%s/%s' % (dir, data.frame_number),
                  {
                      'crossed_lane_markings': lane_markings_data(data.crossed_lane_markings),
                      'timestamp': time.time()
                  })

    def callback_collision(data):
        save_data('%s/%s' % (dir, data.frame_number), {
            'actor_id': data.actor.id,
            'other_actor_id': data.other_actor.id,
            'actor': actor_data(data.actor),
            'other_actor': actor_data(data.other_actor),
            'normal_impulse': vector_data(data.normal_impulse),
            'frame_number': data.frame_number,
            'timestamp': time.time()
        })

    callbacks = {
        'sensor.camera.rgb': callback_image,
        'sensor.camera.depth': callback_image,
        'sensor.camera.semantic_segmentation': callback_image,
        'sensor.lidar.ray_cast': callback_lidar,
        'sensor.other.gnss': callback_gnss,
        'sensor.other.lane_invasion': callback_lane_detector,
        'sensor.other.collision': callback_collision
    }
    return callbacks[sensor_name]


# ==============================================================================
# -- record_loop() ---------------------------------------------------------------
# ==============================================================================


def record_loop(host, port, recording_config, output_dir):
    data_recorder = None
    try:
        client = carla.Client(host, port)
        client.set_timeout(2.0)

        world = client.get_world()

        world.wait_for_tick()

        data_recorder = DataRecorder(recording_config, world, output_dir)

        while True:
            world.wait_for_tick()

    finally:

        if data_recorder:
            data_recorder.destroy()


# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================


def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Data Recorder Client')
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
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
        '-o', '--output-dir',
        metavar='<OUTPUT DIR>',
        default='output',
        help='directory to save recordings in')
    argparser.add_argument(
        '-c', '--config-file',
        metavar='<CONFIG FILE PATH>',
        required=True,
        help='path to file with config')

    args = argparser.parse_args()

    config_parser = configparser.ConfigParser()
    if not config_parser.read(args.config_file):
        print("Provided config file does not exist: %s" % args.config_file)
        return
    config = [
        {
            str(option): str(config_parser[section][option])
            for option in config_parser[section]
        }
        for section in config_parser
        if config_parser[section]
    ]

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

    print(__doc__)

    try:
        record_loop(args.host, args.port, config, args.output_dir)
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')


if __name__ == '__main__':
    main()
