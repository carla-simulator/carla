#!/usr/bin/env python
# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import math
import argparse
import copy
from multiprocessing import Pool
from PIL import Image

import carla
import random

try:
    import pygame
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

try:
    import queue
except ImportError:
    import Queue as queue


class CarlaSyncMode(object):
    """
    Context manager to synchronize output from different sensors. Synchronous
    mode is enabled as long as we are inside this context

        with CarlaSyncMode(world, sensors) as sync_mode:
            while True:
                data = sync_mode.tick(timeout=1.0)

    """

    def __init__(self, world, *sensors, **kwargs):
        self.world = world
        self.sensors = sensors
        self.frame = None
        self.delta_seconds = 1.0 / kwargs.get('fps', 20)
        self._queues = []
        self._settings = None

    def __enter__(self):
        self._settings = self.world.get_settings()
        self.frame = self.world.apply_settings(carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=self.delta_seconds))

        def make_queue(register_event):
            q = queue.Queue()
            register_event(q.put)
            self._queues.append(q)

        make_queue(self.world.on_tick)
        for sensor in self.sensors:
            make_queue(sensor.listen)
        return self
    def tick(self, timeout):
        self.frame = self.world.tick()
        data = [self._retrieve_data(q, timeout) for q in self._queues]
        assert all(x.frame == self.frame for x in data)
        return data

    def __exit__(self, *args, **kwargs):
        self.world.apply_settings(self._settings)

    def _retrieve_data(self, sensor_queue, timeout):
        while True:
            data = sensor_queue.get(timeout=timeout)
            if data.frame == self.frame:
                return data
    # ---------------

def build_projection_matrix(w, h, fov):
    focal = w / (2.0 * np.tan(fov * np.pi / 360.0))
    K = np.identity(3)
    K[0, 0] = K[1, 1] = focal
    K[0, 2] = w / 2.0
    K[1, 2] = h / 2.0
    return K

def get_image_as_array(image):
    array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
    array = np.reshape(array, (image.height, image.width, 4))
    array = array[:, :, :3]
    array = array[:, :, ::-1]
    # make the array writeable doing a deep copy
    array2 = copy.deepcopy(array)
    return array2

def draw_image(surface, array, blend=False):
    image_surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))
    if blend:
        image_surface.set_alpha(100)
    surface.blit(image_surface, (0, 0))


def get_font():
    fonts = [x for x in pygame.font.get_fonts()]
    default_font = 'ubuntumono'
    font = default_font if default_font in fonts else fonts[0]
    font = pygame.font.match_font(font)
    return pygame.font.Font(font, 14)

def get_screen_points(camera, K, image_w, image_h, points3d):
    
    # get 4x4 matrix to transform points from world to camera coordinates
    world_2_camera = np.array(camera.get_transform().get_inverse_matrix())

    # build the points array in numpy format as (x, y, z, 1) to be operable with a 4x4 matrix
    points_temp = []
    for p in points3d:
        points_temp += [p.x, p.y, p.z, 1]
    points = np.array(points_temp).reshape(-1, 4).T
    
    # convert world points to camera space
    points_camera = np.dot(world_2_camera, points)
    
    # New we must change from UE4's coordinate system to an "standard"
    # (x, y ,z) -> (y, -z, x)
    # and we remove the fourth component also
    points = np.array([
        points_camera[1],
        points_camera[2] * -1,
        points_camera[0]])
    
    # Finally we can use our K matrix to do the actual 3D -> 2D.
    points_2d = np.dot(K, points)

    # normalize the values and transpose
    points_2d = np.array([
        points_2d[0, :] / points_2d[2, :],
        points_2d[1, :] / points_2d[2, :],
        points_2d[2, :]]).T

    return points_2d

def draw_points_on_buffer(buffer, image_w, image_h, points_2d, color, size=4):
    half = int(size / 2)
    # draw each point
    for p in points_2d:
        x = int(p[0])
        y = int(p[1])
        for j in range(y - half, y + half):
            if (j >=0 and j <image_h):
                for i in range(x - half, x + half):
                    if (i >=0 and i <image_w):
                        buffer[j][i][0] = color[0]
                        buffer[j][i][1] = color[1]
                        buffer[j][i][2] = color[2]

def draw_line_on_buffer(buffer, image_w, image_h, points_2d, color, size=4):
  x0 = int(points_2d[0][0])
  y0 = int(points_2d[0][1])
  x1 = int(points_2d[1][0])
  y1 = int(points_2d[1][1])
  dx = abs(x1 - x0)
  if x0 < x1:
    sx = 1
  else:
    sx = -1
  dy = -abs(y1 - y0)
  if y0 < y1:
    sy = 1
  else:
    sy = -1
  err = dx + dy
  while True:
    draw_points_on_buffer(buffer, image_w, image_h, ((x0,y0),), color, size)
    if (x0 == x1 and y0 == y1):
      break
    e2 = 2 * err
    if (e2 >= dy):
      err += dy
      x0 += sx
    if (e2 <= dx):
      err += dx
      y0 += sy

def draw_skeleton(buffer, image_w, image_h, boneIndex, points2d, color, size=4):
    try:
        # draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_root"]], points2d[boneIndex["crl_hips__C"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hips__C"]], points2d[boneIndex["crl_spine__C"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hips__C"]], points2d[boneIndex["crl_thigh__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hips__C"]], points2d[boneIndex["crl_thigh__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_spine__C"]], points2d[boneIndex["crl_spine01__C"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_spine01__C"]], points2d[boneIndex["crl_shoulder__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_spine01__C"]], points2d[boneIndex["crl_neck__C"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_spine01__C"]], points2d[boneIndex["crl_shoulder__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_shoulder__L"]], points2d[boneIndex["crl_arm__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_arm__L"]], points2d[boneIndex["crl_foreArm__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_foreArm__L"]], points2d[boneIndex["crl_hand__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hand__L"]], points2d[boneIndex["crl_handThumb__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hand__L"]], points2d[boneIndex["crl_handIndex__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hand__L"]], points2d[boneIndex["crl_handMiddle__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hand__L"]], points2d[boneIndex["crl_handRing__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hand__L"]], points2d[boneIndex["crl_handPinky__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handThumb__L"]], points2d[boneIndex["crl_handThumb01__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handThumb01__L"]], points2d[boneIndex["crl_handThumb02__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handThumb02__L"]], points2d[boneIndex["crl_handThumbEnd__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handIndex__L"]], points2d[boneIndex["crl_handIndex01__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handIndex01__L"]], points2d[boneIndex["crl_handIndex02__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handIndex02__L"]], points2d[boneIndex["crl_handIndexEnd__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handMiddle__L"]], points2d[boneIndex["crl_handMiddle01__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handMiddle01__L"]], points2d[boneIndex["crl_handMiddle02__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handMiddle02__L"]], points2d[boneIndex["crl_handMiddleEnd__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handRing__L"]], points2d[boneIndex["crl_handRing01__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handRing01__L"]], points2d[boneIndex["crl_handRing02__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handRing02__L"]], points2d[boneIndex["crl_handRingEnd__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handPinky__L"]], points2d[boneIndex["crl_handPinky01__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handPinky01__L"]], points2d[boneIndex["crl_handPinky02__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handPinky02__L"]], points2d[boneIndex["crl_handPinkyEnd__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_neck__C"]], points2d[boneIndex["crl_Head__C"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_Head__C"]], points2d[boneIndex["crl_eye__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_Head__C"]], points2d[boneIndex["crl_eye__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_shoulder__R"]], points2d[boneIndex["crl_arm__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_arm__R"]], points2d[boneIndex["crl_foreArm__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_foreArm__R"]], points2d[boneIndex["crl_hand__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hand__R"]], points2d[boneIndex["crl_handThumb__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hand__R"]], points2d[boneIndex["crl_handIndex__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hand__R"]], points2d[boneIndex["crl_handMiddle__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hand__R"]], points2d[boneIndex["crl_handRing__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_hand__R"]], points2d[boneIndex["crl_handPinky__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handThumb__R"]], points2d[boneIndex["crl_handThumb01__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handThumb01__R"]], points2d[boneIndex["crl_handThumb02__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handThumb02__R"]], points2d[boneIndex["crl_handThumbEnd__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handIndex__R"]], points2d[boneIndex["crl_handIndex01__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handIndex01__R"]], points2d[boneIndex["crl_handIndex02__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handIndex02__R"]], points2d[boneIndex["crl_handIndexEnd__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handMiddle__R"]], points2d[boneIndex["crl_handMiddle01__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handMiddle01__R"]], points2d[boneIndex["crl_handMiddle02__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handMiddle02__R"]], points2d[boneIndex["crl_handMiddleEnd__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handRing__R"]], points2d[boneIndex["crl_handRing01__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handRing01__R"]], points2d[boneIndex["crl_handRing02__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handRing02__R"]], points2d[boneIndex["crl_handRingEnd__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handPinky__R"]], points2d[boneIndex["crl_handPinky01__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handPinky01__R"]], points2d[boneIndex["crl_handPinky02__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_handPinky02__R"]], points2d[boneIndex["crl_handPinkyEnd__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_thigh__R"]], points2d[boneIndex["crl_leg__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_leg__R"]], points2d[boneIndex["crl_foot__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_foot__R"]], points2d[boneIndex["crl_toe__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_toe__R"]], points2d[boneIndex["crl_toeEnd__R"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_thigh__L"]], points2d[boneIndex["crl_leg__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_leg__L"]], points2d[boneIndex["crl_foot__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_foot__L"]], points2d[boneIndex["crl_toe__L"]]), color, size)
        draw_line_on_buffer(buffer, image_w, image_h, (points2d[boneIndex["crl_toe__L"]], points2d[boneIndex["crl_toeEnd__L"]]), color, size)
    except:
        pass

def should_quit():
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            return True
        elif event.type == pygame.KEYUP:
            if event.key == pygame.K_ESCAPE:
                return True
    return False

def write_image(frame, id, buffer):
    # Save the image using Pillow module.
    img = Image.fromarray(buffer)
    img.save('_out/%s_%06d.png' % (id, frame))

def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client')
    argparser.add_argument(
        '--fov',
        default=60,
        type=int,
        help='FOV for camera')
    argparser.add_argument(
      '--res',
      metavar='WIDTHxHEIGHT',
      # default='1920x1080',
      default='800x600',
      help='window resolution (default: 800x600)')
    args = argparser.parse_args()
    
    args.width, args.height = [int(x) for x in args.res.split('x')]

    actor_list = []
    pygame.init()

    display = pygame.display.set_mode(
        (args.width, args.height),
        pygame.HWSURFACE | pygame.DOUBLEBUF)
    font = get_font()
    clock = pygame.time.Clock()

    client = carla.Client('localhost', 2000)
    client.set_timeout(5.0)

    world = client.get_world()

    # spawn a camera 
    camera_bp = world.get_blueprint_library().find('sensor.camera.rgb')
    camera_bp.set_attribute("image_size_x", str(args.width))
    camera_bp.set_attribute("image_size_y", str(args.height))
    camera_bp.set_attribute("fov", str(args.fov))
    camera = world.spawn_actor(camera_bp, carla.Transform())
    
    # spawn a pedestrian
    world.set_pedestrians_seed(1235)
    ped_bp = random.choice(world.get_blueprint_library().filter("walker.pedestrian.*"))
    trans = carla.Transform()
    trans.location = world.get_random_location_from_navigation()
    ped = world.spawn_actor(ped_bp, trans)
    walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
    controller = world.spawn_actor(walker_controller_bp, carla.Transform(), ped)
    controller.start()
    controller.go_to_location(world.get_random_location_from_navigation())
    controller.set_max_speed(1.7)

    # keep tracking of actors to remove
    actor_list.append(camera)
    actor_list.append(ped)
    actor_list.append(controller)

    # get some attributes from the camera
    image_w = camera_bp.get_attribute("image_size_x").as_int()
    image_h = camera_bp.get_attribute("image_size_y").as_int()
    fov = camera_bp.get_attribute("fov").as_float()

    try:
        pool = Pool(processes=5)
        # Create a synchronous mode context.
        with CarlaSyncMode(world, camera, fps=30) as sync_mode:
            
            # set the projection matrix
            K = build_projection_matrix(image_w, image_h, fov)

            blending = 0
            turning = 0
            while True:
                if should_quit():
                    return
                clock.tick()

                # make some transition from custom pose to animation
                ped.blend_pose(math.sin(blending))

                # move the pedestrian
                blending += 0.015
                turning += 0.009

                # move camera around
                trans = ped.get_transform()
                x = math.cos(turning) * -3
                y = math.sin(turning) * 3
                trans.location.x += x
                trans.location.y += y
                trans.location.z = 2
                trans.rotation.pitch = -16
                trans.rotation.roll = 0
                trans.rotation.yaw = -360 * (turning/(math.pi*2))
                camera.set_transform(trans)

                # Advance the simulation and wait for the data.
                snapshot, image_rgb = sync_mode.tick(timeout=5.0)

                # Draw the display.
                buffer = get_image_as_array(image_rgb)

                # get the pedestrian bones
                bones = ped.get_bones()
                
                # prepare the bones (get name and world position)
                boneIndex = {}  
                points = []
                for i, bone in enumerate(bones.bone_transforms):
                    boneIndex[bone.name] = i
                    points.append(bone.world.location)
                
                # project the 3d points to 2d screen
                points2d = get_screen_points(camera, K, image_w, image_h, points)

                # draw the skeleton lines
                draw_skeleton(buffer, image_w, image_h, boneIndex, points2d, (0, 255, 0), 2)

                # draw the bone points
                draw_points_on_buffer(buffer, image_w, image_h, points2d[1:], (255, 0, 0), 4)

                draw_image(display, buffer)
                # pool.apply_async(write_image, (snapshot.frame, "ped", buffer))

                # display.blit(font.render('%d bones' % len(points), True, (255, 255, 255)), (8, 10))

                pygame.display.flip()

    finally:
        # time.sleep(5)
        print('destroying actors.')
        for actor in actor_list:
            actor.destroy()
        pygame.quit()
        pool.close()
        print('done.')


if __name__ == '__main__':

    try:

        main()

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
