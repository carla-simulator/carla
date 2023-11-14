import logging
import math
import random
import carla
import time
import numpy as np
import pandas as pd
from math import sin, radians, sqrt
from itertools import product

# Initialize CARLA client and connect to the simulator
client = carla.Client('localhost', 2000)
client.set_timeout(2.0)

# Load the CARLA world and map
world = client.get_world()
map = world.get_map()

# Create an ego vehicle 
ego_vehicle_bp = world.get_blueprint_library().find('vehicle.audi.a2')


'''spawn_points = world.get_map().get_spawn_points()
number_of_spawn_points = len(spawn_points)

if 0 < number_of_spawn_points:
    random.shuffle(spawn_points)
    ego_transform = spawn_points[0]
    ego_vehicle = world.spawn_actor(ego_vehicle_bp,ego_transform)
    print('\nEgo is spawned')
else: 
    logging.warning('Could not find any spawn points')

print(ego_vehicle.get_location())'''

actor_list = []

for actor in actor_list:
    actor.destroy()
actor_list = []


blueprint_library = world.get_blueprint_library()
spawn_points = world.get_map().get_spawn_points()
print(world.get_actors())


#ego_vehicle = world.get_actor(147)
#second_vehicle = world.get_actor(153)
#print(ego_vehicle.get_location())