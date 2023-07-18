import glob
import os
import sys

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

import weakref
import random
import math
import time
import queue
import cv2
import json

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


# =============================================================================
# -- client -------------------------------------------------------------------
# =============================================================================

class GTBoundingBoxes(object):
    
    @staticmethod
    def get_bounding_boxes(world, vehicle, camera, K, img=None):
        bounding_boxes = []

        # get bounding boxes of all types of vehicles
        all_vehicles_bbs = []
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Car)))
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Truck)))
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Motorcycle)))
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Bicycle)))
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Bus)))
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Rider)))
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Train)))
        
        for i, vehicle_class in enumerate(all_vehicles_bbs):
            for bb in vehicle_class:
                bb_verts = GTBoundingBoxes.__filter_bbs(bb, vehicle, camera, K)
                bounding_boxes.append(bb_verts)

                if img is not None and bb_verts:
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

                x_max = -10000
                x_min = 10000
                y_max = -10000
                y_min = 10000

                for vert in verts:
                    p = GTBoundingBoxes.__get_image_point(vert, K, world_2_camera)
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
    def __get_image_point(loc, K, w2c):
        """
        Calculate 2D projection of 3D coordinate.

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

class StaticAttackScenario(object):
    def __init__(self) -> None:
        self.client = None
        self.world = None
        self.bp_lib = None
        self.vehicle_list = []

        self.car = None
        self.camera = None
        self.K = None
        self.image_queue = queue.Queue()

        self.display = None
        self.image = None
        self.capture = True

    def set_synchronous_mode(self, mode):
        # Set up the simulator in synchronous mode
        settings = self.world.get_settings()
        settings.synchronous_mode = mode # Enables/disables synchronous mode
        settings.fixed_delta_seconds = 0.05
        self.world.apply_settings(settings)

    def spawn_npcs(self):
        pass

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

        self.car.set_autopilot(True)

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

    def game_loop(self):
        try: 
            self.client = carla.Client("localhost", 2000)
            self.client.set_timeout(2.0)
            self.world = self.client.get_world()
            self.bp_lib = self.world.get_blueprint_library()
            # spectator = self.world.get_spectator()

            spawn_points = self.world.get_map().get_spawn_points()

            print(type(self.camera))
            self.spawn_ego(spawn_points[0])
            print(type(self.camera))

            # Calculate the camera projection matrix to project from 3D -> 2D and save to camera attributes
            image_w = int(self.camera.attributes["image_size_x"])
            image_h = int(self.camera.attributes["image_size_y"])
            fov = float(self.camera.attributes["fov"])
            self.K = self.get_camera_matrix(image_w, image_h, fov)

            self.set_synchronous_mode(True)

            self.world.tick()
            image = self.image_queue.get()
            # Reshape the raw data into an RGB array
            img = np.reshape(np.copy(image.raw_data), (image.height, image.width, 4))
            # Display the image in an OpenCV display window
            cv2.namedWindow('CameraFeed', cv2.WINDOW_AUTOSIZE)
            cv2.imshow('CameraFeed',img)
            cv2.waitKey(1)

            while True:
                self.world.tick()

                # Retrieve and reshape the image
                image = self.image_queue.get()
                img = np.reshape(np.copy(image.raw_data), (image.height, image.width, 4))

                bb_params, img = GTBoundingBoxes.get_bounding_boxes(self.world, self.car, self.camera, self.K, img)

                cv2.imshow('CameraFeed',img)
                if cv2.waitKey(1) == ord('q'):
                    break

        except KeyboardInterrupt:
            pass

        finally:
            print("Destroying the actors!")
            for npc in self.vehicle_list:
                carla.command.DestroyActor(npc)

            self.car.destroy()
            self.camera.destroy()
            
            cv2.destroyAllWindows()


if __name__ == "__main__":
    try:
        client = StaticAttackScenario()
        client.game_loop()
    finally:
        print("EXIT!")







            # print("Spwaning in ", custom_spawn_point)
            # for i in range(1):
            #     vehicle_bp = random.choice(self.bp_lib.filter('vehicle'))
            #     npc = self.world.try_spawn_actor(vehicle_bp, custom_spawn_point)
            #     if npc:
            #         self.vehicle_list.append(npc)
            #         # npc.set_autopilot(True)

            # # print(self.world.get_actors())
            # print("Spawned")