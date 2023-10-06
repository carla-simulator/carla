import subprocess

import glob
import os
import sys
import argparse

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


parser = argparse.ArgumentParser()
parser.add_argument("--single",action='store_true',help="perfrom single patch attacks")
parser.add_argument("--double",action='store_true',help="perfrom double patch attacks")
parser.add_argument("--output_dir",default="_test/",type=str,help="relative directory to save frames")
args = parser.parse_args()

if args.single and args.double:
    raise Exception("Can't pass single and double argument at the same time.")

SpawnActor = carla.command.SpawnActor

if not args.output_dir.startswith("_"):
    args.output_dir = "_" + args.output_dir
OUTPUT_FOLDER_PATCHED = os.path.join(args.output_dir, "patched/")
OUTPUT_FOLDER_CLEAN = os.path.join(args.output_dir, "clean/")
PATCH_PATH = "/home/magnus/Downloads/universal_patch_300_hs_resized.png"
TEXTURE_PATH = "/home/magnus/carla_own/Unreal/CarlaUE4/Content/Package_Attacks/Static/Static/StaticAttackPedestrian/patch_texture.png"
PATCH_OUT = "_patch_out"

if not os.path.exists(os.path.join("./",args.output_dir)):
    os.makedirs(os.path.join("./",args.output_dir))
if not os.path.exists(os.path.join("./",OUTPUT_FOLDER_PATCHED)):
    os.makedirs(os.path.join("./",OUTPUT_FOLDER_PATCHED))
if not os.path.exists(os.path.join("./",OUTPUT_FOLDER_CLEAN)):
    os.makedirs(os.path.join("./",OUTPUT_FOLDER_CLEAN))
if not os.path.exists(os.path.join("./",PATCH_OUT)):
    os.makedirs(os.path.join("./",PATCH_OUT))

folders = [OUTPUT_FOLDER_PATCHED, OUTPUT_FOLDER_CLEAN, PATCH_OUT]
for folder in folders:
    for item in os.listdir(folder):
        if item.endswith(".png"):
            os.remove(os.path.join(folder, item))


EGO_SPAWN_POINT = carla.Transform(carla.Location(x=-5.883884, y=-67.906418, z=0.5), carla.Rotation(pitch=0.0, yaw=180.0, roll=0.0))
PEDESTRIAN_SPAWN_LIST = [carla.Transform(carla.Location(x=-32.164324, y=-75.203926, z=0.2), carla.Rotation(pitch=0.000000, yaw=0.000000, roll=0.000000)),
                         carla.Transform(carla.Location(x=-30.164324, y=-47.203926, z=0.2), carla.Rotation(pitch=0.000000, yaw=0.000000, roll=0.000000)),
                         carla.Transform(carla.Location(x=-27.164324, y=-74.203926, z=0.2), carla.Rotation(pitch=0.000000, yaw=0.000000, roll=0.000000)),
                         carla.Transform(carla.Location(x=-31.164324, y=-44.203926, z=0.2), carla.Rotation(pitch=0.000000, yaw=0.000000, roll=0.000000)),]


AREAS = []

# =============================================================================
# -- get bounding boxes -------------------------------------------------------
# =============================================================================

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
                AREAS.append(area)

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
    

# =============================================================================
# -- static attack client -----------------------------------------------------
# =============================================================================


class StaticAttackScenario(object):
    def __init__(self) -> None:
        self.client = None
        self.world = None
        self.bp_lib = None
        self.map = None
        self.vehicle_list = []
        self.pedestrian_list = []
        self.static_attacks = []

        self.car = None
        self.camera = None
        self.K = None
        self.image_queue = queue.LifoQueue(maxsize=10)

        self.ground_truth_annotations = {
            "info": {},

            "licenses": {},

            "images": [],

            "categories": [
                {"supercategory": "vehicle", "id": 1, "name": "car" },
                {"supercategory": "vehicle", "id": 2, "name": "truck" },
                {"supercategory": "vehicle", "id": 3, "name": "motorcycle" },
                {"supercategory": "vehicle", "id": 4, "name": "bicycle" },
                {"supercategory": "vehicle", "id": 5, "name": "bus" },
                {"supercategory": "vehicle", "id": 6, "name": "rider" },
                {"supercategory": "vehicle", "id": 7, "name": "train" },
                {"supercategory": "", "id": 8, "name": "pedestrian" }
            ],

            "annotations": []
        }

        self.objectlabel2categoryid = {
            carla.CityObjectLabel.Car           :   1,
            carla.CityObjectLabel.Truck         :   2,
            carla.CityObjectLabel.Motorcycle    :   3,
            carla.CityObjectLabel.Bicycle       :   4,
            carla.CityObjectLabel.Rider         :   5,
            carla.CityObjectLabel.Bus           :   6,
            carla.CityObjectLabel.Train         :   7,
            carla.CityObjectLabel.Pedestrians   :   8
        }

        self.display = None
        self.image = None
        self.capture = True

        # Get Walker spawn points
        # self.walker_spawn_points = []
        # with open("./../pedestrian_spawn_points.csv", 'r') as file:
        #     csvreader = csv.reader(file)
        #     for i, row in enumerate(csvreader):
        #         if i==0:
        #             continue
        #         spawn_point = carla.Transform()
        #         spawn_point.location.x = float(row[1])
        #         spawn_point.location.y = float(row[2])
        #         spawn_point.location.z = float(row[3])
        #         self.walker_spawn_points.append(spawn_point)
        self.walker_spawn_points = PEDESTRIAN_SPAWN_LIST

    def set_synchronous_mode(self, mode):
        # Set up the simulator in synchronous mode
        settings = self.world.get_settings()
        settings.synchronous_mode = mode # Enables/disables synchronous mode
        settings.fixed_delta_seconds = 0.05
        self.world.apply_settings(settings)

    def spawn_npc_vehicles(self, n):
        """
        Spawns a given number of NPC vehicles.

        Input:
            n: number of NPCs
        """
        spawn_points = self.world.get_map().get_spawn_points()

        for i in range(n):
            vehicle_bp = random.choice(self.bp_lib.filter('vehicle'))
            npc = self.world.try_spawn_actor(vehicle_bp, spawn_points[i+2])
            if npc:
                prop_bp = self.bp_lib.find('static.prop.staticattack')
                bb = npc.bounding_box
                prop_pos = carla.Transform(carla.Location(x=bb.location.x-bb.extent.x, y=0,z=bb.location.z))
                prop = self.world.spawn_actor(prop_bp, prop_pos, attach_to=npc)
                self.vehicle_list.append(npc)
                self.vehicle_list.append(prop)
                npc.set_autopilot(True)

        print("Spawned {} NPC vehicles.".format(n))

    def spawn_npc_pedestrians(self, n, hidePatch=False):
        """
        Spawns a given number of NPC pedestrians.

        Input:
            n: number of NPCs
        """
        # -------------
        # Spawn Walkers
        # -------------
        # some settings
        percentagePedestriansRunning = 0.0      # how many pedestrians will run
        percentagePedestriansCrossing = 1.0     # how many pedestrians will walk through the road
        self.world.set_pedestrians_seed(42)
        # 1. take all the random locations to spawn
        spawn_points = []
        for i in range(n):
            spawn_point = carla.Transform()
            loc = self.world.get_random_location_from_navigation()
            if (loc != None):
                spawn_point.location = loc
                spawn_points.append(spawn_point)
        # 2. we spawn the walker object
        batch = []
        walker_speed = []
        # for spawn_point in self.walker_spawn_point:
        for i in range(n):
            spawn_point = self.walker_spawn_points[i] #spawn_points[i]
            walker_bp = self.bp_lib.filter('walker')[i%len(self.bp_lib.filter('walker'))]
            if walker_bp.has_attribute("age"):
                # print(walker_bp.get_attribute("age").as_str())
                if walker_bp.get_attribute("age").as_str() == "child":
                    walker_bp = self.bp_lib.filter('walker')[i%len(self.bp_lib.filter('walker')) + 2]
            # walker_bp = random.choice(self.bp_lib.filter('walker'))
            # set as not invincible
            if walker_bp.has_attribute('is_invincible'):
                walker_bp.set_attribute('is_invincible', 'false')
            # set the max speed
            if walker_bp.has_attribute('speed'):
                if (random.random() > percentagePedestriansRunning):
                    # walking
                    walker_speed.append(walker_bp.get_attribute('speed').recommended_values[1])
                else:
                    # running
                    walker_speed.append(walker_bp.get_attribute('speed').recommended_values[2])
            else:
                print("Walker has no speed")
                walker_speed.append(0.0)

            batch.append(SpawnActor(walker_bp, spawn_point))

        results = self.client.apply_batch_sync(batch, True)
        walker_speed2 = []
        walkers_list = []
        for i in range(len(results)):
            if results[i].error:
                logging.error(results[i].error)
            else:
                walkers_list.append({"id": results[i].actor_id})
                walker_speed2.append(walker_speed[i])
        walker_speed = walker_speed2
        # 3. we spawn the walker controller
        batch = []
        num_walkers = len(walkers_list)
        walker_controller_bp = self.bp_lib.find('controller.ai.walker')
        for i in range(num_walkers):
            batch.append(SpawnActor(walker_controller_bp, carla.Transform(), walkers_list[i]["id"]))
        results = self.client.apply_batch_sync(batch, True)
        for i in range(len(results)):
            if results[i].error:
                logging.error(results[i].error) 
            else:
                walkers_list[i]["con"] = results[i].actor_id
        # Spawn attack patches and attach the to the walkers
        batch = []
        patch_bp = self.bp_lib.find('static.prop.staticattackpedestrian')
        for i in range(num_walkers):
            walker = self.world.get_actor(walkers_list[i]["id"])
            bb_height = walker.bounding_box.extent.z
            if hidePatch:
                patch_pos = carla.Transform(carla.Location(x=-0.2, y=0,z=-10))
            else:
                if bb_height > 0 and bb_height < 2.5:
                    patch_pos = carla.Transform(carla.Location(x=0.0, y=0.3,z=0.45*bb_height), carla.Rotation(yaw=90))
                else:
                    patch_pos = carla.Transform(carla.Location(x=0.0, y=0.3,z=0.45), carla.Rotation(yaw=90))
                    # patch_pos = carla.Transform(carla.Location(x=-0.2, y=0,z=0.45))
            batch.append(SpawnActor(patch_bp, patch_pos, walkers_list[i]["id"]))
        results = self.client.apply_batch_sync(batch, True)
        for i in range(len(results)):
            if results[i].error:
                logging.error(results[i].error)
            else:
                walkers_list[i]["patch"] = results[i].actor_id
        batch = []
        for i in range(num_walkers):
            walker = self.world.get_actor(walkers_list[i]["id"])
            bb_height = walker.bounding_box.extent.z
            if hidePatch:
                patch2_pos = carla.Transform(carla.Location(x=0.2, y=0,z=-10))
            else:
                if bb_height > 0 and bb_height < 2.5:
                    patch2_pos = carla.Transform(carla.Location(x=0.0, y=-0.3,z=0.45*bb_height), carla.Rotation(yaw=90))
                    # patch2_pos = carla.Transform(carla.Location(x=0.2, y=0,z=0.45*bb_height))
                else:
                    patch_pos = carla.Transform(carla.Location(x=0.0, y=-0.3,z=0.45), carla.Rotation(yaw=90))
                    # patch2_pos = carla.Transform(carla.Location(x=0.2, y=0,z=0.45))
            batch.append(SpawnActor(patch_bp, patch2_pos, walkers_list[i]["id"]))
        results = self.client.apply_batch_sync(batch, True)
        for i in range(len(results)):
            if results[i].error:
                logging.error(results[i].error)
            else:
                walkers_list[i]["patch_front"] = results[i].actor_id
        # 4. we put together the walkers and controllers id to get the objects from their id
        for i in range(len(walkers_list)):
            self.pedestrian_list.append(walkers_list[i]["con"])
            self.pedestrian_list.append(walkers_list[i]["id"])
            self.pedestrian_list.append(walkers_list[i]["patch"])
            self.pedestrian_list.append(walkers_list[i]["patch_front"])
        all_pedestrians = self.world.get_actors(self.pedestrian_list)

        self.world.tick()

        # 5. initialize each controller and set target to walk to (list is [controler, actor, controller, actor ...])
        # set how many pedestrians can cross the road
        self.world.set_pedestrians_cross_factor(percentagePedestriansCrossing)
        for i in range(0, len(self.pedestrian_list), 4):
            # start walker
            all_pedestrians[i].start()
            # set walk to random point
            # direction = carla.Vector3D()
            # direction.x = all_pedestrians[i].get_transform().location.x - self.walker_spawn_points[int(i/4)-1].location.x
            # direction.y = all_pedestrians[i].get_transform().location.y - self.walker_spawn_points[int(i/4)-1].location.y
            # direction.z = all_pedestrians[i].get_transform().location.z - self.walker_spawn_points[int(i/4)-1].location.z
            # controller = carla.WalkerControl(direction, 3.0, jump=True)
            # all_pedestrians[i].apply_control(controller)
            all_pedestrians[i].go_to_location(self.walker_spawn_points[int(i/4)-1].location)
            #all_pedestrians[i].go_to_location(self.world.get_random_location_from_navigation())
            # max speed
            all_pedestrians[i].set_max_speed(float(walker_speed[int(i/4)]))

    def spawn_static_attack_parked(self):
        parked_vehicles = self.world.get_environment_objects(carla.CityObjectLabel.Car)
        patch_bp = self.bp_lib.find('static.prop.staticattack')
        spectator = self.world.get_spectator()
        for parked_car in parked_vehicles:
            bb = parked_car.bounding_box
            patch_pos = carla.Transform(carla.Location(x=bb.location.x+bb.extent.x, y=bb.location.y,z=bb.location.z))
            patch = self.world.spawn_actor(patch_bp, patch_pos)
            # spectator.set_transform(patch_pos)
            self.static_attacks.append(patch)

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
    
    def spawn_patch(self, actor):
        patch_bp = self.bp_lib.find('static.prop.staticattackpedestrian')
        fw = self.car.get_transform().rotation.get_forward_vector()
        car_forward = np.array([fw.x, fw.y])
        ped_wrld_loc = actor.get_transform().location
        ped_wrld_vec = np.array([ped_wrld_loc.x, ped_wrld_loc.y, ped_wrld_loc.z, 1])
        world2cam = np.array(self.camera.get_transform().get_inverse_matrix())
        ped_cam_vec = np.dot(world2cam, ped_wrld_vec)
        alpha = np.arctan2(ped_cam_vec[1], ped_cam_vec[0])
        # if alpha > math.pi:
        #     alpha -= 2*math.pi
        # elif alpha <= math.pi:
        #     alpha += 2*math.pi
        alpha_deg = np.rad2deg(alpha)
        # print("RELATIVE TO CAM: ",ped_cam_vec)
        fw_p = actor.get_transform().rotation.get_forward_vector()
        ped_forward = np.array([fw_p.x, fw_p.y])
        theta = np.arctan2(ped_forward[1], ped_forward[0]) - np.arctan2(car_forward[1], car_forward[0])
        if theta > math.pi:
            theta -= 2*math.pi
        elif theta <= -math.pi:
            theta += 2*math.pi
        theta_deg = np.rad2deg(theta)
        angle = theta_deg+alpha_deg
        if angle > 180:
            angle -= 360
        elif angle < -180:
            angle += 360
        # print("ANGLE: ", -angle)
        patch_pos = carla.Transform(carla.Location(x=np.cos(theta)*0.3, y=np.sin(theta)*0.3, z=0.0), carla.Rotation(yaw=angle))
        for i in range(1,len(self.pedestrian_list),4):
            if self.pedestrian_list[i] == actor.id:
                print("Spawning")
                patch = self.world.spawn_actor(patch_bp, patch_pos, attach_to=actor)
                pseudo_patch = self.world.get_actor(self.pedestrian_list[i+1])
                # print(self.pedestrian_list[i+1])
                # print("OLD PSEUDO: ",pseudo_patch)
                pseudo_patch.destroy()
                self.pedestrian_list[i+1] = patch.id
                # print("NEW PATCH: ",patch.id)

    def spawn_double_patch(self, actor, delta=0.1):
        patch_bp = self.bp_lib.find('static.prop.staticattackpedestrian')
        fw = self.car.get_transform().rotation.get_forward_vector()
        car_forward = np.array([fw.x, fw.y])
        fw_p = actor.get_transform().rotation.get_forward_vector()
        ped_forward = np.array([fw_p.x, fw_p.y])
        theta = np.arctan2(ped_forward[1], ped_forward[0]) - np.arctan2(car_forward[1], car_forward[0])
        if theta > math.pi:
            theta -= 2*math.pi
        elif theta <= math.pi:
            theta += 2*math.pi
        theta_deg = np.rad2deg(theta)
        bb_height = 2*actor.bounding_box.extent.z
        bb_width = 2*actor.bounding_box.extent.y*np.sin(theta) + 2*actor.bounding_box.extent.x*np.cos(theta)
        x1 = np.cos(theta)*0.3-delta*bb_width*np.sin(abs(theta))
        y1 = np.sin(theta)*0.3-delta*bb_width*np.cos(abs(theta))
        z1 = delta*bb_height
        patch1_pos = carla.Transform(carla.Location(x=x1, y=y1, z=z1), carla.Rotation(yaw=theta_deg))
        x2 = np.cos(theta)*0.35+delta*bb_width*np.sin(abs(theta))
        y2 = np.sin(theta)*0.35+delta*bb_width*np.cos(abs(theta))
        z2 = -delta*bb_height
        patch2_pos = carla.Transform(carla.Location(x=x2, y=y2, z=z2), carla.Rotation(yaw=theta_deg))
        for i in range(1,len(self.pedestrian_list),4):
            if self.pedestrian_list[i] == actor.id:
                print("Spawning double")
                patch = self.world.spawn_actor(patch_bp, patch1_pos, attach_to=actor)
                pseudo_patch = self.world.get_actor(self.pedestrian_list[i+1])
                pseudo_patch.destroy()
                self.pedestrian_list[i+1] = patch.id
                patch = self.world.spawn_actor(patch_bp, patch2_pos, attach_to=actor)
                pseudo_patch = self.world.get_actor(self.pedestrian_list[i+2])
                pseudo_patch.destroy()
                self.pedestrian_list[i+2] = patch.id

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
        if patch_world_loc.z <= 0:
            print(patch)
            return
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
        # patch_cropped = img[min(int(patch_vert_img[0][1]), int(patch_vert_img[3][1])):max(int(patch_vert_img[1][1]), int(patch_vert_img[2][1])),
        #                     min(int(patch_vert_img[0][0]), int(patch_vert_img[1][0])):max(int(patch_vert_img[2][0]), int(patch_vert_img[3][0]))]
        
        patch_cropped = img[int(patch_vert_img[4][1])-15:int(patch_vert_img[4][1])+15,
                            int(patch_vert_img[4][0])-15:int(patch_vert_img[4][0])+15]
        
        print("SHAPE:   ",patch_cropped.shape)
        if patch_cropped.shape[0] == 0 or patch_cropped.shape[1] == 0:
            print("NVM!!!!")
            return

        cv2.imshow('Patch',patch_cropped)
        cv2.imwrite(os.path.join(PATCH_OUT, "patch_{:01d}.png".format(num+1)), patch_cropped)
        

    def game_loop(self, attack):
        try: 
            self.client = carla.Client("localhost", 2000)
            self.client.set_timeout(2.0)
            self.world = self.client.get_world()
            self.map = self.world.get_map()
            self.bp_lib = self.world.get_blueprint_library()
            spectator = self.world.get_spectator()
            # tf = carla.Transform(carla.Location(x=-32.164324, y=-75.203926, z=1.0), carla.Rotation(pitch=0.000000, yaw=0.000000, roll=0.000000))
            # spectator.set_transform(tf)
            labels = [carla.CityObjectLabel.Car,
                      carla.CityObjectLabel.Truck,
                      carla.CityObjectLabel.Motorcycle,
                      carla.CityObjectLabel.Bicycle,
                      carla.CityObjectLabel.Rider,
                      carla.CityObjectLabel.Bus,
                      carla.CityObjectLabel.Train,
                      carla.CityObjectLabel.Pedestrians]

            self.spawn_static_attack_parked()

            self.world.reset_all_traffic_lights()

            self.set_synchronous_mode(True)

            # self.spawn_npc_vehicles(5)
            self.spawn_npc_pedestrians(4, hidePatch=True)

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

            all_pedestrians = self.world.get_actors(self.pedestrian_list)
            in_FOI = []
            for i in range(int(len(self.pedestrian_list)/4)):
                in_FOI.append(False)

            # for i in range(1, len(self.pedestrian_list), 4):
            #     self.spawn_double_patch(all_pedestrians[i])


            while True:
                self.world.tick()

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

                frame_number += 1

                # Retrieve and reshape the image
                image = self.image_queue.get()
                img = np.reshape(np.copy(image.raw_data), (image.height, image.width, 4))
                bb_img = img.copy()

                FOI = self.get_FOI()

                if attack:
                    for i in range(1, len(self.pedestrian_list), 4):
                        if self.is_in_FOI(all_pedestrians[i], FOI):
                            if not(in_FOI[int(i/4)]):

                                if args.single:
                                    self.spawn_patch(all_pedestrians[i])
                                elif args.double:
                                    self.spawn_double_patch(all_pedestrians[i])
                                else:
                                    self.spawn_patch(all_pedestrians[i])

                                in_FOI[int(i/4)] = True
                            # elif in_FOI[int(i/4)]:
                            #     # pass
                            #     self.get_patch_from_img(img, self.pedestrian_list[i+1])
                            #     self.spawn_patch(all_pedestrians[i])
                        else:
                            if in_FOI[int(i/4)]:
                                in_FOI[int(i/4)] = False

                # Save frame to annotations json
                frame_file = "{:05d}.png".format(frame_number)

                self.ground_truth_annotations["images"].append({
                    "file_name": frame_file,
                    "height": image.height,
                    "width": image.width,
                    "id": frame_number
                })

                bounding_boxes, bb_img = GTBoundingBoxes.get_bounding_boxes(self.world, self.car, self.camera, self.K, labels, bb_img)

                for bb_verts in bounding_boxes:
                    if bb_verts:
                        bb_cocoFormat = [bb_verts[0], bb_verts[1], bb_verts[2]-bb_verts[0], bb_verts[3]-bb_verts[1]]
                        category = self.objectlabel2categoryid[bb_verts[-1]]
                        self.ground_truth_annotations["annotations"].append({
                            "segmentation": [],
                            "area": bb_cocoFormat[2]*bb_cocoFormat[3],
                            "iscrowd": 0,
                            "category_id": category,
                            "image_id": frame_number,
                            "bbox": bb_cocoFormat
                        })

                if attack:
                    cv2.imwrite(os.path.join(OUTPUT_FOLDER_PATCHED, frame_file), img)
                else:
                    cv2.imwrite(os.path.join(OUTPUT_FOLDER_CLEAN, frame_file), img)

                cv2.imshow('CameraFeed',bb_img)
                if cv2.waitKey(1) == ord('q'):
                    break

                if frame_number >= 380:
                    break

        except (KeyboardInterrupt, Exception):
            pass

        finally:
            self.set_synchronous_mode(False)
            print("Destroying the actors!")

            # Destroy them static attacks
            self.client.apply_batch([carla.command.DestroyActor(x) for x in self.static_attacks])
            
            # Destroy them vehicles
            self.client.apply_batch([carla.command.DestroyActor(x) for x in self.vehicle_list])

            # Stop walker controllers
            all_pedestrians = self.world.get_actors(self.pedestrian_list)
            for i in range(0, len(self.pedestrian_list), 4):
                all_pedestrians[i].stop()
            # Destroy them pedestrians
            print(len(self.pedestrian_list))
            self.client.apply_batch([carla.command.DestroyActor(x) for x in self.pedestrian_list])

            # self.car.destroy()
            # self.camera.destroy()
            if self.car.destroy() and self.camera.destroy():

                print("Destroyed {} vehicles, {} walkers and the ego-vehicle and camera.".format(len(self.vehicle_list), int(len(self.pedestrian_list)/4)))
            
            cv2.destroyAllWindows()
            
            print("Saving annotations to json file.")
            if attack:
                with open(os.path.join(OUTPUT_FOLDER_PATCHED, 'annotations.json'), 'w') as json_file:
                    json.dump(self.ground_truth_annotations, json_file)
            else:
                with open(os.path.join(OUTPUT_FOLDER_CLEAN, 'annotations.json'), 'w') as json_file:
                    json.dump(self.ground_truth_annotations, json_file)
            

            average_area = 0
            for area in AREAS:
                average_area += area
            average_area = average_area/len(AREAS)

            print(average_area)


if __name__ == "__main__":
    try:
        attack_scenario = StaticAttackScenario()
        attack_scenario.game_loop(attack=True)

        # patch = cv2.imread(PATCH_PATH, cv2.IMREAD_UNCHANGED)
        # cv2.imwrite(TEXTURE_PATH, patch)

        # result = subprocess.run(["/home/magnus/carla_own/PythonAPI/attack_scenario/static/shellscript.sh"], shell=True, text=True)
        # # print(result)

        time.sleep(3)

        attack_scenario.client.reload_world()
        attack_scenario = StaticAttackScenario()
        attack_scenario.game_loop(attack=False)
    finally:
        print("EXIT!")