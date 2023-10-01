import subprocess

import glob
import os
import sys

try:
    sys.path.append(glob.glob('/home/magnus/carla_own/PythonAPI/carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass


# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import carla
import logging



import weakref
import random
import math
import time
import queue
import cv2
import json
import csv

try:
    import pygame
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_SPACE
    from pygame.locals import K_a
    from pygame.locals import K_d
    from pygame.locals import K_s
    from pygame.locals import K_w
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')


SpawnActor = carla.command.SpawnActor


OUTPUT_FOLDER = "_testing_clean" # "_pedestrians_nopatch"
PATCH_PATH = "/home/magnus/Downloads/universal_patch_300_hs_resized.png"
TEXTURE_PATH = "/home/magnus/carla_own/Unreal/CarlaUE4/Content/Package_Attacks/Static/Static/StaticAttackPedestrian/patch_texture.png"
PATCH_OUT = "_patch_out"

if not os.path.exists(os.path.join("./",OUTPUT_FOLDER)):
    os.makedirs(os.path.join("./",OUTPUT_FOLDER))
if not os.path.exists(os.path.join("./",PATCH_OUT)):
    os.makedirs(os.path.join("./",PATCH_OUT))

folder = os.listdir(OUTPUT_FOLDER)

for item in folder:
    if item.endswith(".png"):
        os.remove(os.path.join(OUTPUT_FOLDER, item))

folder = os.listdir(PATCH_OUT)

for item in folder:
    os.remove(os.path.join(PATCH_OUT, item))

EGO_SPAWN_POINT = carla.Transform(carla.Location(x=-5.883884, y=-67.906418, z=0.5), carla.Rotation(pitch=0.0, yaw=180.0, roll=0.0))
PATCH_SPAWN_LIST = [carla.Transform(carla.Location(x=-32.164324, y=-68.229904, z=1.0), carla.Rotation(pitch=0.000000, yaw=0.000000, roll=0.000000)),
                         carla.Transform(carla.Location(x=-30.164324, y=-68.229904, z=1.0), carla.Rotation(pitch=0.000000, yaw=0.000000, roll=0.000000)),
                         carla.Transform(carla.Location(x=-29.164324, y=-68.229904, z=1.0), carla.Rotation(pitch=0.000000, yaw=0.000000, roll=0.000000)),
                         carla.Transform(carla.Location(x=-31.164324, y=-68.229904, z=1.0), carla.Rotation(pitch=0.000000, yaw=0.000000, roll=0.000000)),]


class GTBoundingBoxes(object):
    
    @staticmethod
    def get_bounding_boxes(world, vehicle, camera, K, labels, img=None):
        bounding_boxes = []

        for label in labels:
            bbs = list(world.get_level_bbs(label))
            for bb in bbs:
                bb_verts = GTBoundingBoxes.__filter_bbs(bb, vehicle, camera, K)
                bounding_boxes.append(bb_verts)

                if img is not None and bb_verts:
                    bb_verts.append(label)
                    img = cv2.line(img, (int(bb_verts[0]),int(bb_verts[1])), (int(bb_verts[2]),int(bb_verts[1])), (0,0,255, 255), 1)
                    img = cv2.line(img, (int(bb_verts[0]),int(bb_verts[3])), (int(bb_verts[2]),int(bb_verts[3])), (0,0,255, 255), 1)
                    img = cv2.line(img, (int(bb_verts[0]),int(bb_verts[1])), (int(bb_verts[0]),int(bb_verts[3])), (0,0,255, 255), 1)
                    img = cv2.line(img, (int(bb_verts[2]),int(bb_verts[1])), (int(bb_verts[2]),int(bb_verts[3])), (0,0,255, 255), 1)
        
        if img is not None:
            return bounding_boxes, img
        
        return bounding_boxes

    @staticmethod
    def __filter_bbs(bb, ego_vehicle, camera, K):
        bbox_verts = []

        # Get the camera matrix 
        world_2_camera = np.array(camera.get_transform().get_inverse_matrix())
        image_w = int(camera.attributes["image_size_x"])
        image_h = int(camera.attributes["image_size_y"])

        dist = bb.location.distance(ego_vehicle.get_transform().location)

        # Filter for the vehicles within 50m
        if dist < 50:
            # Calculate the dot product between the forward vector
            # of the vehicle and the vector between the vehicle
            # and the other vehicle. We threshold this dot product
            # to limit to drawing bounding boxes IN FRONT OF THE CAMERA
            forward_vec = ego_vehicle.get_transform().get_forward_vector()
            ray = bb.location - ego_vehicle.get_transform().location

            if forward_vec.dot(ray) > 1:
                verts = [v for v in bb.get_world_vertices(carla.Transform())]

                area = bb.extent.x*2 * bb.extent.z*2

                x_max = -10000
                x_min = 10000
                y_max = -10000
                y_min = 10000

                for vert in verts:
                    p = GTBoundingBoxes.get_image_point(vert, K, world_2_camera)
                    # Find the rightmost vertex
                    if p[0] > x_max:
                        x_max = p[0]
                    # Find the leftmost vertex
                    if p[0] < x_min:
                        x_min = p[0]
                    # Find the highest vertex
                    if p[1] > y_max:
                        y_max = p[1]
                    # Find the lowest  vertex
                    if p[1] < y_min:
                        y_min = p[1]

                    # make sure bb is in image!
                    if x_min > 0 and x_max < image_w and y_min > 0 and y_max < image_h:
                        bbox_verts = [x_min, y_min, x_max, y_max]

        return bbox_verts
                
    @staticmethod
    def get_image_point(loc, K, w2c):
        """
        Calculate 2D projecbb_vertstion of 3D coordinate.

        Input:
            loc: 3D coordinate of object (carla.Position object)
            K: camera projection matrix
            w2c: transformation matrix from point in worl to point in camera coord. system
        Output:
            2D point in image. 
        """

        # Format the input coordinate
        point = np.array([loc.x, loc.y, loc.z, 1])
        # transform to camera coordinates
        point_camera = np.dot(w2c, point)

        # New we must change from UE4's coordinate system to an "standard"
        # (x, y ,z) -> (y, -z, x)
        # and we remove the fourth component also
        point_camera = [point_camera[1], -point_camera[2], point_camera[0]]

        # now project 3D->2D using the camera matrix
        point_img = np.dot(K, point_camera)
        # normalize
        point_img[0] /= point_img[2]
        point_img[1] /= point_img[2]

        return point_img[0:2]


class GetSimPatch(object):
    def __init__(self) -> None:
        self.client = None
        self.world = None
        self.bp_lib = None
        self.map = None
        self.static_attacks = [None, None, None, None]

        self.car = None
        self.camera = None
        self.K = None
        self.image_queue = queue.LifoQueue(maxsize=10)

    def set_synchronous_mode(self, mode):
        # Set up the simulator in synchronous mode
        settings = self.world.get_settings()
        settings.synchronous_mode = mode # Enables/disables synchronous mode
        settings.fixed_delta_seconds = 0.05
        self.world.apply_settings(settings)

    def spawn_ego(self, transform):
        """
        Spawns the ego-vehicle and attaches an RGB camera to it.
        """
        vehicle_bp = self.bp_lib.find('vehicle.lincoln.mkz_2020')
        self.car = self.world.try_spawn_actor(vehicle_bp, transform)

        camera_bp = self.bp_lib.find('sensor.camera.rgb')
        camera_init_trans = carla.Transform(carla.Location(x=1.5, z=2.4))
        self.camera = self.world.spawn_actor(camera_bp, camera_init_trans, attach_to=self.car)
        self.camera.listen(self.image_queue.put)


    def get_camera_matrix(self, w, h, fov):
        """
        Calculate projection matrix of the camera (intrinsics).

        Input:
            w: width of image (px)
            h: height of image (px)
            fov: field of view (deg)
        Output:
            K: 3x3 camera matrix
        """
        focal = w / (2.0 * np.tan(fov * np.pi / 360.0))
        K = np.identity(3)
        K[0, 0] = K[1, 1] = focal
        K[0, 2] = w / 2.0
        K[1, 2] = h / 2.0
        return K
    

    def get_FOI(self):
        l = self.car.get_location()
        car_location = np.array([l.x, l.y, l.z])
        fw = self.car.get_transform().rotation.get_forward_vector().make_unit_vector()
        forward = np.array([fw.x, fw.y, fw.z])
        min_fw = car_location + 6*forward
        max_fw = car_location + 11*forward
        rv = self.car.get_transform().rotation.get_right_vector()
        right = np.array([rv.x, rv.y, rv.z])
        
        verteces = [min_fw-5*right, min_fw+5*right, max_fw-5*right, max_fw+5*right]

        return verteces
    
    def is_in_FOI(self, actor, FOI):
        pos = actor.get_location()
        actor_location = np.array([pos.x, pos.y, pos.z])
        if FOI[2][0] < actor_location[0] < FOI[0][0]:
            if FOI[1][1] < actor_location[1] < FOI[0][1]:
                return True
        return False
    
    def spawn_patches(self, transforms):
        patch_bp = self.bp_lib.find('static.prop.staticattackpedestrian')
        for i in range(4):
            patch_pos = carla.Transform(transforms[i].location)
            if self.static_attacks[i] is not None:
                old_patch = self.world.get_actor(self.static_attacks[i])
                old_patch.destroy()
            patch = self.world.spawn_actor(patch_bp, patch_pos)
            self.static_attacks[i] = patch.id

    def get_patch_from_img(self, frame, patch_id):
        patch_size = 0.447
        patch = self.world.get_actor(patch_id)
        # print(patch)

        # actor = patch.parent
        # actor_world_loc = actor.get_transform().location
        # actor_world_pos = np.array([actor_world_loc.x, actor_world_loc.y, actor_world_loc.z, 1])
        world2cam = np.array(self.camera.get_transform().get_inverse_matrix())
        # actor_cam_pos = np.dot(world2cam, actor_world_pos)
        # actor_cam_pos = [actor_cam_pos[1], -actor_cam_pos[2], actor_cam_pos[0]]

        patch_world_loc = patch.get_transform().location
        
        # print(patch_world_loc)
        patch_fw = patch.get_transform().rotation.get_forward_vector().make_unit_vector()
        print("FW: ", patch_fw)
        theta = np.arctan2(patch_fw.y, patch_fw.x)
        print("THETA ", np.rad2deg(theta))
        horizontal_vec = np.array([-np.sin(theta), np.cos(theta), 0])
        # fw_vec = np.array([patch_fw.x, patch_fw.y, patch_fw.z])
        # c = np.dot(fw_vec, horizontal_vec)/np.linalg.norm(fw_vec)/np.linalg.norm(horizontal_vec)
        # angle = np.arccos(np.clip(c, -1, 1))
        print("HORIZONTAL: ", horizontal_vec)
        margin = 0.0
        patch_top_left = carla.Location(patch_world_loc.x + (patch_size/2 + margin)*horizontal_vec[0],
                          patch_world_loc.y + (patch_size/2 + margin)*horizontal_vec[1],
                          patch_world_loc.z + patch_size/2 + margin)
        patch_bottom_left = carla.Location(patch_world_loc.x + (patch_size/2 + margin)*horizontal_vec[0],
                          patch_world_loc.y + (patch_size/2 + margin)*horizontal_vec[1],
                          patch_world_loc.z - patch_size/2 + margin)
        patch_bottom_right = carla.Location(patch_world_loc.x - (patch_size/2 + margin)*horizontal_vec[0],
                          patch_world_loc.y - (patch_size/2 + margin)*horizontal_vec[1],
                          patch_world_loc.z - patch_size/2 + margin)
        patch_top_right = carla.Location(patch_world_loc.x - (patch_size/2 + margin)*horizontal_vec[0],
                          patch_world_loc.y - (patch_size/2 + margin)*horizontal_vec[1],
                          patch_world_loc.z + patch_size/2 + margin)
        patch_center = carla.Location(patch_world_loc.x, patch_world_loc.y, patch_world_loc.z)
        
        patch_verteces = [patch_top_left, patch_bottom_left, patch_bottom_right, patch_top_right, patch_center]
        patch_vert_img = []
        for vertex in patch_verteces:
            img_point = GTBoundingBoxes.get_image_point(vertex, self.K, world2cam)
            img_point[0] = int(img_point[0])
            img_point[1] = int(img_point[1])
            patch_vert_img.append(img_point)
        
        img = frame.copy()
        top_left = (min(int(patch_vert_img[0][0]), int(patch_vert_img[1][0])), min(int(patch_vert_img[0][1]), int(patch_vert_img[3][1])))
        bottom_left = (min(int(patch_vert_img[0][0]), int(patch_vert_img[1][0])), max(int(patch_vert_img[1][1]), int(patch_vert_img[2][1])))
        bottom_right = (max(int(patch_vert_img[2][0]), int(patch_vert_img[3][0])), max(int(patch_vert_img[1][1]), int(patch_vert_img[2][1])))
        top_right = (max(int(patch_vert_img[2][0]), int(patch_vert_img[3][0])), min(int(patch_vert_img[0][1]), int(patch_vert_img[3][1])))
        center = (int(patch_vert_img[4][0]), int(patch_vert_img[4][1]))
        # print(top_left)
        # cv2.rectangle(img, top_left, bottom_right, (255,0,0), 1)
        # cv2.circle(img, center, 3, (0,0,0), -1)
        # cv2.circle(img, bottom_left, 2, (0,0,255), -1)
        # cv2.circle(img, bottom_right, 2, (0,0,255), -1)
        # cv2.circle(img, top_right, 2, (0,0,255), -1)
        num = len(os.listdir(PATCH_OUT))
        # cv2.imwrite(os.path.join(PATCH_OUT, "patch_{:01d}.png".format(num)), img)
        # print(patch_vert_img)
        patch_cropped = img[min(int(patch_vert_img[0][1]), int(patch_vert_img[3][1])):max(int(patch_vert_img[1][1]), int(patch_vert_img[2][1])),
                            min(int(patch_vert_img[0][0]), int(patch_vert_img[1][0])):max(int(patch_vert_img[2][0]), int(patch_vert_img[3][0]))]
        
        # patch_cropped = img[int(patch_vert_img[4][1])-15:int(patch_vert_img[4][1])+15,
        #                     int(patch_vert_img[4][0])-15:int(patch_vert_img[4][0])+15]
        
        print("SHAPE:   ",patch_cropped.shape)
        if patch_cropped.shape[0] == 0 or patch_cropped.shape[1] == 0:
            print("NVM!!!!")
            return

        cv2.imshow('Patch',patch_cropped)
        cv2.imwrite(os.path.join(PATCH_OUT, "patch_{:01d}.png".format(num+1)), patch_cropped)

    def sim_patches(self):
        try:
            self.client = carla.Client("localhost", 2000)
            self.client.set_timeout(2.0)
            self.world = self.client.get_world()
            self.map = self.world.get_map()
            self.bp_lib = self.world.get_blueprint_library()
            spectator = self.world.get_spectator()

            self.world.reset_all_traffic_lights()

            self.set_synchronous_mode(True)

            spawn_points = self.map.get_spawn_points()

            self.spawn_ego(EGO_SPAWN_POINT)

            self.world.tick()

            # Calculate the camera projection matrix to project from 3D -> 2D and save to camera attributes
            image_w = int(self.camera.attributes["image_size_x"])
            image_h = int(self.camera.attributes["image_size_y"])
            fov = float(self.camera.attributes["fov"])
            self.K = self.get_camera_matrix(image_w, image_h, fov)

            self.car.set_autopilot(True)

            self.world.tick()
            frame_number = 0

            image = self.image_queue.get()
            # Reshape the raw data into an RGB array
            img = np.reshape(np.copy(image.raw_data), (image.height, image.width, 4))

            # Display the image in an OpenCV display window
            cv2.namedWindow('CameraFeed', cv2.WINDOW_AUTOSIZE)
            cv2.imshow('CameraFeed',img)
            cv2.waitKey(1)

            stopatLight = True
            traffic_lights = self.world.get_actors().filter('traffic.traffic_light*')

            # all_pedestrians = self.world.get_actors(self.pedestrian_list)
            # in_FOI = []
            # for i in range(int(len(self.pedestrian_list)/4)):
            #     in_FOI.append(False)

            # for i in range(1, len(self.pedestrian_list), 4):
            #     self.spawn_double_patch(all_pedestrians[i])
            increments = list(np.arange(-3,3.5,0.5))
            index = 0

            print("Lessgooooooo")
            while True:
                self.world.tick()

                frame_number += 1

                frame_file = "{:05d}.png".format(frame_number)

                if stopatLight:
                    waypoint = self.map.get_waypoint(self.car.get_location())
                    waypoints = waypoint.next(30)
                    for light in traffic_lights:
                        for bb in light.get_light_boxes():
                            if bb.location.distance(waypoints[0].transform.location) < 10:
                                print("SETTING RED")
                                light.set_red_time(20)
                                light.set_state(carla.TrafficLightState.Red)
                                stopatLight = False

                if self.car.is_at_traffic_light() and frame_number%5 == 0:
                    for i,tf in enumerate(PATCH_SPAWN_LIST):
                        tf.location.y += increments[(index + i*3)%len(increments)]
                    index+=1

                    self.spawn_patches(PATCH_SPAWN_LIST)

                # Retrieve and reshape the image
                image = self.image_queue.get()
                img = np.reshape(np.copy(image.raw_data), (image.height, image.width, 4))
                bb_img = img.copy()

                for patch in self.static_attacks:
                    if patch is not None:
                        self.get_patch_from_img(img, patch)
                        
                cv2.imwrite(os.path.join(OUTPUT_FOLDER, frame_file), img)

                cv2.imshow('CameraFeed',bb_img)
                if cv2.waitKey(1) == ord('q'):
                    break


                if (index+1)%(2*len(increments)) == 0:
                    break

                continue

        except KeyboardInterrupt:
            pass

        finally:
            self.set_synchronous_mode(False)
            print("Destroying the actors!")

            # Destroy them static attacks
            patches = []
            if self.static_attacks[1] is not None:
                for patch_id in self.static_attacks:
                    patches.append(self.world.get_actor(patch_id))
            
            self.client.apply_batch([carla.command.DestroyActor(x) for x in patches])

            # self.car.destroy()
            # self.camera.destroy()
            if self.car.destroy() and self.camera.destroy():

                print("Destroyed the ego-vehicle and camera.")
            
            cv2.destroyAllWindows()


if __name__ == "__main__":
    try:
        give_me = GetSimPatch()
        give_me.sim_patches()

        # patch = cv2.imread(PATCH_PATH, cv2.IMREAD_UNCHANGED)
        # cv2.imwrite(TEXTURE_PATH, patch)

        # result = subprocess.run(["/home/magnus/carla_own/PythonAPI/attack_scenario/static/shellscript.sh"], shell=True, text=True)
        # # print(result)

        # time.sleep(3)

        # attack_scenario.client.reload_world()
        # attack_scenario = StaticAttackScenario()
        # attack_scenario.game_loop()
    finally:
        print("EXIT!")