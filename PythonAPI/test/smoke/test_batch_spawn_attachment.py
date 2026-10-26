# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

from . import SyncSmokeTest


class TestBatchSpawnAttachment(SyncSmokeTest):
    def test_then_spawn_attaches_a_sensor_to_the_new_vehicle(self):
        library = self.world.get_blueprint_library()
        vehicles = library.filter('vehicle.*')
        self.assertGreater(len(vehicles), 0)
        sensor_blueprint = library.find('sensor.other.gnss')
        relative_transform = carla.Transform(carla.Location(x=1.0, z=1.4))
        previous_sensors = {
            actor.id for actor in self.world.get_actors().filter('sensor.other.gnss')}
        vehicle_id = None

        try:
            for spawn_point in self.world.get_map().get_spawn_points():
                command = carla.command.SpawnActor(vehicles[0], spawn_point).then(
                    carla.command.SpawnActor(
                        sensor_blueprint, relative_transform, carla.command.FutureActor))
                response = self.client.apply_batch_sync([command])[0]
                if not response.has_error():
                    vehicle_id = response.actor_id
                    break
            self.assertIsNotNone(vehicle_id, 'No vehicle spawn point was available')
            self.world.tick()

            sensors = [actor for actor in self.world.get_actors().filter('sensor.other.gnss')
                       if actor.id not in previous_sensors]
            self.assertEqual(len(sensors), 1)
            sensor = sensors[0]
            self.assertIsNotNone(sensor.parent)
            self.assertEqual(sensor.parent.id, vehicle_id)
            vehicle = self.world.get_actor(vehicle_id)
            expected = vehicle.get_transform().transform(relative_transform.location)
            self.assertLess(sensor.get_location().distance(expected), 0.1)
        finally:
            ids = [actor.id for actor in self.world.get_actors().filter('sensor.other.gnss')
                   if actor.id not in previous_sensors]
            if vehicle_id is not None:
                ids.append(vehicle_id)
            if ids:
                self.client.apply_batch_sync([carla.command.DestroyActor(actor_id) for actor_id in ids])
                self.world.tick()
