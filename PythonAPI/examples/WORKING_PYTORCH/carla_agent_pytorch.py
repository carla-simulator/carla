import glob
import os
import sys
import random
import time
import numpy as np
import cv2
import math
import pickle
from collections import deque
import matplotlib.pyplot as plt
import torch 
import torch.nn as nn
import torch.nn.functional as F
from threading import Thread

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass
import carla
from carla import ColorConverter as cc


SHOW_PREVIEW = True
IM_WIDTH = 360
IM_HEIGHT = 240
EPISODES = 40_000
PREDICTED_ANGLES = [-1, 0, 1]
MAX_TIMESTEP = 1000

class CarEnv:
    SHOW_CAM = SHOW_PREVIEW
    im_width = IM_WIDTH
    im_height = IM_HEIGHT
    front_camera = None

    def __init__(self):
        self.client = carla.Client("127.0.0.1", 2000)
        self.client.set_timeout(10.0)
        self.world = self.client.get_world()
        self.world.constant_velocity_enabled=True
        self.blueprint_library = self.world.get_blueprint_library()
        self.model_3 = self.blueprint_library.filter("model3")[0]


    def restart(self):
        self.collision_hist = deque(maxlen=2000)
        self.actor_list = deque(maxlen=2000)
        self.lane_invasion = deque(maxlen=2000)
        self.obstacle_distance = deque(maxlen=2000)

        self.transform = random.choice(self.world.get_map().get_spawn_points())
        self.vehicle = self.world.spawn_actor(self.model_3, self.transform)
        self.actor_list.append(self.vehicle)

        # self.rgb_cam = self.blueprint_library.find('sensor.camera.rgb')
        # self.rgb_cam.set_attribute("image_size_x", f"{self.im_width}")
        # self.rgb_cam.set_attribute("image_size_y", f"{self.im_height}")
        # self.rgb_cam.set_attribute("fov", f"110")

        self.ss_cam = self.blueprint_library.find('sensor.camera.semantic_segmentation')
        self.ss_cam.set_attribute("image_size_x", f"{self.im_width}")
        self.ss_cam.set_attribute("image_size_y", f"{self.im_height}")
        self.ss_cam.set_attribute("fov", f"110")

        transform = carla.Transform(carla.Location(x=2.5, z=0.7))
        self.sensor = self.world.spawn_actor(self.ss_cam, transform, attach_to=self.vehicle)
        self.actor_list.append(self.sensor)
        self.sensor.listen(lambda data: self.process_img(data))

        self.vehicle.apply_control(carla.VehicleControl(throttle=0.0, brake=0.0))
        self.vehicle.enable_constant_velocity(carla.Vector3D(11, 0, 0))
        time.sleep(4)

        col_sensor = self.blueprint_library.find("sensor.other.collision")
        self.col_sensor = self.world.spawn_actor(col_sensor, transform, attach_to=self.vehicle)
        self.actor_list.append(self.col_sensor)
        self.col_sensor.listen(lambda event: self.collision_data(event))

        lane_sensor = self.blueprint_library.find("sensor.other.lane_invasion")
        self.lane_sensor = self.world.spawn_actor(lane_sensor, transform, attach_to=self.vehicle)
        self.actor_list.append(self.lane_sensor)
        self.lane_sensor.listen(lambda lanesen: self.lane_data(lanesen))
        
        obs_sensor = self.blueprint_library.find("sensor.other.radar")
        self.obs_sensor = self.world.spawn_actor(
            obs_sensor,
            carla.Transform(
                carla.Location(x=2.8, z=1.0),
                carla.Rotation(pitch=5)),
            attach_to=self.vehicle)
        self.actor_list.append(self.obs_sensor)
        self.obs_sensor.listen(lambda obsen: self.obs_data(obsen))
        
        while self.front_camera is None:
            time.sleep(0.01)

        self.episode_start = time.time()
        self.vehicle.apply_control(carla.VehicleControl(throttle=0.0, brake=0.0))

        return self.front_camera

    def collision_data(self, event):
        self.collision_hist.append(event)

    def lane_data(self, lanesen):
        lane_types = set(x.type for x in lanesen.crossed_lane_markings)
        text = ['%r' % str(x).split()[-1] for x in lane_types]
        self.lane_invasion.append(text[0])

    def obs_data(self, obsen):
        for detect in obsen:
            if detect.depth <= float(30.0):
                self.obstacle_distance.append(detect.depth)
    
    def process_img(self, image):
        image.convert(cc.CityScapesPalette)
        i = np.array(image.raw_data, dtype='uint8')
        i2 = i.reshape((self.im_height, self.im_width, 4))
        i3 = i2[:, :, :3]       
        if self.SHOW_CAM:
            cv2.imshow("test", i3)
            cv2.waitKey(1)
        self.front_camera = i3

    def get_rewards(self,angle):
        done = False

        # Assign Reward for Collision
        reward_collision = 0
        if len(self.collision_hist) > 0:
            reward_collision = -6
            done = True

        #Crossing lanes
        reward_lane = 0
        if len(self.lane_invasion) > 0:
            lane_invasion_error = self.lane_invasion.pop()
            if 'Broken' in lane_invasion_error:
                reward_lane += -2
            elif 'Solid' in lane_invasion_error:
                reward_lane += -4

        total_reward = reward_collision + reward_lane
        return total_reward, done


    def step(self, actionIndex, timestep):
        angle = PREDICTED_ANGLES[actionIndex]
        self.vehicle.apply_control(carla.VehicleControl(steer=angle))
        v = self.vehicle.get_velocity()

        kmh = int(3.6 * math.sqrt(v.x**2 + v.y**2 + v.z**2))
        speed_reward = kmh/5

        total_rewards, done = self.get_rewards(angle)
        total_rewards+=speed_reward

        if timestep > MAX_TIMESTEP:
            done = True

        return self.front_camera, total_rewards, done, None
