### Program to test the carla_vehicle_annotator.py functions performance
### By Mukhlas Adib
### Based example from CARLA Github client_bounding_boxes.py
### 2020
### Last tested on CARLA 0.9.8

### This program is a modified version of CARLA example program client_bounding_boxes.py
### Except functions that convert 3D bounding boxes to 2D bounding boxes
### CARLA Simulator and client_bounding_boxes.py are licensed under the terms of the MIT license
### For a copy, see <https://opensource.org/licenses/MIT>
### For more information about CARLA Simulator, visit https://carla.org/

"""
An example of client-side bounding boxes with basic car controls.

Controls:

    W            : throttle
    S            : brake
    AD           : steer
    Space        : hand-brake
    Q            : debugging (show the depth image and try to save the output)
    
    ESC          : quit
"""

# ==============================================================================
# -- find carla module ---------------------------------------------------------
# ==============================================================================


import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    print('carla not found')
    pass


# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import carla
import carla_vehicle_annotator
import cv2
from matplotlib import pyplot as plt

import weakref
import random
import queue

try:
    import pygame
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_SPACE
    from pygame.locals import K_a
    from pygame.locals import K_d
    from pygame.locals import K_s
    from pygame.locals import K_w
    from pygame.locals import K_q
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

VIEW_WIDTH = 1920//2
VIEW_HEIGHT = 1080//2
VIEW_FOV = 90

BB_COLOR = (248, 64, 24)
D_COLOR = (0, 0, 240)

depth_queue = queue.Queue()

# ==============================================================================
# -- set filter parameters -----------------------------------------------------
# ==============================================================================

max_distance = 100
depth_margin = -1
patch_ratio = 0.5
resize_ratio=0.5


# ==============================================================================
# -- ClientSideBoundingBoxes ---------------------------------------------------
# ==============================================================================


class ClientSideBoundingBoxes(object):
    """
    This is a module responsible for creating 3D bounding boxes and drawing them
    client-side on pygame surface.
    """

    @staticmethod
    def get_bounding_boxes(vehicles, camera, carla_rgb, depth_capture):
        """
        Creates 3D bounding boxes based on carla vehicle list and camera.
        """

        save_out = depth_capture

        if not depth_queue.empty():
            depth_img = depth_queue.get()
            depth_img.convert(carla.ColorConverter.Depth)
            depth_meter = np.array(depth_img.raw_data).reshape((VIEW_HEIGHT,VIEW_WIDTH,4))[:,:,0] * 1000 / 255
        else:
            depth_meter = np.array([[0]])
        vehicles = carla_vehicle_annotator.filter_angle_distance(vehicles , camera, max_distance)
        bounding_boxes_3d = [ClientSideBoundingBoxes.get_bounding_box(vehicle, camera) for vehicle in vehicles]
        bounding_boxes_2d = [carla_vehicle_annotator.p3d_to_p2d_bb(bbox) for bbox in bounding_boxes_3d]
        vehicle_class = carla_vehicle_annotator.get_vehicle_class(vehicles, 'vehicle_class_json_file.txt')
        filtered_out,removed_out,depth_area,depth_capture = carla_vehicle_annotator.filter_occlusion_bbox(bounding_boxes_2d,vehicles,camera,depth_meter,vehicle_class,depth_capture,depth_margin, patch_ratio, resize_ratio)
        if save_out:
            carla_vehicle_annotator.save_output(carla_rgb, filtered_out['bbox'], filtered_out['class'], removed_out['bbox'], removed_out['class'], save_patched=True, out_format='json')
            #carla_vehicle_annotator.save2darknet(filtered_out['bbox'], filtered_out['class'], carla_rgb, save_train=True)
        
        return filtered_out['bbox'] , depth_area, depth_capture

    @staticmethod
    def draw_bounding_boxes(display, bounding_boxes, BB = False):
        """
        Draws bounding boxes on pygame display.
        """

        bb_surface = pygame.Surface((VIEW_WIDTH, VIEW_HEIGHT))
        bb_surface.set_colorkey((0, 0, 0))
        if BB:
            DR_COLOR = BB_COLOR
        else:
            DR_COLOR = D_COLOR
        for p2d_points in bounding_boxes:
            point_0 = [p2d_points[0,0] , p2d_points[0,1]]
            point_1 = [p2d_points[1,0] , p2d_points[0,1]]
            point_2 = [p2d_points[1,0] , p2d_points[1,1]]
            point_3 = [p2d_points[0,0] , p2d_points[1,1]]
            # draw lines
            pygame.draw.line(bb_surface, DR_COLOR, point_0, point_1)
            pygame.draw.line(bb_surface, DR_COLOR, point_1, point_2)
            pygame.draw.line(bb_surface, DR_COLOR, point_2, point_3)
            pygame.draw.line(bb_surface, DR_COLOR, point_3, point_0)
            
        display.blit(bb_surface, (0, 0))

    @staticmethod
    def get_bounding_box(vehicle, camera):
        """
        Returns 3D bounding box for a vehicle based on camera view.
        """

        bb_cords = ClientSideBoundingBoxes._create_bb_points(vehicle)
        cords_x_y_z = ClientSideBoundingBoxes._vehicle_to_sensor(bb_cords, vehicle, camera)[:3, :]
        cords_y_minus_z_x = np.concatenate([cords_x_y_z[1, :], -cords_x_y_z[2, :], cords_x_y_z[0, :]])
        bbox = np.transpose(np.dot(camera.calibration, cords_y_minus_z_x))
        camera_bbox = np.concatenate([bbox[:, 0] / bbox[:, 2], bbox[:, 1] / bbox[:, 2], bbox[:, 2]], axis=1)
        return camera_bbox

    @staticmethod
    def _create_bb_points(vehicle):
        """
        Returns 3D bounding box for a vehicle.
        """

        cords = np.zeros((8, 4))
        extent = vehicle.bounding_box.extent
        cords[0, :] = np.array([extent.x, extent.y, -extent.z, 1])
        cords[1, :] = np.array([-extent.x, extent.y, -extent.z, 1])
        cords[2, :] = np.array([-extent.x, -extent.y, -extent.z, 1])
        cords[3, :] = np.array([extent.x, -extent.y, -extent.z, 1])
        cords[4, :] = np.array([extent.x, extent.y, extent.z, 1])
        cords[5, :] = np.array([-extent.x, extent.y, extent.z, 1])
        cords[6, :] = np.array([-extent.x, -extent.y, extent.z, 1])
        cords[7, :] = np.array([extent.x, -extent.y, extent.z, 1])
        return cords

    @staticmethod
    def _vehicle_to_sensor(cords, vehicle, sensor):
        """
        Transforms coordinates of a vehicle bounding box to sensor.
        """

        world_cord = ClientSideBoundingBoxes._vehicle_to_world(cords, vehicle)
        sensor_cord = ClientSideBoundingBoxes._world_to_sensor(world_cord, sensor)
        return sensor_cord

    @staticmethod
    def _vehicle_to_world(cords, vehicle):
        """
        Transforms coordinates of a vehicle bounding box to world.
        """

        bb_transform = carla.Transform(vehicle.bounding_box.location)
        bb_vehicle_matrix = ClientSideBoundingBoxes.get_matrix(bb_transform)
        vehicle_world_matrix = ClientSideBoundingBoxes.get_matrix(vehicle.get_transform())
        bb_world_matrix = np.dot(vehicle_world_matrix, bb_vehicle_matrix)
        world_cords = np.dot(bb_world_matrix, np.transpose(cords))
        return world_cords

    @staticmethod
    def _world_to_sensor(cords, sensor):
        """
        Transforms world coordinates to sensor.
        """

        sensor_world_matrix = ClientSideBoundingBoxes.get_matrix(sensor.get_transform())
        world_sensor_matrix = np.linalg.inv(sensor_world_matrix)
        sensor_cords = np.dot(world_sensor_matrix, cords)
        return sensor_cords

    @staticmethod
    def get_matrix(transform):
        """
        Creates matrix from carla transform.
        """

        rotation = transform.rotation
        location = transform.location
        c_y = np.cos(np.radians(rotation.yaw))
        s_y = np.sin(np.radians(rotation.yaw))
        c_r = np.cos(np.radians(rotation.roll))
        s_r = np.sin(np.radians(rotation.roll))
        c_p = np.cos(np.radians(rotation.pitch))
        s_p = np.sin(np.radians(rotation.pitch))
        matrix = np.matrix(np.identity(4))
        matrix[0, 3] = location.x
        matrix[1, 3] = location.y
        matrix[2, 3] = location.z
        matrix[0, 0] = c_p * c_y
        matrix[0, 1] = c_y * s_p * s_r - s_y * c_r
        matrix[0, 2] = -c_y * s_p * c_r - s_y * s_r
        matrix[1, 0] = s_y * c_p
        matrix[1, 1] = s_y * s_p * s_r + c_y * c_r
        matrix[1, 2] = -s_y * s_p * c_r + c_y * s_r
        matrix[2, 0] = s_p
        matrix[2, 1] = -c_p * s_r
        matrix[2, 2] = c_p * c_r
        return matrix


# ==============================================================================
# -- BasicSynchronousClient ----------------------------------------------------
# ==============================================================================


class BasicSynchronousClient(object):
    """
    Basic implementation of a synchronous client.
    """

    def __init__(self):
        self.client = None
        self.world = None
        self.camera = None
        self.depth = None
        self.car = None
        self.npc_list = []
        self.spawn_points = []

        self.display = None
        self.image = None
        self.capture = True
        self.depth_dbg = False

    def camera_blueprint(self):
        """
        Returns camera blueprint.
        """

        camera_bp = self.world.get_blueprint_library().find('sensor.camera.rgb')
        camera_bp.set_attribute('image_size_x', str(VIEW_WIDTH))
        camera_bp.set_attribute('image_size_y', str(VIEW_HEIGHT))
        camera_bp.set_attribute('fov', str(VIEW_FOV))
        return camera_bp

    def depth_blueprint(self):
        """
        Returns depth blueprint.
        """

        depth_bp = self.world.get_blueprint_library().find('sensor.camera.depth')
        depth_bp.set_attribute('image_size_x', str(VIEW_WIDTH))
        depth_bp.set_attribute('image_size_y', str(VIEW_HEIGHT))
        depth_bp.set_attribute('fov', str(VIEW_FOV))
        return depth_bp

    def set_synchronous_mode(self, synchronous_mode):
        """
        Sets synchronous mode.
        """

        settings = self.world.get_settings()
        settings.synchronous_mode = synchronous_mode
        self.world.apply_settings(settings)

    def setup_car(self):
        """
        Spawns actor-vehicle to be controled.
        """

        car_bp = self.world.get_blueprint_library().filter('vehicle.*')[0]
        location = random.choice(self.spawn_points)
        self.car = self.world.spawn_actor(car_bp, location)

    def setup_npc(self, number_of_npc=50):
        self.spawn_points = self.world.get_map().get_spawn_points()
        number_of_spawn_points = len(self.spawn_points)

        if number_of_npc < number_of_spawn_points:
            random.shuffle(self.spawn_points)
        elif number_of_npc > number_of_spawn_points:
            number_of_npc = number_of_spawn_points

        blueprints = self.world.get_blueprint_library().filter('vehicle.*')
        batch = []
        for n, transform in enumerate(self.spawn_points):
            if n >= number_of_npc:
                break
            blueprint = random.choice(blueprints)
            batch.append(carla.command.SpawnActor(blueprint, transform))
            self.spawn_points.pop(0)

        for response in self.client.apply_batch_sync(batch):
            self.npc_list.append(response.actor_id)
                
    def setup_camera(self):
        """
        Spawns actor-camera to be used to render view.
        Sets calibration for client-side boxes rendering.
        """

        camera_transform = carla.Transform(carla.Location(x=1.5, z=2.4))
        
        self.camera = self.world.spawn_actor(self.camera_blueprint(), camera_transform, attach_to=self.car)
        weak_self = weakref.ref(self)
        self.camera.listen(lambda image: weak_self().set_image(weak_self, image))

        self.depth = self.world.spawn_actor(self.depth_blueprint(), camera_transform, attach_to=self.car)
        self.depth.listen(depth_queue.put)

        calibration = np.identity(3)
        calibration[0, 2] = VIEW_WIDTH / 2.0
        calibration[1, 2] = VIEW_HEIGHT / 2.0
        calibration[0, 0] = calibration[1, 1] = VIEW_WIDTH / (2.0 * np.tan(VIEW_FOV * np.pi / 360.0))
        self.camera.calibration = calibration

    def control(self, car):
        """
        Applies control to main car based on pygame pressed keys.
        Will return True If ESCAPE is hit, otherwise False to end main loop.
        """

        keys = pygame.key.get_pressed()
        if keys[K_ESCAPE]:
            return True
        if keys[K_q]:
            self.depth_dbg = True

        control = car.get_control()
        control.throttle = 0
        if keys[K_w]:
            control.throttle = 1
            control.reverse = False
        elif keys[K_s]:
            control.throttle = 1
            control.reverse = True
        if keys[K_a]:
            control.steer = max(-1., min(control.steer - 0.05, 0))
        elif keys[K_d]:
            control.steer = min(1., max(control.steer + 0.05, 0))
        else:
            control.steer = 0
        control.hand_brake = keys[K_SPACE]

        car.apply_control(control)
        return False

    @staticmethod
    def set_image(weak_self, img):
        """
        Sets image coming from camera sensor.
        The self.capture flag is a mean of synchronization - once the flag is
        set, next coming image will be stored.
        """

        self = weak_self()
        if self.capture:
            self.image = img
            self.capture = False

    def render(self, display):
        """
        Transforms image from camera sensor and blits it to main pygame display.
        """

        if self.image is not None:
            array = np.frombuffer(self.image.raw_data, dtype=np.dtype("uint8"))
            array = np.reshape(array, (self.image.height, self.image.width, 4))
            array = array[:, :, :3]
            array = array[:, :, ::-1]
            surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))
            display.blit(surface, (0, 0))

    def game_loop(self):
        """
        Main program loop.
        """

        try:
            pygame.init()

            self.client = carla.Client('127.0.0.1', 2000)
            self.client.set_timeout(2.0)
            self.world = self.client.get_world()

            self.setup_npc()
            self.setup_car()
            self.setup_camera()

            self.display = pygame.display.set_mode((VIEW_WIDTH, VIEW_HEIGHT), pygame.HWSURFACE | pygame.DOUBLEBUF)
            pygame_clock = pygame.time.Clock()

            self.set_synchronous_mode(True)
            vehicles = self.world.get_actors().filter('vehicle.*')

            while True:
                self.world.tick()

                self.capture = True
                pygame_clock.tick_busy_loop(20)

                self.render(self.display)
                bounding_boxes, depth_patches, self.depth_dbg = ClientSideBoundingBoxes.get_bounding_boxes(vehicles, self.camera, self.image, self.depth_dbg)
                ClientSideBoundingBoxes.draw_bounding_boxes(self.display, depth_patches)
                ClientSideBoundingBoxes.draw_bounding_boxes(self.display, bounding_boxes, BB = True)

                pygame.display.flip()

                pygame.event.pump()
                if self.control(self.car):
                    return

        finally:
            self.set_synchronous_mode(False)
            self.camera.destroy()
            self.car.destroy()
            self.depth.destroy()
            self.client.apply_batch([carla.command.DestroyActor(x) for x in self.npc_list])
            self.client.apply_batch([carla.command.DestroyActor(x) for x in self.world.get_actors().filter('vehicle.*')])
            pygame.quit()


# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================


def main():
    """
    Initializes the client-side bounding box demo.
    """

    try:
        client = BasicSynchronousClient()
        client.game_loop()
    finally:
        print('EXIT')


if __name__ == '__main__':
    main()
