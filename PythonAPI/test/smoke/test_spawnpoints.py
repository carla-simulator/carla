# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

from . import SyncSmokeTest

TEST_TOWN = "Town10HD_Opt"
# Clearance on top of the two half-diagonals, for suspension travel and for
# the wheels a bounding box does not cover.
SPAWN_MARGIN = 0.5


class TestSpawnpoints(SyncSmokeTest):
    def test_every_spawn_point_accepts_every_vehicle(self):
        print("TestSpawnpoints.test_every_spawn_point_accepts_every_vehicle")
        # One town: the snapshot transforms are test_snapshot's job, and
        # walking every map only multiplied the map loads.
        self.world = self.load_world(TEST_TOWN)
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        self.world.apply_settings(settings)
        self.world.tick()

        blueprints = self.filter_vehicles_for_old_towns(
            self.world.get_blueprint_library().filter("vehicle.*"))
        spawn_points = self.world.get_map().get_spawn_points()

        for vehicle in blueprints:
            # Spawn points can sit closer together than a vehicle is long, so
            # occupying all of them at once is only meaningful once thinned.
            min_distance = 2.0 * self.vehicle_spawn_radius(vehicle) + SPAWN_MARGIN
            usable = self.spaced_spawn_points(spawn_points, min_distance)

            batch = [carla.command.SpawnActor(vehicle, t) for t in usable]
            response = self.client.apply_batch_sync(batch, False)
            failed = [r.error for r in response if r.error]
            self.assertFalse(
                failed,
                "%s failed at %d of %d spawn points spaced %.2f m apart: %s"
                % (vehicle.id, len(failed), len(usable), min_distance, failed[0] if failed else ""))

            self.client.apply_batch_sync(
                [carla.command.DestroyActor(r.actor_id) for r in response], True)
            self.world.tick()
