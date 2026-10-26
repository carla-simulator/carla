# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.

from contextlib import contextmanager

import carla

from . import SyncSmokeTest


class TestCommandActorConstructors(SyncSmokeTest):
    @contextmanager
    def vehicle(self):
        blueprints = self.world.get_blueprint_library().filter('vehicle.*')
        self.assertTrue(blueprints, 'A vehicle blueprint is required')
        actor = None
        for transform in self.world.get_map().get_spawn_points():
            actor = self.world.try_spawn_actor(blueprints[0], transform)
            if actor is not None:
                break
        self.assertIsNotNone(actor, 'No free vehicle spawn point')
        try:
            yield actor
        finally:
            actor.destroy()

    def test_debug_telemetry_accepts_actor_and_id(self):
        with self.vehicle() as vehicle:
            for enabled in (True, False):
                with self.subTest(enabled=enabled):
                    command = carla.command.ShowDebugTelemetry(vehicle, enabled)
                    keyword = carla.command.ShowDebugTelemetry(actor=vehicle, enabled=enabled)
                    control = carla.command.ShowDebugTelemetry(vehicle.id, enabled)
                    self.assertEqual(command.actor_id, vehicle.id)
                    self.assertEqual(command.enabled, enabled)
                    self.assertEqual(keyword.actor_id, control.actor_id)
                    self.assertEqual(keyword.enabled, control.enabled)

    def test_vehicle_lights_preserve_mask_for_actor_and_id(self):
        with self.vehicle() as vehicle:
            masks = (0, int(carla.VehicleLightState.LowBeam),
                     int(carla.VehicleLightState.Brake | carla.VehicleLightState.RightBlinker),
                     0xffffffff)
            for mask in masks:
                with self.subTest(mask=mask):
                    command = carla.command.SetVehicleLightState(vehicle, mask)
                    keyword = carla.command.SetVehicleLightState(actor=vehicle, light_state=mask)
                    control = carla.command.SetVehicleLightState(vehicle.id, mask)
                    self.assertEqual(command.actor_id, vehicle.id)
                    self.assertEqual(command.light_state, mask)
                    self.assertEqual(keyword.actor_id, control.actor_id)
                    self.assertEqual(keyword.light_state, control.light_state)
