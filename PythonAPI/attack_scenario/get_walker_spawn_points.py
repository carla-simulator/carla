# modified from https://github.com/ricardodeazambuja/carla-simulator-python/blob/804c05416ebf754ecd41eb18e71b21795eb8b65e/my_modified_files/getSpawnPoint4Walker.py

import traceback
import sys
from time import sleep

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D # https://stackoverflow.com/a/56222305/7658422
from matplotlib.widgets import Button

import glob
import os

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import numpy as np

import carla

fig = plt.figure()
ax = plt.axes(projection = '3d')
ax.view_init(elev=30., azim=50)
ax.dist = 10

ax.axes.set_xlim3d(left=-500, right=500)
ax.axes.set_ylim3d(bottom=-500, top=500) 
ax.axes.set_zlim3d(bottom=-2, top=10)

sampled_pts, = ax.plot([], [], [], linestyle="", marker="*", color='red', markersize=5)

failed_attempts = 0

def button_click(event, time2exit=False):
    global failed_attempts
    failed_attempts = 1000
    if time2exit:
        exit(0)

# defining button and add its functionality
axes = plt.axes([0.65, 0.05, 0.1, 0.075])
bexit = Button(axes, 'Exit', color="red")
bexit.on_clicked(lambda e: button_click(e, time2exit=True))
axes = plt.axes([0.8, 0.05, 0.1, 0.075])
bnext = Button(axes, 'Next', color="orange")
bnext.on_clicked(button_click)


plt.ion()
plt.show()

try:
    # setup client/server
    client = carla.Client('localhost', 2000)
    client.set_timeout(60)
    available_maps = client.get_available_maps()

    # world = client.load_world(available_maps[0])
    world = client.get_world()
    mapID = world.get_map()
    settings = world.get_settings()
    settings.fixed_delta_seconds = 1./10
    settings.synchronous_mode = True
    world.apply_settings(settings)
    world.set_weather(getattr(carla.WeatherParameters, 'ClearNoon'))
    world.tick()
    
    with open(f'pedestrian_spawn_points.csv','a') as csvfile:
        print('mapID, spawn_location.x, spawn_location.y, spawn_location.z', file=csvfile)

        ax.set_title(mapID)
        spawn_points = []
        uniqueWalkerSpawnPoints = set()
        attemptedSpawn = 0
        # world = client.load_world(mapID)
        walkerBP = world.get_blueprint_library().filter('walker.pedestrian.*')[0]
        spawnPoint = carla.Transform()
        failed_attempts = 0
        while failed_attempts<300:
            world.tick()
            spawnPoint.location = world.get_random_location_from_navigation()
            if spawnPoint.location is None:
                failed_attempts += 1
                sleep(0.1)
                continue
            newWalker = world.try_spawn_actor(walkerBP, spawnPoint)
            spawn_point = [round(i,1) for i in (spawnPoint.location.x,spawnPoint.location.y,spawnPoint.location.z)]
            spawn_point_set = tuple([round(i,0) for i in spawn_point])
            if (newWalker is not None) and (spawn_point_set not in uniqueWalkerSpawnPoints):
                uniqueWalkerSpawnPoints.add(spawn_point_set)
                print(f'{mapID}, {spawn_point[0]}, {spawn_point[1]}, {spawn_point[2]}', file=csvfile)
                print(f'[{failed_attempts:03d}] - {mapID}, {spawn_point[0]}, {spawn_point[1]}, {spawn_point[2]}')
                failed_attempts = 0
                spawn_points.append(spawn_point)
                tmp = np.asanyarray(spawn_points)
                sampled_pts.set_data (tmp[:,0], tmp[:,1])
                sampled_pts.set_3d_properties(tmp[:,2])
                plt.draw()
                plt.pause(0.01)
                newWalker.destroy()
            else:
                failed_attempts += 1
                sleep(0.1)
                continue

except Exception as err:
    print(traceback.format_exc())