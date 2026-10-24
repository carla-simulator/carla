# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Execute the actual UE4 documentation snippet against a deterministic API fake."""
from pathlib import Path
import random
import types
import unittest


SOURCE = (Path(__file__).resolve().parents[2] / "docs" / "snipets" /
          "carla.Client.apply_batch_sync.py")


class Location:
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.x, self.y, self.z = x, y, z


class Transform:
    def __init__(self):
        self._location = Location()

    @property
    def location(self):
        return self._location

    @location.setter
    def location(self, value):
        if not isinstance(value, Location):
            raise TypeError("The native Transform.location member requires a Location")
        self._location = value


class Controller:
    def __init__(self, actor_id):
        self.id = actor_id
        self.started = False
        self.destination = None
        self.speed = None

    def start(self):
        self.started = True

    def go_to_location(self, destination):
        if not isinstance(destination, Location):
            raise TypeError("go_to_location requires a Location")
        self.destination = destination

    def set_max_speed(self, speed):
        self.speed = speed


class World:
    def __init__(self, spawn_locations=None, missing_destinations=False):
        self.locations = [Location(i, 0.0, 0.0) for i in range(50)] if spawn_locations is None else spawn_locations
        self.missing_destinations = missing_destinations
        self.navigation_calls = 0
        self.actors = {}
        self.queried_ids = None
        self.ticks_waited = 0

    def get_blueprint_library(self):
        return types.SimpleNamespace(
            filter=lambda pattern: [types.SimpleNamespace(kind="walker")],
            find=lambda name: types.SimpleNamespace(kind="controller"))

    def get_random_location_from_navigation(self):
        index = self.navigation_calls
        self.navigation_calls += 1
        if index < 50:
            return self.locations[index]
        return None if self.missing_destinations else Location(0.0, index, 0.0)

    def get_actors(self, actor_ids):
        self.queried_ids = list(actor_ids)
        return [self.actors[actor_id] for actor_id in actor_ids]

    def wait_for_tick(self):
        self.ticks_waited += 1


class Client:
    def __init__(self, world, failed_walkers=(), failed_controllers=()):
        self.world = world
        self.failed_walkers = set(failed_walkers)
        self.failed_controllers = set(failed_controllers)
        self.batches = []
        self.destroyed_ids = []
        self.successful_walkers = []
        self.successful_pairs = []

    def get_world(self):
        return self.world

    def apply_batch_sync(self, commands, tick):
        assert tick is True
        phase = len(self.batches)
        self.batches.append(list(commands))
        results = []
        for index, command in enumerate(commands):
            expected_kind = "walker" if phase == 0 else "controller"
            assert command.blueprint.kind == expected_kind
            failures = self.failed_walkers if phase == 0 else self.failed_controllers
            if index in failures:
                results.append(types.SimpleNamespace(error="spawn failed", actor_id=0))
                continue
            actor_id = (100 if phase == 0 else 200) + index
            if phase == 0:
                self.successful_walkers.append(actor_id)
                self.world.actors[actor_id] = types.SimpleNamespace(id=actor_id)
            else:
                assert command.parent in self.world.actors
                self.successful_pairs.append((actor_id, command.parent))
                self.world.actors[actor_id] = Controller(actor_id)
            results.append(types.SimpleNamespace(error="", actor_id=actor_id))
        return results

    def apply_batch(self, commands):
        for command in commands:
            self.destroyed_ids.append(command.actor_id)
            del self.world.actors[command.actor_id]


def spawn_actor(blueprint, transform, parent=None):
    return types.SimpleNamespace(blueprint=blueprint, transform=transform, parent=parent)


def destroy_actor(actor_id):
    return types.SimpleNamespace(actor_id=actor_id)


class TestApplyBatchSyncSnippet(unittest.TestCase):
    def run_snippet(self, failed_walkers=(), failed_controllers=(),
                    spawn_locations=None, missing_destinations=False):
        world = World(spawn_locations, missing_destinations)
        client = Client(world, failed_walkers, failed_controllers)
        errors = []
        namespace = {
            "client": client,
            "carla": types.SimpleNamespace(
                Transform=Transform,
                command=types.SimpleNamespace(SpawnActor=spawn_actor, DestroyActor=destroy_actor)),
            "random": random.Random(10),
            "logging": types.SimpleNamespace(error=errors.append),
        }
        exec(compile(SOURCE.read_text(encoding="utf-8"), str(SOURCE), "exec"), namespace)
        expected_ids = [actor_id for pair in client.successful_pairs for actor_id in pair]
        self.assertEqual(namespace["all_id"], expected_ids)
        self.assertEqual(world.queried_ids, expected_ids)
        self.assertEqual([actor.id for actor in namespace["all_actors"]], expected_ids)
        self.assertEqual(namespace["walkers_list"], [
            {"id": walker_id, "con": controller_id}
            for controller_id, walker_id in client.successful_pairs])
        paired_walkers = {walker_id for _, walker_id in client.successful_pairs}
        self.assertEqual(set(client.destroyed_ids), set(client.successful_walkers) - paired_walkers)
        self.assertEqual(set(world.actors), set(expected_ids))
        for controller_id, _ in client.successful_pairs:
            controller = world.actors[controller_id]
            self.assertTrue(controller.started)
            self.assertTrue(1.0 <= controller.speed <= 2.0)
            if missing_destinations:
                self.assertIsNone(controller.destination)
            else:
                self.assertIsInstance(controller.destination, Location)
        self.assertEqual(len(errors), len(failed_walkers) + len(failed_controllers))
        self.assertEqual(world.ticks_waited, 1)
        return namespace, client

    def test_successful_spawns_collect_controller_walker_pairs(self):
        namespace, _ = self.run_snippet()
        self.assertEqual(len(namespace["walkers_list"]), 50)

    def test_failed_walker_spawns_do_not_misalign_controller_results(self):
        namespace, _ = self.run_snippet(failed_walkers={1, 4, 7})
        self.assertEqual(len(namespace["walkers_list"]), 47)

    def test_failed_controller_spawns_destroy_only_the_orphan_walkers(self):
        namespace, client = self.run_snippet(failed_walkers={1, 4, 7}, failed_controllers={0, 3, 9})
        self.assertEqual(len(namespace["walkers_list"]), 44)
        self.assertEqual(len(client.destroyed_ids), 3)

    def test_all_walker_spawns_can_fail(self):
        namespace, _ = self.run_snippet(failed_walkers=set(range(50)))
        self.assertEqual(namespace["walkers_list"], [])

    def test_all_controller_spawns_can_fail(self):
        namespace, client = self.run_snippet(failed_controllers=set(range(50)))
        self.assertEqual(namespace["walkers_list"], [])
        self.assertEqual(len(client.destroyed_ids), 50)

    def test_missing_spawn_locations_are_checked_before_native_assignment(self):
        namespace, _ = self.run_snippet(spawn_locations=[Location() if i % 3 == 0 else None for i in range(50)])
        self.assertEqual(len(namespace["walkers_list"]), 17)

    def test_no_available_spawn_locations_leaves_empty_collections(self):
        namespace, _ = self.run_snippet(spawn_locations=[None] * 50)
        self.assertEqual(namespace["all_id"], [])

    def test_missing_destinations_are_not_passed_to_native_controller(self):
        self.run_snippet(missing_destinations=True)


if __name__ == "__main__":
    unittest.main()


