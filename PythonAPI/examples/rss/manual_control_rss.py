#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
# Copyright (c) 2019-2020 Intel Corporation
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
    AD           : steer
    Q            : toggle reverse
    Space        : hand-brake
    P            : toggle autopilot

    TAB          : change view
    Backspace    : change vehicle

    R            : toggle recording images to disk

    F2           : toggle RSS visualization mode
    F3           : increase log level
    F4           : decrease log level
    F5           : increase map log level
    F6           : decrease map log level
    B            : toggle RSS Road Boundaries Mode
    G            : RSS check drop current route
    T            : toggle RSS
    N            : pause simulation

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
import signal

try:
    sys.path.append(glob.glob(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/carla/dist/carla-*%d.%d-%s.egg' % (
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
import logging
import math
import random
import weakref
from rss_sensor import RssSensor # pylint: disable=relative-import
from rss_visualization import RssUnstructuredSceneVisualizer, RssBoundingBoxVisualizer, RssStateVisualizer # pylint: disable=relative-import

try:
    import pygame
    from pygame.locals import KMOD_CTRL
    from pygame.locals import KMOD_SHIFT
    from pygame.locals import K_BACKSPACE
    from pygame.locals import K_TAB
    from pygame.locals import K_DOWN
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_F1
    from pygame.locals import K_F2
    from pygame.locals import K_F3
    from pygame.locals import K_F4
    from pygame.locals import K_F5
    from pygame.locals import K_F6
    from pygame.locals import K_LEFT
    from pygame.locals import K_RIGHT
    from pygame.locals import K_SLASH
    from pygame.locals import K_SPACE
    from pygame.locals import K_UP
    from pygame.locals import K_a
    from pygame.locals import K_b
    from pygame.locals import K_d
    from pygame.locals import K_g
    from pygame.locals import K_h
    from pygame.locals import K_n
    from pygame.locals import K_p
    from pygame.locals import K_q
    from pygame.locals import K_r
    from pygame.locals import K_s
    from pygame.locals import K_w
    from pygame.locals import K_l
    from pygame.locals import K_i
    from pygame.locals import K_z
    from pygame.locals import K_x
    from pygame.locals import MOUSEBUTTONDOWN
    from pygame.locals import MOUSEBUTTONUP
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')


# ==============================================================================
# -- World ---------------------------------------------------------------------
# ==============================================================================


class World(object):

    def __init__(self, carla_world, args):
        self.world = carla_world
        self.actor_role_name = args.rolename
        self.dim = (args.width, args.height)
        try:
            self.map = self.world.get_map()
        except RuntimeError as error:
            print('RuntimeError: {}'.format(error))
            print('  The server could not send the OpenDRIVE (.xodr) file:')
            print('  Make sure it exists, has the same name of your town, and is correct.')
            sys.exit(1)
        self.external_actor = args.externalActor

        self.hud = HUD(args.width, args.height, carla_world)
        self.recording_frame_num = 0
        self.recording = False
        self.recording_dir_num = 0
        self.player = None
        self.actors = []
        self.rss_sensor = None
        self.rss_unstructured_scene_visualizer = None
        self.rss_bounding_box_visualizer = None
        self._actor_filter = args.filter
        if not self._actor_filter.startswith("vehicle."):
            print('Error: RSS only supports vehicles as ego.')
            sys.exit(1)

        self.restart()
        self.world_tick_id = self.world.on_tick(self.on_world_tick)

    def on_world_tick(self, world_snapshot):
        self.hud.on_world_tick(world_snapshot)

    def toggle_pause(self):
        settings = self.world.get_settings()
        self.pause_simulation(not settings.synchronous_mode)

    def pause_simulation(self, pause):
        settings = self.world.get_settings()
        if pause and not settings.synchronous_mode:
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            self.world.apply_settings(settings)
        elif not pause and settings.synchronous_mode:
            settings.synchronous_mode = False
            settings.fixed_delta_seconds = None
            self.world.apply_settings(settings)

    def restart(self):

        if self.external_actor:
            # Check whether there is already an actor with defined role name
            for actor in self.world.get_actors():
                if actor.attributes.get('role_name') == self.actor_role_name:
                    self.player = actor
                    break
        else:
            # Get a random blueprint.
            blueprint = random.choice(self.world.get_blueprint_library().filter(self._actor_filter))
            blueprint.set_attribute('role_name', self.actor_role_name)
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
            if blueprint.has_attribute('driver_id'):
                driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
                blueprint.set_attribute('driver_id', driver_id)
            if blueprint.has_attribute('is_invincible'):
                blueprint.set_attribute('is_invincible', 'true')
            # Spawn the player.
            if self.player is not None:
                spawn_point = self.player.get_transform()
                spawn_point.location.z += 2.0
                spawn_point.rotation.roll = 0.0
                spawn_point.rotation.pitch = 0.0
                self.destroy()
                self.player = self.world.try_spawn_actor(blueprint, spawn_point)
            while self.player is None:
                if not self.map.get_spawn_points():
                    print('There are no spawn points available in your map/town.')
                    print('Please add some Vehicle Spawn Point to your UE4 scene.')
                    sys.exit(1)
                spawn_points = self.map.get_spawn_points()
                spawn_point = random.choice(spawn_points) if spawn_points else carla.Transform()
                self.player = self.world.try_spawn_actor(blueprint, spawn_point)

        if self.external_actor:
            ego_sensors = []
            for actor in self.world.get_actors():
                if actor.parent == self.player:
                    ego_sensors.append(actor)

            for ego_sensor in ego_sensors:
                if ego_sensor is not None:
                    ego_sensor.destroy()

        # Set up the sensors.
        self.camera = Camera(self.player, self.dim)
        self.rss_unstructured_scene_visualizer = RssUnstructuredSceneVisualizer(self.player, self.world, self.dim)
        self.rss_bounding_box_visualizer = RssBoundingBoxVisualizer(self.dim, self.world, self.camera.sensor)
        self.rss_sensor = RssSensor(self.player, self.world,
                                    self.rss_unstructured_scene_visualizer, self.rss_bounding_box_visualizer, self.hud.rss_state_visualizer)

    def tick(self, clock):
        self.hud.tick(self.player, clock)

    def toggle_recording(self):
        if not self.recording:
            dir_name = "_out%04d" % self.recording_dir_num
            while os.path.exists(dir_name):
                self.recording_dir_num += 1
                dir_name = "_out%04d" % self.recording_dir_num
            self.recording_frame_num = 0
            os.mkdir(dir_name)
        else:
            self.hud.notification('Recording finished (folder: _out%04d)' % self.recording_dir_num)

        self.recording = not self.recording

    def render(self, display):
        self.camera.render(display)
        self.rss_bounding_box_visualizer.render(display, self.camera.current_frame)
        self.rss_unstructured_scene_visualizer.render(display)
        self.hud.render(display)

        if self.recording:
            pygame.image.save(display, "_out%04d/%08d.bmp" % (self.recording_dir_num, self.recording_frame_num))
            self.recording_frame_num += 1

    def destroy(self):
        # stop from ticking
        if self.world_tick_id:
            self.world.remove_on_tick(self.world_tick_id)

        if self.camera:
            self.camera.destroy()
        if self.rss_sensor:
            self.rss_sensor.destroy()
        if self.rss_unstructured_scene_visualizer:
            self.rss_unstructured_scene_visualizer.destroy()
        if self.player:
            self.player.destroy()


# ==============================================================================
# -- Camera --------------------------------------------------------------------
# ==============================================================================

class Camera(object):

    def __init__(self, parent_actor, display_dimensions):
        self.surface = None
        self._parent = parent_actor
        self.current_frame = None
        bp_library = self._parent.get_world().get_blueprint_library()
        bp = bp_library.find('sensor.camera.rgb')
        bp.set_attribute('image_size_x', str(display_dimensions[0]))
        bp.set_attribute('image_size_y', str(display_dimensions[1]))
        self.sensor = self._parent.get_world().spawn_actor(bp, carla.Transform(carla.Location(
            x=-5.5, z=2.5), carla.Rotation(pitch=8.0)), attach_to=self._parent, attachment_type=carla.AttachmentType.SpringArm)

        # We need to pass the lambda a weak reference to self to avoid
        # circular reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(lambda image: Camera._parse_image(weak_self, image))

    def destroy(self):
        self.sensor.stop()
        self.sensor.destroy()
        self.sensor = None

    def render(self, display):
        if self.surface is not None:
            display.blit(self.surface, (0, 0))

    @staticmethod
    def _parse_image(weak_self, image):
        self = weak_self()
        if not self:
            return
        self.current_frame = image.frame
        image.convert(cc.Raw)
        array = np.frombuffer(image.raw_data, dtype=np.dtype("uint8"))
        array = np.reshape(array, (image.height, image.width, 4))
        array = array[:, :, :3]
        array = array[:, :, ::-1]
        self.surface = pygame.surfarray.make_surface(array.swapaxes(0, 1))

# ==============================================================================
# -- VehicleControl -----------------------------------------------------------
# ==============================================================================


class VehicleControl(object):

    MOUSE_STEERING_RANGE = 200
    signal_received = False

    """Class that handles keyboard input."""

    def __init__(self, world, start_in_autopilot):
        self._autopilot_enabled = start_in_autopilot
        self._world = world
        self._control = carla.VehicleControl()
        self._lights = carla.VehicleLightState.NONE
        world.player.set_autopilot(self._autopilot_enabled)
        self._restrictor = carla.RssRestrictor()
        self._vehicle_physics = world.player.get_physics_control()
        world.player.set_light_state(self._lights)
        self._steer_cache = 0.0
        self._mouse_steering_center = None

        self._surface = pygame.Surface((self.MOUSE_STEERING_RANGE * 2, self.MOUSE_STEERING_RANGE * 2))
        self._surface.set_colorkey(pygame.Color('black'))
        self._surface.set_alpha(60)

        line_width = 2
        pygame.draw.polygon(self._surface,
                            (0, 0, 255),
                            [
                                (0, 0),
                                (0, self.MOUSE_STEERING_RANGE * 2 - line_width),
                                (self.MOUSE_STEERING_RANGE * 2 - line_width,
                                 self.MOUSE_STEERING_RANGE * 2 - line_width),
                                (self.MOUSE_STEERING_RANGE * 2 - line_width, 0),
                                (0, 0)
                            ], line_width)
        pygame.draw.polygon(self._surface,
                            (0, 0, 255),
                            [
                                (0, self.MOUSE_STEERING_RANGE),
                                (self.MOUSE_STEERING_RANGE * 2, self.MOUSE_STEERING_RANGE)
                            ], line_width)
        pygame.draw.polygon(self._surface,
                            (0, 0, 255),
                            [
                                (self.MOUSE_STEERING_RANGE, 0),
                                (self.MOUSE_STEERING_RANGE, self.MOUSE_STEERING_RANGE * 2)
                            ], line_width)

        world.hud.notification("Press 'H' or '?' for help.", seconds=4.0)

    def render(self, display):
        if self._mouse_steering_center:
            display.blit(
                self._surface, (self._mouse_steering_center[0] - self.MOUSE_STEERING_RANGE, self._mouse_steering_center[1] - self.MOUSE_STEERING_RANGE))

    @staticmethod
    def signal_handler(signum, _):
        print('\nReceived signal {}. Trigger stopping...'.format(signum))
        VehicleControl.signal_received = True

    def parse_events(self, world, clock):
        if VehicleControl.signal_received:
            print('\nAccepted signal. Stopping loop...')
            return True
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
                elif event.key == K_F1:
                    world.hud.toggle_info()
                elif event.key == K_h or (event.key == K_SLASH and pygame.key.get_mods() & KMOD_SHIFT):
                    world.hud.help.toggle()
                elif event.key == K_TAB:
                    world.rss_unstructured_scene_visualizer.toggle_camera()
                elif event.key == K_n:
                    world.toggle_pause()
                elif event.key == K_r:
                    world.toggle_recording()
                elif event.key == K_F2:
                    if self._world and self._world.rss_sensor:
                        self._world.rss_sensor.toggle_debug_visualization_mode()
                elif event.key == K_F3:
                    if self._world and self._world.rss_sensor:
                        self._world.rss_sensor.decrease_log_level()
                        self._restrictor.set_log_level(self._world.rss_sensor.log_level)
                elif event.key == K_F4:
                    if self._world and self._world.rss_sensor:
                        self._world.rss_sensor.increase_log_level()
                        self._restrictor.set_log_level(self._world.rss_sensor.log_level)
                elif event.key == K_F5:
                    if self._world and self._world.rss_sensor:
                        self._world.rss_sensor.decrease_map_log_level()
                elif event.key == K_F6:
                    if self._world and self._world.rss_sensor:
                        self._world.rss_sensor.increase_map_log_level()
                elif event.key == K_b:
                    if self._world and self._world.rss_sensor:
                        if self._world.rss_sensor.sensor.road_boundaries_mode == carla.RssRoadBoundariesMode.Off:
                            self._world.rss_sensor.sensor.road_boundaries_mode = carla.RssRoadBoundariesMode.On
                            print("carla.RssRoadBoundariesMode.On")
                        else:
                            self._world.rss_sensor.sensor.road_boundaries_mode = carla.RssRoadBoundariesMode.Off
                            print("carla.RssRoadBoundariesMode.Off")
                elif event.key == K_g:
                    if self._world and self._world.rss_sensor:
                        self._world.rss_sensor.drop_route()
                if isinstance(self._control, carla.VehicleControl):
                    if event.key == K_q:
                        self._control.gear = 1 if self._control.reverse else -1
                    elif event.key == K_p and not pygame.key.get_mods() & KMOD_CTRL:
                        self._autopilot_enabled = not self._autopilot_enabled
                        world.player.set_autopilot(self._autopilot_enabled)
                        world.hud.notification(
                            'Autopilot %s' % ('On' if self._autopilot_enabled else 'Off'))
                    elif event.key == K_l and pygame.key.get_mods() & KMOD_CTRL:
                        current_lights ^= carla.VehicleLightState.Special1
                    elif event.key == K_l and pygame.key.get_mods() & KMOD_SHIFT:
                        current_lights ^= carla.VehicleLightState.HighBeam
                    elif event.key == K_l:
                        # Use 'L' key to switch between lights:
                        # closed -> position -> low beam -> fog
                        if not self._lights & carla.VehicleLightState.Position:
                            world.hud.notification("Position lights")
                            current_lights |= carla.VehicleLightState.Position
                        else:
                            world.hud.notification("Low beam lights")
                            current_lights |= carla.VehicleLightState.LowBeam
                        if self._lights & carla.VehicleLightState.LowBeam:
                            world.hud.notification("Fog lights")
                            current_lights |= carla.VehicleLightState.Fog
                        if self._lights & carla.VehicleLightState.Fog:
                            world.hud.notification("Lights off")
                            current_lights ^= carla.VehicleLightState.Position
                            current_lights ^= carla.VehicleLightState.LowBeam
                            current_lights ^= carla.VehicleLightState.Fog
                    elif event.key == K_i:
                        current_lights ^= carla.VehicleLightState.Interior
                    elif event.key == K_z:
                        current_lights ^= carla.VehicleLightState.LeftBlinker
                    elif event.key == K_x:
                        current_lights ^= carla.VehicleLightState.RightBlinker
            elif event.type == MOUSEBUTTONDOWN:
                # store current mouse position for mouse-steering
                if event.button == 1:
                    self._mouse_steering_center = event.pos
            elif event.type == MOUSEBUTTONUP:
                if event.button == 1:
                    self._mouse_steering_center = None
        if not self._autopilot_enabled:
            prev_steer_cache = self._steer_cache
            self._parse_vehicle_keys(pygame.key.get_pressed(), clock.get_time())
            if pygame.mouse.get_pressed()[0]:
                self._parse_mouse(pygame.mouse.get_pos())
            self._control.reverse = self._control.gear < 0

            vehicle_control = self._control
            world.hud.original_vehicle_control = vehicle_control
            world.hud.restricted_vehicle_control = vehicle_control

            # limit speed to 30kmh
            v = self._world.player.get_velocity()
            if (3.6 * math.sqrt(v.x**2 + v.y**2 + v.z**2)) > 30.0:
                self._control.throttle = 0

            # if self._world.rss_sensor and self._world.rss_sensor.ego_dynamics_on_route and not self._world.rss_sensor.ego_dynamics_on_route.ego_center_within_route:
            #    print ("Not on route!" +  str(self._world.rss_sensor.ego_dynamics_on_route))
            if self._restrictor:
                rss_proper_response = self._world.rss_sensor.proper_response if self._world.rss_sensor and self._world.rss_sensor.response_valid else None
                if rss_proper_response:
                    if not (pygame.key.get_mods() & KMOD_CTRL):
                        vehicle_control = self._restrictor.restrict_vehicle_control(
                            vehicle_control, rss_proper_response, self._world.rss_sensor.ego_dynamics_on_route, self._vehicle_physics)
                    world.hud.restricted_vehicle_control = vehicle_control
                    world.hud.allowed_steering_ranges = self._world.rss_sensor.get_steering_ranges()
                    if world.hud.original_vehicle_control.steer != world.hud.restricted_vehicle_control.steer:
                        self._steer_cache = prev_steer_cache

            # Set automatic control-related vehicle lights
            if vehicle_control.brake:
                current_lights |= carla.VehicleLightState.Brake
            else:  # Remove the Brake flag
                current_lights &= carla.VehicleLightState.All ^ carla.VehicleLightState.Brake
            if vehicle_control.reverse:
                current_lights |= carla.VehicleLightState.Reverse
            else:  # Remove the Reverse flag
                current_lights &= carla.VehicleLightState.All ^ carla.VehicleLightState.Reverse
            if current_lights != self._lights:  # Change the light state only if necessary
                self._lights = current_lights
                world.player.set_light_state(carla.VehicleLightState(self._lights))

            world.player.apply_control(vehicle_control)

    def _parse_vehicle_keys(self, keys, milliseconds):
        if keys[K_UP] or keys[K_w]:
            self._control.throttle = min(self._control.throttle + 0.2, 1)
        else:
            self._control.throttle = max(self._control.throttle - 0.2, 0)

        if keys[K_DOWN] or keys[K_s]:
            self._control.brake = min(self._control.brake + 0.2, 1)
        else:
            self._control.brake = max(self._control.brake - 0.2, 0)

        steer_increment = 5e-4 * milliseconds
        if keys[K_LEFT] or keys[K_a]:
            if self._steer_cache > 0:
                self._steer_cache = 0
            else:
                self._steer_cache -= steer_increment
        elif keys[K_RIGHT] or keys[K_d]:
            if self._steer_cache < 0:
                self._steer_cache = 0
            else:
                self._steer_cache += steer_increment
        elif self._steer_cache > 0:
            self._steer_cache = max(self._steer_cache - steer_increment, 0.0)
        elif self._steer_cache < 0:
            self._steer_cache = min(self._steer_cache + steer_increment, 0.0)
        else:
            self._steer_cache = 0

        self._steer_cache = min(1.0, max(-1.0, self._steer_cache))
        self._control.steer = round(self._steer_cache, 1)
        self._control.hand_brake = keys[K_SPACE]

    def _parse_mouse(self, pos):
        if not self._mouse_steering_center:
            return

        lateral = float(pos[0] - self._mouse_steering_center[0])
        longitudinal = float(pos[1] - self._mouse_steering_center[1])
        max_val = self.MOUSE_STEERING_RANGE
        lateral = -max_val if lateral < -max_val else max_val if lateral > max_val else lateral
        longitudinal = -max_val if longitudinal < -max_val else max_val if longitudinal > max_val else longitudinal
        self._control.steer = lateral / max_val
        if longitudinal < 0.0:
            self._control.throttle = -longitudinal / max_val
            self._control.brake = 0.0
        elif longitudinal > 0.0:
            self._control.throttle = 0.0
            self._control.brake = longitudinal / max_val

    @staticmethod
    def _is_quit_shortcut(key):
        return (key == K_ESCAPE) or (key == K_q and pygame.key.get_mods() & KMOD_CTRL)


# ==============================================================================
# -- HUD -----------------------------------------------------------------------
# ==============================================================================


class HUD(object):

    def __init__(self, width, height, world):
        self.dim = (width, height)
        self._world = world
        self.map_name = world.get_map().name
        font = pygame.font.Font(pygame.font.get_default_font(), 20)
        font_name = 'courier' if os.name == 'nt' else 'mono'
        fonts = [x for x in pygame.font.get_fonts() if font_name in x]
        default_font = 'ubuntumono'
        mono = default_font if default_font in fonts else fonts[0]
        mono = pygame.font.match_font(mono)
        self._font_mono = pygame.font.Font(mono, 12 if os.name == 'nt' else 14)
        self._notifications = FadingText(font, (width, 40), (0, height - 40))
        self.help = HelpText(pygame.font.Font(mono, 16), width, height)
        self.server_fps = 0
        self.frame = 0
        self.simulation_time = 0
        self.original_vehicle_control = None
        self.restricted_vehicle_control = None
        self.allowed_steering_ranges = []
        self._show_info = True
        self._info_text = []
        self._server_clock = pygame.time.Clock()
        self.rss_state_visualizer = RssStateVisualizer(self.dim, self._font_mono, self._world)

    def on_world_tick(self, timestamp):
        self._server_clock.tick()
        self.server_fps = self._server_clock.get_fps()
        self.frame = timestamp.frame
        self.simulation_time = timestamp.elapsed_seconds

    def tick(self, player, clock):
        self._notifications.tick(clock)
        if not self._show_info:
            return
        t = player.get_transform()
        v = player.get_velocity()
        c = player.get_control()

        self._info_text = [
            'Server:  % 16.0f FPS' % self.server_fps,
            'Client:  % 16.0f FPS' % clock.get_fps(),
            'Map:     % 20s' % self.map_name,
            '',
            'Speed:   % 15.0f km/h' % (3.6 * math.sqrt(v.x**2 + v.y**2 + v.z**2)),
            'Location:% 20s' % ('(% 5.1f, % 5.1f)' % (t.location.x, t.location.y)),
            'Heading: % 20.2f' % math.radians(t.rotation.yaw),
            '']
        if self.original_vehicle_control:
            orig_control = self.original_vehicle_control
            restricted_control = self.restricted_vehicle_control
            allowed_steering_ranges = self.allowed_steering_ranges
            self._info_text += [
                ('Throttle:', orig_control.throttle, 0.0, 1.0, restricted_control.throttle),
                ('Steer:', orig_control.steer, -1.0, 1.0, restricted_control.steer, allowed_steering_ranges),
                ('Brake:', orig_control.brake, 0.0, 1.0, restricted_control.brake)]
        self._info_text += [
            ('Reverse:', c.reverse),
            '']

    def toggle_info(self):
        self._show_info = not self._show_info

    def notification(self, text, seconds=2.0):
        self._notifications.set_text(text, seconds=seconds)

    def error(self, text):
        self._notifications.set_text('Error: %s' % text, (255, 0, 0))

    def render(self, display):
        if self._show_info:
            info_surface = pygame.Surface((220, self.dim[1]))
            info_surface.set_alpha(100)
            display.blit(info_surface, (0, 0))
            v_offset = 4
            bar_h_offset = 100
            bar_width = 106
            for item in self._info_text:
                text_color = (255, 255, 255)
                if v_offset + 18 > self.dim[1]:
                    break
                if isinstance(item, list):
                    if len(item) > 1:
                        points = [(x + 8, v_offset + 8 + (1.0 - y) * 30) for x, y in enumerate(item)]
                        pygame.draw.lines(display, (255, 136, 0), False, points, 2)
                    item = None
                    v_offset += 18
                elif isinstance(item, tuple):
                    if isinstance(item[1], bool):
                        rect = pygame.Rect((bar_h_offset, v_offset + 2), (10, 10))
                        pygame.draw.rect(display, (255, 255, 255), rect, 0 if item[1] else 1)
                    else:
                        # draw allowed steering ranges
                        if len(item) == 6 and item[2] < 0.0:
                            for steering_range in item[5]:
                                starting_value = min(steering_range[0], steering_range[1])
                                length = (max(steering_range[0], steering_range[1]) -
                                          min(steering_range[0], steering_range[1])) / 2
                                rect = pygame.Rect(
                                    (bar_h_offset + (starting_value + 1) * (bar_width / 2), v_offset + 2), (length * bar_width, 14))
                                pygame.draw.rect(display, (0, 255, 0), rect)

                        # draw border
                        rect_border = pygame.Rect((bar_h_offset, v_offset + 2), (bar_width, 14))
                        pygame.draw.rect(display, (255, 255, 255), rect_border, 1)

                        # draw value / restricted value
                        input_value_rect_fill = 0
                        if len(item) >= 5:
                            if item[1] != item[4]:
                                input_value_rect_fill = 1
                                f = (item[4] - item[2]) / (item[3] - item[2])
                                if item[2] < 0.0:
                                    rect = pygame.Rect(
                                        (bar_h_offset + 1 + f * (bar_width - 6), v_offset + 3), (12, 12))
                                else:
                                    rect = pygame.Rect((bar_h_offset + 1, v_offset + 3), (f * bar_width, 12))
                                pygame.draw.rect(display, (255, 0, 0), rect)

                        f = (item[1] - item[2]) / (item[3] - item[2])
                        rect = None
                        if item[2] < 0.0:
                            rect = pygame.Rect((bar_h_offset + 2 + f * (bar_width - 14), v_offset + 4), (10, 10))
                        else:
                            if item[1] != 0:
                                rect = pygame.Rect((bar_h_offset + 2, v_offset + 4), (f * (bar_width - 4), 10))
                        if rect:
                            pygame.draw.rect(display, (255, 255, 255), rect, input_value_rect_fill)
                    item = item[0]
                if item:  # At this point has to be a str.
                    surface = self._font_mono.render(item, True, text_color)
                    display.blit(surface, (8, v_offset))
                v_offset += 18

            self.rss_state_visualizer.render(display, v_offset)
        self._notifications.render(display)
        self.help.render(display)


# ==============================================================================
# -- FadingText ----------------------------------------------------------------
# ==============================================================================


class FadingText(object):

    def __init__(self, font, dim, pos):
        self.font = font
        self.dim = dim
        self.pos = pos
        self.seconds_left = 0
        self.surface = pygame.Surface(self.dim)

    def set_text(self, text, color=(255, 255, 255), seconds=2.0):
        text_texture = self.font.render(text, True, color)
        self.surface = pygame.Surface(self.dim)
        self.seconds_left = seconds
        self.surface.fill((0, 0, 0, 0))
        self.surface.blit(text_texture, (10, 11))

    def tick(self, clock):
        delta_seconds = 1e-3 * clock.get_time()
        self.seconds_left = max(0.0, self.seconds_left - delta_seconds)
        self.surface.set_alpha(500.0 * self.seconds_left)

    def render(self, display):
        display.blit(self.surface, self.pos)


# ==============================================================================
# -- HelpText ------------------------------------------------------------------
# ==============================================================================


class HelpText(object):

    """Helper class to handle text output using pygame"""

    def __init__(self, font, width, height):
        lines = __doc__.split('\n')
        self.font = font
        self.line_space = 18
        self.dim = (780, len(lines) * self.line_space + 12)
        self.pos = (0.5 * width - 0.5 * self.dim[0], 0.5 * height - 0.5 * self.dim[1])
        self.seconds_left = 0
        self.surface = pygame.Surface(self.dim)
        self.surface.fill((0, 0, 0, 0))
        for n, line in enumerate(lines):
            text_texture = self.font.render(line, True, (255, 255, 255))
            self.surface.blit(text_texture, (22, n * self.line_space))
            self._render = False
        self.surface.set_alpha(220)

    def toggle(self):
        self._render = not self._render

    def render(self, display):
        if self._render:
            display.blit(self.surface, self.pos)

# ==============================================================================
# -- game_loop() ---------------------------------------------------------------
# ==============================================================================


def game_loop(args):
    pygame.init()
    pygame.font.init()
    world = None

    try:
        client = carla.Client(args.host, args.port)
        client.set_timeout(2.0)

        display = pygame.display.set_mode(
            (args.width, args.height),
            pygame.HWSURFACE | pygame.DOUBLEBUF)

        world = World(client.get_world(), args)
        controller = VehicleControl(world, args.autopilot)

        clock = pygame.time.Clock()
        while True:
            clock.tick_busy_loop(60)
            if controller.parse_events(world, clock):
                return
            world.tick(clock)
            world.render(display)
            controller.render(display)
            pygame.display.flip()

    finally:

        if world is not None:
            print('Destroying the world...')
            world.destroy()
            print('Destroyed!')

        pygame.quit()


# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================

def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client RSS')
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
        '--filter',
        metavar='PATTERN',
        default='vehicle.*',
        help='actor filter (default: "vehicle.*")')
    argparser.add_argument(
        '--rolename',
        metavar='NAME',
        default='hero',
        help='actor role name (default: "hero")')
    argparser.add_argument(
        '--externalActor',
        action='store_true',
        help='attaches to externally created actor by role name')
    args = argparser.parse_args()

    args.width, args.height = [int(x) for x in args.res.split('x')]

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

    print(__doc__)

    signal.signal(signal.SIGINT, VehicleControl.signal_handler)

    try:
        game_loop(args)

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')


if __name__ == '__main__':

    main()
