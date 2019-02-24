import argparse
import numpy as np

import carla

list_towns = ['Town1', 'Town2', 'Town3', 'Town4', 'Town5']
number_locations = 5
number_ticks = 100
actor_list = ['vehicle.*']


def weathers():
    list_weathers = [ carla.WeatherParameters.ClearNoon,
                      carla.WeatherParameters.CloudyNoon,
                      carla.WeatherParameters.MidRainSunset,
                      carla.WeatherParameters.HardRainSunset,
                      carla.WeatherParameters.SoftRainSunset
                      ]

    return list_weathers


def define_sensors():
    list_sensor_specs = []

    sensors00 = [{'type':'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 300, 'height': 200, 'fov': 100, 'label':'cam-300x200'}]

    sensors01 = [{'type':'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 800, 'height': 600, 'fov': 100, 'label':'cam-800x600'}]

    sensors02 = [{'type':'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 1900, 'height': 1080, 'fov': 100, 'label':'cam-1900x1080'}]

    sensors03 = [{'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 300, 'height': 200, 'fov': 100, 'label': 'cam-300x200'},
                 {'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.4, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 300, 'height': 200, 'fov': 100, 'label': 'cam-300x200'},
                 ]

    sensors04 = [{'type': 'sensor.lidar.ray_cast', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'yaw': 0.0, 'pitch': 0.0, 'roll': 0.0,
                  'label': 'LIDAR'}]

    list_sensor_specs.append(sensors00)
    list_sensor_specs.append(sensors01)
    list_sensor_specs.append(sensors02)
    list_sensor_specs.append(sensors03)
    list_sensor_specs.append(sensors04)

    return list_sensor_specs


def create_ego_vehicle(world, ego_vehicle, spawn_point, list_sensor_spec):
    if ego_vehicle:
        ego_vehicle.set_transform(spawn_point)
        sensors = None
    else:
        blueprint_library = world.get_blueprint_library()
        blueprint = blueprint_library.filter('vehicle.lincoln.mkz2017')[0]
        ego_vehicle = world.try_spawn_actor(blueprint, spawn_point)

        # setup sensors
        for sensor_spec in list_sensor_spec:
            bp = blueprint_library.find(sensor_spec['id'])
            if sensor_spec['id'].startswith('sensor.camera'):
                bp.set_attribute('image_size_x', str(sensor_spec['width']))
                bp.set_attribute('image_size_y', str(sensor_spec['height']))
                bp.set_attribute('fov', str(sensor_spec['fov']))
                sensor_location = carla.Location(x=sensor_spec['x'], y=sensor_spec['y'], z=sensor_spec['z'])
                sensor_rotation = carla.Rotation(pitch=sensor_spec['pitch'], roll=sensor_spec['roll'], yaw=sensor_spec['yaw'])
            elif sensor_spec['id'].startswith('sensor.lidar'):
                bp.set_attribute('range', '5000')
                sensor_location = carla.Location(x=sensor_spec['x'], y=sensor_spec['y'], z=sensor_spec['z'])
                sensor_rotation = carla.Rotation(pitch=sensor_spec['pitch'], roll=sensor_spec['roll'], yaw=sensor_spec['yaw'])
            elif sensor_spec['id'].startswith('sensor.other.gnss'):
                sensor_location = carla.Location(x=sensor_spec['x'], y=sensor_spec['y'], z=sensor_spec['z'])
                sensor_rotation = carla.Rotation()

            # create sensor
            sensor_transform = carla.Transform(sensor_location, sensor_rotation)
            sensors = world.spawn_actor(bp, sensor_transform, ego_vehicle)

    return ego_vehicle, sensors


def run_benchmark(world, sensor_specs_list, number_locations, number_ticks, actor_list):
    spawn_points = world.get_map().get_spawn_points()
    n = min(number_locations, len(spawn_points))

    ego_vehicle = None
    list_fps = []
    sensor_list = None
    for i in range(n):
        spawn_point = spawn_points[i]
        ego_vehicle, sensors = create_ego_vehicle(world, ego_vehicle, spawn_point, sensor_specs_list)
        if sensors:
            sensor_list = sensors
        ego_vehicle.set_autopilot(True)

        ticks = 0
        while ticks < number_ticks:
            if world.wait_for_tick(10.0):
                continue

            # TODO: get real  FPS
            current_fps = 0

            list_fps.append(current_fps)
            ticks += 1

    for sensor in sensor_list:
        sensor.destroy()
    ego_vehicle.destroy()

    return list_fps


def compute_mean_std(list_values):
    np_values = np.array(list_values)
    mean = np.mean(np_values)
    std = np.std(np_values)

    return mean, std


def serialize_records(records, filename):
    with open(filename, 'w+') as fd:
        s = "| Sensors | Town | Weather | Samples | Mean fps | Std fps |\n"
        s += "| ----------- | ----------- | ----------- | ----------- | ----------- | ----------- |"
        fd.write(s)

        for record in records:
            s = "| {} | {} | {} | {} | {:06.2f} | {:06.2f} |".format(record['sensors'],
                                                                   record['town'],
                                                                   record['weather'],
                                                                   record['samples'],
                                                                   record['fps_mean'],
                                                                   record['fps_std'])
            fd.write(s)


def main(args):
    client = carla.Client('localhost', 2000)
    client.set_timeout(10.0)

    records = []
    for town in list_towns:
        world = client.load_world(town)
        for weather in weathers():
            world.set_weather(weather)
            for sensors in define_sensors():
                list_fps = run_benchmark(world, sensors, number_locations, number_ticks, actor_list)
                mean, std = compute_mean_std(list_fps)

                sensor_str = ""
                for sensor in sensors:
                    sensor_str += (sensor['label'] + " ")

                record = {'sensors': sensor_str,
                          'weather': weather,
                          'town': town,
                          'samples': number_locations*number_ticks,
                          'fps_mean': mean,
                          'fps_std': mean}

                records.append(record)
                print(record)

    serialize_records(records, args.file)

    return 0


if __name__ == '__main__':
    description = "Benchmark CARLA performance in your platform for different towns and sensor configurations\n"

    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('--host', default='localhost', help='IP of the host server (default: localhost)')
    parser.add_argument('--port', default='2000', help='TCP port to listen to (default: 2000)')
    parser.add_argument('--file', type=str, help='Write results into a txt file', default="benchmark.md")
    args = parser.parse_args()

    main(args)


