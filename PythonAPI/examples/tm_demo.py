
import time
import random

import sys, os
sys.path.append("/home/jacopobartiromo/carla/PythonAPI/carla/dist/carla-0.9.6-py2.7-linux-x86_64.egg")
sys.path.append("/home/jacopobartiromo/carla/TrafficManager/source/pipeline/python/Wrapper.cpp")
import carla
from traffic_manager import traffic_manager, parameters, actor_list

ip = 'localhost'
port = 2000
number_of_vehicles = 20

client_connection = carla.Client(ip, port)
client_connection.set_timeout(2.0)

world = client_connection.get_world()
blueprints = world.get_blueprint_library().filter('vehicle.*')

blueprints = [x for x in blueprints if int(x.get_attribute('number_of_wheels')) == 4]
blueprints = [x for x in blueprints if not x.id.endswith('isetta')]
blueprints = [x for x in blueprints if not x.id.endswith('carlacola')]

spawn_points = world.get_map().get_spawn_points()
random.shuffle(spawn_points)

vehicle_list = []
batch = []

for n, transform in enumerate(spawn_points):
    if n >= number_of_vehicles:
        break
    blueprint = random.choice(blueprints)
    if blueprint.has_attribute('color'):
        color = random.choice(blueprint.get_attribute('color').recommended_values)
        blueprint.set_attribute('color', color)
    if blueprint.has_attribute('driver_id'):
        driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
        blueprint.set_attribute('driver_id', driver_id)
    blueprint.set_attribute('role_name', 'autopilot')
    vehicle = world.try_spawn_actor(blueprint, transform)
    vehicle_list.append(vehicle)

tm = None

try:

    long_pid = parameters()
    long_high_pid = parameters()
    lat_pid = parameters()
    vehicle_vec = actor_list()

    long_pid.extend([0.1, 0.15, 0.01])
    long_high_pid.extend([5.0, 0.0, 0.1])
    lat_pid.extend([10.0, 0.01, 0.1])
    vehicle_vec.extend(vehicle_list)

    tm = traffic_manager(long_pid, long_high_pid, lat_pid, 25.0/3.6, 50.0/3.6, client_connection)
    tm.start()
    time.sleep(1)
    tm.register_vehicles(vehicle_vec)

    while True:
        time.sleep(1)

except Exception, e:

    print e
    print "Stopping TrafficManager!"

    if tm:
        tm.stop()

    for vehicle in vehicle_list:
		vehicle.destroy()
