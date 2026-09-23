#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Five-camera AV rig demo with optional DLSS Super Resolution.

Spawns an autopilot vehicle carrying five 1920x1080 RGB cameras
(left-front, center-front, right-front, rear-left, rear-right) and shows
them in a pygame grid together with per-camera delivered FPS. Press D to
respawn the rig with DLSS upscaling toggled (cameras render at
--dlss-percentage internally and DLSS reconstructs full resolution), so
the throughput of both modes can be compared live.

Controls:
    D            : toggle DLSS on the whole rig
    ESC / q      : quit
"""

import argparse
import time
import weakref

import carla

import numpy as np
import pygame

CAMERA_WIDTH = 1920
CAMERA_HEIGHT = 1080
TILE_WIDTH = 640
TILE_HEIGHT = 360

# name, yaw (deg), tile column/row in the 3x2 grid
RIG = [
    ('Left-Front', -55.0, 0, 0),
    ('Center-Front', 0.0, 1, 0),
    ('Right-Front', 55.0, 2, 0),
    ('Rear-Left', -140.0, 0, 1),
    ('Rear-Right', 140.0, 2, 1),
]


class RigCamera(object):
    def __init__(self, world, vehicle, name, yaw, dlss, dlss_percentage):
        self.name = name
        self.surface = None
        self.frame_times = []
        bp = world.get_blueprint_library().find('sensor.camera.rgb')
        bp.set_attribute('image_size_x', str(CAMERA_WIDTH))
        bp.set_attribute('image_size_y', str(CAMERA_HEIGHT))
        bp.set_attribute('fov', '90')
        if dlss:
            bp.set_attribute('enable_dlss', 'true')
            bp.set_attribute('dlss_screen_percentage', str(dlss_percentage))
        x = 0.8 if abs(yaw) < 90.0 else -0.8
        self.sensor = world.spawn_actor(
            bp,
            carla.Transform(carla.Location(x=x, z=1.8), carla.Rotation(yaw=yaw)),
            attach_to=vehicle)
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda image: RigCamera._on_image(weak_self, image))

    @staticmethod
    def _on_image(weak_self, image):
        self = weak_self()
        if self is None:
            return
        array = np.frombuffer(image.raw_data, dtype=np.uint8)
        array = array.reshape((image.height, image.width, 4))[:, :, 2::-1]
        self.surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))
        now = time.time()
        self.frame_times.append(now)
        self.frame_times = [t for t in self.frame_times if now - t < 2.0]

    @property
    def fps(self):
        if len(self.frame_times) < 2:
            return 0.0
        span = self.frame_times[-1] - self.frame_times[0]
        return (len(self.frame_times) - 1) / span if span > 0.0 else 0.0

    def destroy(self):
        self.sensor.stop()
        self.sensor.destroy()


def spawn_rig(world, vehicle, dlss, dlss_percentage):
    return [RigCamera(world, vehicle, name, yaw, dlss, dlss_percentage)
            for name, yaw, _, _ in RIG]


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument('--host', default='127.0.0.1')
    argparser.add_argument('-p', '--port', default=2000, type=int)
    argparser.add_argument('--tm-port', default=8000, type=int)
    argparser.add_argument('--dlss', action='store_true',
                           help='start with DLSS upscaling enabled')
    argparser.add_argument('--dlss-percentage', default=50.0, type=float,
                           help='internal render resolution percentage when DLSS is on')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(120.0)
    world = client.get_world()

    pygame.init()
    display = pygame.display.set_mode(
        (3 * TILE_WIDTH, 2 * TILE_HEIGHT), pygame.HWSURFACE | pygame.DOUBLEBUF)
    pygame.display.set_caption('CARLA 5-camera rig')
    font = pygame.font.Font(pygame.font.get_default_font(), 16)
    big_font = pygame.font.Font(pygame.font.get_default_font(), 22)
    clock = pygame.time.Clock()

    vehicle = None
    cameras = []
    dlss = args.dlss
    try:
        bp = world.get_blueprint_library().filter('vehicle.lincoln.mkz*')[0]
        for spawn_point in world.get_map().get_spawn_points():
            vehicle = world.try_spawn_actor(bp, spawn_point)
            if vehicle is not None:
                break
        if vehicle is None:
            raise RuntimeError('no free spawn point for the vehicle')
        vehicle.set_autopilot(True, client.get_trafficmanager(args.tm_port).get_port())
        cameras = spawn_rig(world, vehicle, dlss, args.dlss_percentage)

        running = True
        while running:
            clock.tick(60)
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                elif event.type == pygame.KEYUP:
                    if event.key in (pygame.K_ESCAPE, pygame.K_q):
                        running = False
                    elif event.key == pygame.K_d:
                        dlss = not dlss
                        for camera in cameras:
                            camera.destroy()
                        cameras = spawn_rig(world, vehicle, dlss, args.dlss_percentage)

            display.fill((15, 15, 15))
            for camera, (name, _, col, row) in zip(cameras, RIG):
                rect = (col * TILE_WIDTH, row * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT)
                if camera.surface is not None:
                    display.blit(
                        pygame.transform.scale(camera.surface, (TILE_WIDTH, TILE_HEIGHT)),
                        rect[:2])
                label = font.render(
                    '%s  %.1f fps' % (name, camera.fps), True, (255, 255, 255))
                display.blit(label, (rect[0] + 8, rect[1] + 8))

            # Center-bottom tile: rig summary.
            aggregate = sum(camera.fps for camera in cameras)
            mode = ('DLSS %d%% -> %dx%d' % (args.dlss_percentage, CAMERA_WIDTH, CAMERA_HEIGHT)
                    if dlss else 'native %dx%d' % (CAMERA_WIDTH, CAMERA_HEIGHT))
            lines = [
                '5 x sensor.camera.rgb',
                mode,
                'per camera: %.1f fps' % (aggregate / len(cameras) if cameras else 0.0),
                'aggregate:  %.1f frames/s' % aggregate,
                'client UI:  %.0f fps' % clock.get_fps(),
                '',
                'D: toggle DLSS   ESC: quit',
            ]
            for i, line in enumerate(lines):
                text = big_font.render(line, True, (200, 220, 255))
                display.blit(text, (TILE_WIDTH + 24, TILE_HEIGHT + 24 + 30 * i))

            pygame.display.flip()
    finally:
        for camera in cameras:
            camera.destroy()
        if vehicle is not None:
            vehicle.set_autopilot(False)
            vehicle.destroy()
        pygame.quit()


if __name__ == '__main__':
    main()
