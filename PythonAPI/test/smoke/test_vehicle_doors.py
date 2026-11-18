# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke tests for `Vehicle.is_door_open()` (#7701)."""

import carla

from . import SyncSmokeTest

DOORS = (carla.VehicleDoor.FL, carla.VehicleDoor.FR,
         carla.VehicleDoor.RL, carla.VehicleDoor.RR)

# open_door and close_door do not wait for the server: give the call a few
# ticks to land before the snapshot is read.
SETTLE_TICKS = 3


class TestVehicleDoors(SyncSmokeTest):
    """Opens and closes doors and checks what `is_door_open()` reports."""

    def setUp(self):
        super().setUp()
        self.actors = []

    def tearDown(self):
        for actor in self.actors:
            actor.destroy()
        self.actors = []
        super().tearDown()

    def _spawn(self, has_doors):
        blueprints = [
            bp for bp in self.filter_vehicles_for_old_towns(
                self.world.get_blueprint_library().filter('vehicle.*'))
            if int(bp.get_attribute('number_of_wheels')) == 4
            and bp.get_attribute('has_dynamic_doors').as_bool() == has_doors]
        self.assertGreater(len(blueprints), 0)
        for spawn_point in self.world.get_map().get_spawn_points():
            vehicle = self.world.try_spawn_actor(blueprints[0], spawn_point)
            if vehicle is not None:
                self.actors.append(vehicle)
                self._tick(10)
                return vehicle
        self.fail('no free spawn point')

    def _tick(self, count=SETTLE_TICKS):
        for _ in range(count):
            self.world.tick()

    def _open(self, vehicle):
        return [door for door in DOORS if vehicle.is_door_open(door)]

    def test_door_state_follows_open_and_close(self):
        """The getter follows open_door/close_door, and rebuilding the door
        constraints keeps open doors open."""
        print('TestVehicleDoors.test_door_state_follows_open_and_close')
        vehicle = self._spawn(has_doors=True)
        self.assertEqual(self._open(vehicle), [], 'a vehicle spawns with its doors closed')
        self.assertFalse(vehicle.is_door_open(carla.VehicleDoor.All))

        vehicle.open_door(carla.VehicleDoor.FL)
        self._tick()
        self.assertEqual(self._open(vehicle), [carla.VehicleDoor.FL])
        self.assertFalse(vehicle.is_door_open(carla.VehicleDoor.All))

        vehicle.open_door(carla.VehicleDoor.All)
        self._tick()
        self.assertEqual(self._open(vehicle), list(DOORS))
        self.assertTrue(vehicle.is_door_open(carla.VehicleDoor.All))

        # Both rebuild the door constraints, which used to close every door.
        vehicle.apply_physics_control(vehicle.get_physics_control())
        self._tick()
        self.assertEqual(self._open(vehicle), list(DOORS),
                         'apply_physics_control closed the open doors')
        vehicle.set_simulate_physics(False)
        self._tick()
        vehicle.set_simulate_physics(True)
        self._tick()
        self.assertEqual(self._open(vehicle), list(DOORS),
                         'set_simulate_physics closed the open doors')

        vehicle.close_door(carla.VehicleDoor.FL)
        self._tick()
        self.assertEqual(self._open(vehicle), list(DOORS[1:]))
        self.assertFalse(vehicle.is_door_open(carla.VehicleDoor.All))

        vehicle.close_door(carla.VehicleDoor.All)
        self._tick()
        self.assertEqual(self._open(vehicle), [])

    def test_vehicle_without_doors_never_has_one_open(self):
        """A vehicle without dynamic doors reports every door closed."""
        print('TestVehicleDoors.test_vehicle_without_doors_never_has_one_open')
        vehicle = self._spawn(has_doors=False)
        vehicle.open_door(carla.VehicleDoor.All)
        self._tick()
        self.assertEqual(self._open(vehicle), [])
        self.assertFalse(vehicle.is_door_open(carla.VehicleDoor.All))
        self.assertFalse(vehicle.is_door_open(carla.VehicleDoor.Trunk))
