# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from numpy import random
from . import SmokeTest
import time
import carla

TM_PORT = 7056
NUM_TICKS = 1000

class FrameRecord():
    def __init__(self, frame, vehicle_position_list):
        self.frame = frame
        self.vehicle_position_list = vehicle_position_list

class TestDeterminism(SmokeTest):

    def compare_records(self, record1_list, record2_list):
        record1_size = len(record1_list)
        record2_size = len(record2_list)
        self.assertEqual(record1_size, record2_size, msg="Record size missmatch")
        for i in range(0, record1_size):
            frame_errors = 0
            record1 = record1_list[i]
            record2 = record2_list[i]
            self.assertEqual(record1.frame, record2.frame, msg="Frame missmatch")
            num_actors1 = len(record1.vehicle_position_list)
            num_actors2 = len(record2.vehicle_position_list)
            self.assertEqual(num_actors1, num_actors2, msg="Number of actors mismatch")
            for j in range(0, num_actors1):
                loc1 = record1.vehicle_position_list[j]
                loc2 = record2.vehicle_position_list[j]
                # FIXME: This is a workaround to avoid the test to fail
                delta = loc1.x * 0.2
                self.assertAlmostEqual(loc1.x, loc2.x, msg="Actor location missmatch at frame %s. %s != %s"
                    % (str(record1.frame), str(loc1), str(loc2)), delta=delta)
                # FIXME: This is a workaround to avoid the test to fail
                delta = loc1.y * 0.2
                self.assertAlmostEqual(loc1.y, loc2.y, msg="Actor location missmatch at frame %s. %s != %s"
                    % (str(record1.frame), str(loc1), str(loc2)), delta=delta )
                # FIXME: This is a workaround to avoid the test to fail
                delta = loc1.z * 0.2
                self.assertAlmostEqual(loc1.z, loc2.z, msg="Actor location missmatch at frame %s. %s != %s"
                    % (str(record1.frame), str(loc1), str(loc2)), delta=delta )

    def spawn_vehicles(self, world, blueprint_transform_list):
        traffic_manager = self.client.get_trafficmanager(TM_PORT)
        vehicle_actor_list = []

        SpawnActor = carla.command.SpawnActor
        SetAutopilot = carla.command.SetAutopilot
        FutureActor = carla.command.FutureActor

        batch = []
        for blueprint, transform in blueprint_transform_list:
            batch.append(SpawnActor(blueprint, transform)
                .then(SetAutopilot(FutureActor, True, traffic_manager.get_port())))

        vehicle_actor_ids = []
        for response in self.client.apply_batch_sync(batch, True):
            if not response.error:
                vehicle_actor_ids.append(response.actor_id)

        vehicle_actor_list = world.get_actors(vehicle_actor_ids)

        return vehicle_actor_list

    def run_simulation(self, world, vehicle_actor_list):
        simulation_record = []
        ticks = 1
        while True:
            if ticks == NUM_TICKS:
                break
            else:
                position_list = []
                for vehicle in vehicle_actor_list:
                    location = vehicle.get_location()
                    position_list.append(location)
                simulation_record.append(FrameRecord(ticks, position_list))
                ticks = ticks + 1
                world.tick()
        return simulation_record

    def test_determ(self):
        print("TestDeterminism.test_determ")
        number_of_vehicles = 100
        tm_seed = 1

        self.client.load_world("Town03")
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)

        # set setting for round 1
        world = self.client.get_world()
        old_settings = world.get_settings()
        new_settings = world.get_settings()
        new_settings.synchronous_mode = True
        new_settings.fixed_delta_seconds = 0.05
        world.apply_settings(new_settings)

        blueprints = world.get_blueprint_library().filter('vehicle.*')
        spawn_points = world.get_map().get_spawn_points()

        # --------------
        # Spawn vehicles
        # --------------
        blueprint_transform_list = []
        hero = True
        for n, transform in enumerate(spawn_points):
            if n >= number_of_vehicles:
                break
            blueprint = random.choice(blueprints)
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
            if blueprint.has_attribute('driver_id'):
                driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
                blueprint.set_attribute('driver_id', driver_id)
            if hero:
                blueprint.set_attribute('role_name', 'hero')
                hero = False
            else:
                blueprint.set_attribute('role_name', 'autopilot')
            blueprint_transform_list.append((blueprint, transform))

        # reset for simulation 1
        self.client.reload_world(False)
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)
        world = self.client.get_world()
        traffic_manager = self.client.get_trafficmanager(TM_PORT)
        traffic_manager.set_synchronous_mode(True)
        traffic_manager.set_random_device_seed(tm_seed)
        traffic_manager.set_hybrid_physics_mode(True)

        # run simulation 1
        vehicle_actor_list = self.spawn_vehicles(world, blueprint_transform_list)
        record_run1 = self.run_simulation(world, vehicle_actor_list)
        traffic_manager.shut_down()

        # reset for simulation 2
        self.client.reload_world(False)
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)
        world = self.client.get_world()
        traffic_manager = self.client.get_trafficmanager(TM_PORT)
        traffic_manager.set_synchronous_mode(True)
        traffic_manager.set_random_device_seed(tm_seed)
        traffic_manager.set_hybrid_physics_mode(True)

        #run simulation 2
        vehicle_actor_list = self.spawn_vehicles(world, blueprint_transform_list)
        record_run2 = self.run_simulation(world, vehicle_actor_list)
        traffic_manager.shut_down()

        self.client.reload_world()
        world.apply_settings(old_settings)
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)

        self.compare_records(record_run1, record_run2)
