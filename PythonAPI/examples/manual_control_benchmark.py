#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Allows controlling a vehicle with a keyboard. For a simpler and more
# documented example, please take a look at tutorial.py.

"""
Welcome to CARLA manual control.

Use ARROWS or WASD keys for control.

    W            : throttle
    S            : brake
    A/D          : steer left/right
    Q            : toggle reverse
    Space        : hand-brake
    P            : toggle autopilot
    M            : toggle manual transmission
    ,/.          : gear up/down

    L            : toggle next light type
    SHIFT + L    : toggle high beam
    Z/X          : toggle right/left blinker
    I            : toggle interior light

    TAB          : change sensor position
    ` or N       : next sensor
    [1-9]        : change to sensor [1-9]
    G            : toggle radar visualization
    C            : change weather (Shift+C reverse)
    Backspace    : change vehicle

    R            : toggle recording images to disk

    CTRL + R     : toggle recording of simulation (replacing any previous)
    CTRL + P     : start replaying last recorded simulation
    CTRL + +     : increments the start time of the replay by 1 second (+SHIFT = 10 seconds)
    CTRL + -     : decrements the start time of the replay by 1 second (+SHIFT = 10 seconds)

    F1           : toggle HUD
    H/?          : toggle help
    ESC          : quit
"""

from __future__ import print_function


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
    pass


# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================


import carla

from carla import ColorConverter as cc

import argparse
import collections
import datetime
import logging
import math
import random
import re
import weakref
import json

try:
    import pygame
    from pygame.locals import KMOD_CTRL
    from pygame.locals import KMOD_SHIFT
    from pygame.locals import K_0
    from pygame.locals import K_9
    from pygame.locals import K_BACKQUOTE
    from pygame.locals import K_BACKSPACE
    from pygame.locals import K_COMMA
    from pygame.locals import K_DOWN
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_F1
    from pygame.locals import K_F2
    from pygame.locals import K_F3
    from pygame.locals import K_F4
    from pygame.locals import K_F5
    from pygame.locals import K_F12
    from pygame.locals import K_LEFT
    from pygame.locals import K_PERIOD
    from pygame.locals import K_RIGHT
    from pygame.locals import K_SLASH
    from pygame.locals import K_SPACE
    from pygame.locals import K_TAB
    from pygame.locals import K_UP
    from pygame.locals import K_a
    from pygame.locals import K_c
    from pygame.locals import K_g
    from pygame.locals import K_d
    from pygame.locals import K_h
    from pygame.locals import K_m
    from pygame.locals import K_n
    from pygame.locals import K_p
    from pygame.locals import K_q
    from pygame.locals import K_r
    from pygame.locals import K_s
    from pygame.locals import K_t
    from pygame.locals import K_w
    from pygame.locals import K_l
    from pygame.locals import K_i
    from pygame.locals import K_z
    from pygame.locals import K_x
    from pygame.locals import K_MINUS
    from pygame.locals import K_EQUALS
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')


# ==============================================================================
# -- Global functions ----------------------------------------------------------
# ==============================================================================


def find_weather_presets():
    rgx = re.compile('.+?(?:(?<=[a-z])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])|$)')
    name = lambda x: ' '.join(m.group(0) for m in rgx.finditer(x))
    presets = [x for x in dir(carla.WeatherParameters) if re.match('[A-Z].+', x)]
    return [(getattr(carla.WeatherParameters, x), name(x)) for x in presets]


def get_actor_display_name(actor, truncate=250):
    name = ' '.join(actor.type_id.replace('_', '.').title().split('.')[1:])
    return (name[:truncate - 1] + u'\u2026') if len(name) > truncate else name


# ==============================================================================
# -- World ---------------------------------------------------------------------
# ==============================================================================


class World(object):
    def __init__(self, carla_world, hud, args):
        self.world = carla_world
        try:
            self.map = self.world.get_map()
        except RuntimeError as error:
            print('RuntimeError: {}'.format(error))
            print('  The server could not send the OpenDRIVE (.xodr) file:')
            print('  Make sure it exists, has the same name of your town, and is correct.')
            sys.exit(1)

        self.sync = args.sync
        # self.sync = False
        self.hud = hud
        self.player = None
        self.benchmark_sensor = None
        self.camera_manager = None
        self.recording_enabled = False
        self.recording_start = 0
        self.current_spawn_point = 8

        self.restart()
        self.world.on_tick(hud.on_world_tick)

    def restart(self):
        if self.sync:
            settings = carla.WorldSettings(
                no_rendering_mode=False,
                synchronous_mode=True,
                fixed_delta_seconds=1.0/20.0)
            self.world.apply_settings(settings)

        self.player_max_speed = 1.589
        self.player_max_speed_fast = 3.713
        # Get car blueprint
        blueprint = self.world.get_blueprint_library().find('vehicle.lincoln.mkz2017')
        blueprint.set_attribute('role_name', 'hero')
        blueprint.set_attribute('color', '0,0,0')
        # Select spawn point
        spawn_point = self.map.get_spawn_points()[self.current_spawn_point]
        spawn_point.location.z += 1.0

        while self.player is None:
            self.player = self.world.try_spawn_actor(blueprint, spawn_point)

        # Set up the sensors.
        self.camera_manager = CameraManager(self.player, self.hud)
        self.benchmark_sensor = BenchmarkSensor(self.player)

    def tick(self, clock):
        if self.sync:
            if not self.camera_manager.ready_for_tick():
                return
            self.world.tick()
            self.camera_manager.tick()
        else:
            self.world.wait_for_tick()
        self.hud.tick(self, clock)

    def render(self, display):
        self.camera_manager.render(display)
        if self.hud.enabled:
            self.hud.render(display)

    def destroy_sensors(self):
        actors = [
            self.camera_manager,
            self.benchmark_sensor,
            ]
        for actor in actors:
            if actor is not None:
                actor.destroy()

    def destroy(self):
        if self.sync:
            settings = carla.WorldSettings(
                no_rendering_mode=False,
                synchronous_mode=False,
                fixed_delta_seconds=0.0)
            self.world.apply_settings(settings)

        actors = [
            self.camera_manager,
            self.benchmark_sensor,
            self.player]
        for actor in actors:
            if actor is not None:
                actor.destroy()

    def next_position(self, reverse=False):
        increment = 1 if not reverse else -1
        spawn_points = self.map.get_spawn_points()
        self.current_spawn_point += increment
        if self.current_spawn_point > len(spawn_points):
            self.current_spawn_point = 0
        spawn_point = spawn_points[self.current_spawn_point]
        spawn_point.location.z += 1.0
        self.player.set_transform(carla.Transform(spawn_point.location, spawn_point.rotation))

    def toggle_hud(self):
        self.hud.enabled = not self.hud.enabled

    def toggle_image_diff(self):
        for camera in self.camera_manager.sensor:
            camera.show_image_diff = not camera.show_image_diff

    def freeze_render(self):
        for camera in self.camera_manager.sensor:
            camera.freeze_render = not camera.freeze_render

    def freeze_image_diff(self):
        for camera in self.camera_manager.sensor:
            camera.freeze_image_diff = not camera.freeze_image_diff

    def profile(self):
        self.benchmark_sensor.toggle_capture()

# ==============================================================================
# -- KeyboardControl -----------------------------------------------------------
# ==============================================================================


class KeyboardControl(object):
    """Class that handles keyboard input."""
    def __init__(self, world, start_in_autopilot):
        self._autopilot_enabled = start_in_autopilot
        if isinstance(world.player, carla.Vehicle):
            self._control = carla.VehicleControl()
            self._lights = carla.VehicleLightState.NONE
            world.player.set_autopilot(self._autopilot_enabled)
            world.player.set_light_state(self._lights)
        elif isinstance(world.player, carla.Walker):
            self._control = carla.WalkerControl()
            self._autopilot_enabled = False
            self._rotation = world.player.get_transform().rotation
        else:
            raise NotImplementedError("Actor type not supported")
        self._steer_cache = 0.0

    def parse_events(self, client, world, clock):
        if isinstance(self._control, carla.VehicleControl):
            current_lights = self._lights
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return True
            elif event.type == pygame.KEYUP:
                if self._is_quit_shortcut(event.key):
                    return True
                elif event.key == K_BACKSPACE:
                    if self._autopilot_enabled:
                        world.player.set_autopilot(False)
                        world.restart()
                        world.player.set_autopilot(True)
                    else:
                        world.restart()
                elif event.key == K_LEFT:
                    world.next_position(True)
                elif event.key == K_RIGHT:
                    world.next_position()
                elif event.key == K_F1:
                    world.toggle_hud()
                elif event.key == K_F2:
                    world.toggle_image_diff()
                elif event.key == K_F3:
                    world.freeze_image_diff()
                elif event.key == K_F4:
                    world.freeze_render()
                elif event.key == K_F5:
                    world.profile()
                elif event.key == K_F12:
                    world.camera_manager.screenshot(0)
                elif event.key == K_c and pygame.key.get_mods() & KMOD_SHIFT:
                    world.next_weather(reverse=True)
                elif event.key == K_c:
                    world.next_weather()
                elif event.key == K_p and (pygame.key.get_mods() & KMOD_CTRL):
                    # stop recorder
                    client.stop_recorder()
                    world.recording_enabled = False
                    # work around to fix camera at start of replaying
                    current_index = world.camera_manager.index
                    world.destroy_sensors()
                    # disable autopilot
                    self._autopilot_enabled = False
                    world.player.set_autopilot(self._autopilot_enabled)
                    # replayer
                    client.replay_file("manual_recording.rec", world.recording_start, 0, 0)
                    world.camera_manager.set_sensor(current_index)
                if isinstance(self._control, carla.VehicleControl):
                    if event.key == K_q:
                        self._control.gear = 1 if self._control.reverse else -1
                    elif event.key == K_p and not pygame.key.get_mods() & KMOD_CTRL:
                        self._autopilot_enabled = not self._autopilot_enabled
                        world.player.set_autopilot(self._autopilot_enabled)


        if not self._autopilot_enabled:
            if isinstance(self._control, carla.VehicleControl):
                self._parse_vehicle_keys(pygame.key.get_pressed(), clock.get_time())
                self._control.reverse = self._control.gear < 0
                # Set automatic control-related vehicle lights
                if self._control.brake:
                    current_lights |= carla.VehicleLightState.Brake
                else: # Remove the Brake flag
                    current_lights &= carla.VehicleLightState.All ^ carla.VehicleLightState.Brake
                if self._control.reverse:
                    current_lights |= carla.VehicleLightState.Reverse
                else: # Remove the Reverse flag
                    current_lights &= carla.VehicleLightState.All ^ carla.VehicleLightState.Reverse
                if current_lights != self._lights: # Change the light state only if necessary
                    self._lights = current_lights
                    world.player.set_light_state(carla.VehicleLightState(self._lights))

            world.player.apply_control(self._control)

    def _parse_vehicle_keys(self, keys, milliseconds):
        if keys[K_w]:
            self._control.throttle = min(self._control.throttle + 0.01, 1)
        else:
            self._control.throttle = 0.0

        if keys[K_s]:
            self._control.brake = min(self._control.brake + 0.2, 1)
        else:
            self._control.brake = 0

        steer_increment = 5e-4 * milliseconds
        if keys[K_a]:
            if self._steer_cache > 0:
                self._steer_cache = 0
            else:
                self._steer_cache -= steer_increment
        elif keys[K_d]:
            if self._steer_cache < 0:
                self._steer_cache = 0
            else:
                self._steer_cache += steer_increment
        else:
            self._steer_cache = 0.0
        self._steer_cache = min(0.7, max(-0.7, self._steer_cache))
        self._control.steer = round(self._steer_cache, 1)
        self._control.hand_brake = keys[K_SPACE]

    @staticmethod
    def _is_quit_shortcut(key):
        return (key == K_ESCAPE) or (key == K_q and pygame.key.get_mods() & KMOD_CTRL)


# ==============================================================================
# -- HUD -----------------------------------------------------------------------
# ==============================================================================


class HUD(object):
    def __init__(self, width, height):
        self.dim = (width, height)
        font = pygame.font.Font(pygame.font.get_default_font(), 20)
        font_name = 'courier' if os.name == 'nt' else 'mono'
        fonts = [x for x in pygame.font.get_fonts() if font_name in x]
        default_font = 'ubuntumono'
        mono = default_font if default_font in fonts else fonts[0]
        mono = pygame.font.match_font(mono)
        self._font_mono = pygame.font.Font(mono, 12 if os.name == 'nt' else 14)
        self.server_fps = 0
        self.frame = 0
        self.simulation_time = 0
        self._show_info = True
        self._info_text = []
        self._server_clock = pygame.time.Clock()
        self.enabled = True

    def on_world_tick(self, timestamp):
        self._server_clock.tick()
        self.server_fps = self._server_clock.get_fps()
        self.frame = timestamp.frame
        self.simulation_time = timestamp.elapsed_seconds

    def tick(self, world, clock):
        player_loc = world.player.get_transform().location
        player_rot = world.player.get_transform().rotation
        self._info_text = [
            'Map:  % 20s' % world.map.name,
            ' ',
            'Frame:   %d'          % self.frame,
            'Server:  % 16.0f FPS' % self.server_fps,
            'Client:  % 16.0f FPS' % (clock.get_fps()),
            ' ',
            'Spawn point:  %d' % world.current_spawn_point,
            'Car transform',
            '  Loc :% 20s' % ('[% 5.1f, % 5.1f, % 5.1f]' % (player_loc.x, player_loc.y, player_loc.z)),
            '  Rot :% 20s' % ('[% 5.1f, % 5.1f, % 5.1f]' % (player_rot.pitch, player_rot.yaw, player_rot.roll)),
            ' ',
        ]

        for i in range(len(world.camera_manager.sensor)):
            camera = world.camera_manager.sensor[i]
            loc = camera.sensor.get_transform().location
            rot = camera.sensor.get_transform().rotation
            self._info_text.append('Camera %d transform' % i)
            self._info_text.append('  Loc :% 20s' % ('[% 5.1f, % 5.1f, % 5.1f]' % (loc.x, loc.y, loc.z)))
            self._info_text.append('  Rot :% 20s' % ('[% 5.1f, % 5.1f, % 5.1f]' % (rot.pitch, rot.yaw, rot.roll)))
            self._info_text.append(' ')


    def render(self, display):
        info_surface = pygame.Surface((220, self.dim[1]))
        info_surface.set_alpha(100)
        display.blit(info_surface, (0, 0))
        v_offset = 4
        bar_h_offset = 100
        bar_width = 106
        for item in self._info_text:
            surface = self._font_mono.render(item, True, (255, 255, 255))
            display.blit(surface, (8, v_offset))
            v_offset += 18


# ==============================================================================
# -- CameraManager -------------------------------------------------------------
# ==============================================================================

class CameraManager(object):
    def __init__(self, parent_actor, hud):
        self.sensor = []
        self._parent = parent_actor
        self.hud = hud
        self.recording = False
        self.num_sensors = 4

        bound_y = 0.5 + self._parent.bounding_box.extent.y
        Attachment = carla.AttachmentType
        self.transforms = [
            carla.Transform(carla.Location(x= 1.75, y= 0.0 , z=1.5 ), carla.Rotation(pitch= 0.0 , yaw=   0.0 , roll= 0.0 )),
            carla.Transform(carla.Location(x=-2.5 , y= 0.0 , z=1.0 ), carla.Rotation(pitch= 0.0 , yaw= 180.0 , roll= 0.0 )),
            carla.Transform(carla.Location(x= 0.6 , y= 1.0 , z=1.25), carla.Rotation(pitch= 0.0 , yaw=  90.0 , roll= 0.0 )),
            carla.Transform(carla.Location(x= 0.6 , y=-1.0 , z=1.25), carla.Rotation(pitch= 0.0 , yaw=- 90.0 , roll= 0.0 )),
        ]
        world = self._parent.get_world()
        bp = world.get_blueprint_library().find('sensor.camera.rgb')
        bp.set_attribute('image_size_x', str(hud.dim[0]/2)) #str(1920))  # str(hud.dim[0])) # str(1))
        bp.set_attribute('image_size_y', str(hud.dim[1]/2)) #str(1080))  # str(hud.dim[1])) # str(1))

        x = 0
        y = 0
        width_offset = int(self.hud.dim[0]/2)
        height_offset = int(self.hud.dim[1]/2)

        # We need to pass the lambda a weak reference to self to avoid circular reference.
        weak_self = weakref.ref(self)
        for i in range(self.num_sensors):
            new_sensor = world.spawn_actor(bp, self.transforms[i], attach_to=self._parent)
            camera = Camera(new_sensor, [width_offset, height_offset], (width_offset*x, height_offset*y))
            self.sensor.append(camera)
            x += 1
            if(x == 2):
                x = 0
                y += 1

    def destroy(self):
        for i in range(self.num_sensors):
            self.sensor[i].destroy()

    def toggle_recording(self):
        for sensor in self.sensor:
            sensor.recording = not sensor.recording
        self.hud.notification('Recording %s' % ('On' if self.recording else 'Off'))

    def render(self, display):
        # display.fill((40,40,40))
        for sensor in self.sensor:
            sensor.render(display)

    def ready_for_tick(self):
        for sensor in self.sensor:
            if not sensor.image_ready:
                return False
        return True

    def tick (self):
        for sensor in self.sensor:
            sensor.image_ready = False

    def screenshot(self, index):
        image = self.sensor[index].image
        image.save_to_disk('_out/camera%02d_%08d' % (index, image.frame))

class Camera(object,):
    def __init__(self, sensor, size, rect):
        self.sensor = sensor
        self.surface = None
        self.prev_surface = None
        self.recording = False
        self.size = size
        self.rect = rect
        self.diff_surface = None
        self.show_image_diff = False
        self.freeze_render = False
        self.freeze_image_diff = False
        self.image_ready = True

        self.sensor.listen(lambda image: self.parse_image(image) )

    def render(self, display):
        if self.freeze_render:
            return
        if self.surface is not None:
            display.blit(self.surface, self.rect)

    def parse_image(self, image):
        image.convert(cc.Raw)
        array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        array = np.reshape(array, (int(image.height), int(image.width), 4))
        array = array[:, :, :3]
        array = array[:, :, ::-1]
        array = array.swapaxes(0, 1)

        surface = pygame.surfarray.make_surface(array)

        if self.show_image_diff:
            self.surface = self.image_diff(surface)
        else:
            self.surface = surface

        if self.recording:
            image.save_to_disk('_out/%08d' % image.frame)

        self.image_ready = True

    def destroy(self):
        self.sensor.destroy()

    def image_diff(self, surface):
        if self.prev_surface is None:
            self.prev_surface = surface
            return

        img1_arr = pygame.PixelArray(self.prev_surface)
        img2_arr = pygame.PixelArray(surface)
        diff_arr = img2_arr.compare(img1_arr, distance=0.075)
        if not self.freeze_image_diff:
            self.prev_surface = surface

        return diff_arr.surface.copy()



# ==============================================================================
# -- BenchmarkSensor ---------------------------------------------------------------
# ==============================================================================


class BenchmarkSensor(object):
    def __init__(self, parent_actor):
        self.sensor = None
        self._parent = parent_actor
        self.world = self._parent.get_world()
        self.capture = False

        self.num_samples = 1000
        self.current_samples = 0

        self.result_list = []
        self.total = {}

        # self.start()

    def start(self):
        # Reset previous stats
        self.world.send_console_command("stat none")
        self.world.send_console_command("stat fps")

        # Disable spectator
        spectator = self.world.get_spectator()
        spectator.set_transform(carla.Transform(carla.Location(z=500), carla.Rotation(pitch=90)))
        self.world.send_console_command("r.screenpercentage 1")

        bp = self.world.get_blueprint_library().find('sensor.other.benchmark')

        with open('performance_queries.json') as f:
            queries = f.read()
        bp.set_attribute('queries', str(queries))
        self.sensor = self.world.spawn_actor(bp, carla.Transform(), attach_to=self._parent)
        self.sensor.listen( lambda benchmark_data: self.benchmark_callback(benchmark_data))


    def destroy(self):
        self.world.send_console_command("r.screenpercentage 100")
        self.world.send_console_command("stat none")

        if self.sensor is not None:
            self.sensor.destroy()
        return

    def toggle_capture(self, samples = 2):
        self.capture = not self.capture
        self.num_samples = samples
        if self.capture:
            self.result_list = []
            self.total = {}

    def benchmark_callback(self, benchmark_data):
        if not self or not self.capture:
            return
        data = json.loads(benchmark_data.result)

        self.result_list.append(data)

        self.current_samples += 1

        if self.current_samples >= self.num_samples:
            self.capture = False
            self.current_samples = 0
            self.calculate_results()

    def calculate_results(self):
        for result in self.result_list:
            for key in result:
                print(key)
                if "STATGROUP_" in key:
                    for stat in result[key]:
                        for stat_key in stat:
                            value_dict = self.total.get(key, {})
                            value_list = value_dict.get(stat_key, [])
                            value_list.append(float(stat[stat_key]))
                            value_dict[stat_key] = value_list
                            self.total[key] = value_dict
                else:
                    value_list = self.total.get(key, [])
                    value_list.append(float(result[key]))
                    self.total[key] = value_list

        print("Total: ")
        for key in self.total:
            if "STATGROUP_" in key:
                for stat_key in self.total[key]:
                    print(stat_key)
                    mean, std, amin, amax = self.calculate_stadistics(self.total[key][stat_key])
                    self.total[key][stat_key] = { "mean":mean, "std":std, "min":amin, "max":amax }
                    print(stat_key, ":", self.total[key][stat_key])
            else:
                mean, std, amin, amax = self.calculate_stadistics(self.total[key])
                self.total[key] = { "mean":mean, "std":std, "min":amin, "max":amax }
                print(key,":", self.total[key])

    def calculate_stadistics(self, values):
        mean = np.mean(values)
        std = np.std(values)
        amin = np.amin(values)
        amax = np.amax(values)
        return mean, std, amin, amax


# ==============================================================================
# -- game_loop() ---------------------------------------------------------------
# ==============================================================================


def game_loop(args):
    world = None
    run_sync = False

    try:
        client = carla.Client(args.host, args.port, 0)
        client.set_timeout(20.0)

        pygame.init()
        pygame.font.init()
        display = pygame.display.set_mode(
            (args.width, args.height),
            pygame.HWSURFACE | pygame.DOUBLEBUF)

        hud = HUD(args.width, args.height)
        world = World(client.get_world(), hud, args)
        controller = KeyboardControl(world, args.autopilot)

        clock = pygame.time.Clock()
        stop_trace = False
        while True:
            clock.tick_busy_loop(60)
            if controller.parse_events(client, world, clock):
                return
            world.tick(clock)
            world.render(display)
            pygame.display.flip()

    finally:

        print("Finished")

        if (world and world.recording_enabled):
            client.stop_recorder()

        if world is not None:
            world.destroy()

        pygame.quit()


# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================


def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client')
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-a', '--autopilot',
        action='store_true',
        help='enable autopilot')
    argparser.add_argument(
        '--res',
        metavar='WIDTHxHEIGHT',
        default='1280x720',
        help='window resolution (default: 1280x720)')
    argparser.add_argument(
        '--sync',
        action='store_true',
        help='Synchronous mode execution')
    args = argparser.parse_args()

    args.width, args.height = [int(x) for x in args.res.split('x')]

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

    print(__doc__)

    try:

        game_loop(args)

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')


if __name__ == '__main__':
    main()
