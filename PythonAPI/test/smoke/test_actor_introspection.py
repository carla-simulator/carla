# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import math
import random
import time

import carla

from . import SmokeTest

# The packaged build only ships Town10HD_Opt and Mine_01; the default
# SmokeTest.tearDown loads Town03, which does not exist on the package.
TEST_TOWN = "Town10HD_Opt"

# Bones in each pedestrian skeleton. Skel_Pedestrian_G2 has 66; the GEN3
# children (walker.pedestrian.0051/0052) use Skel_Pedestrian_G3, which has
# 26 because it has no finger bones. A walker of any other generation must
# report at least one bone.
WALKER_MIN_BONES_BY_GENERATION = {2: 66, 3: 26}


class TestActorIntrospection(SmokeTest):

    def setUp(self):
        super().setUp()
        self.load_world(TEST_TOWN)
        self.world = self.client.get_world()
        self.spawned = []

    def tearDown(self):
        for actor in self.spawned:
            try:
                actor.destroy()
            except RuntimeError:
                pass
        self.spawned = []
        self.load_world(TEST_TOWN)
        self.world = None
        self.client = None

    def _spawn(self, blueprint_filter):
        blueprint_library = self.world.get_blueprint_library()
        candidates = blueprint_library.filter(blueprint_filter)
        self.assertTrue(len(candidates) > 0, f"no blueprint matched {blueprint_filter}")
        blueprint = random.choice(candidates)
        spawn_points = self.world.get_map().get_spawn_points()
        self.assertTrue(len(spawn_points) > 0, "map has no spawn points")
        for spawn_point in spawn_points:
            actor = self.world.try_spawn_actor(blueprint, spawn_point)
            if actor is not None:
                self.spawned.append(actor)
                self.world.tick() if self.world.get_settings().synchronous_mode else self.world.wait_for_tick()
                return actor
        self.fail(f"could not spawn any {blueprint_filter} actor")

    def test_vehicle_components(self):
        print("TestActorIntrospection.test_vehicle_components")
        vehicle = self._spawn("vehicle.*")

        component_names = vehicle.get_component_names()
        self.assertGreater(len(component_names), 0, "vehicle has zero components")
        self.assertTrue(
            any("Mesh" in name for name in component_names),
            f"no mesh-like component in {component_names}",
        )

        # Probe the world transform of the first scene-component-bearing
        # name and verify it lands inside a 200 m envelope around the
        # vehicle (sanity bound, the spawn point is at the origin scale
        # of CARLA towns).
        vehicle_location = vehicle.get_location()
        probed_any = False
        for name in component_names:
            try:
                component_tf = vehicle.get_component_world_transform(name)
            except RuntimeError:
                # Non-USceneComponent components (UActorComponent only)
                # legitimately return ComponentNotFound here.
                continue
            self.assertTrue(math.isfinite(component_tf.location.x))
            self.assertLess(
                abs(component_tf.location.x - vehicle_location.x), 200.0,
                f"{name} world x {component_tf.location.x} far from vehicle {vehicle_location.x}",
            )
            probed_any = True
        self.assertTrue(probed_any, "no scene component world transform probed")

    def test_vehicle_sockets_consistent(self):
        print("TestActorIntrospection.test_vehicle_sockets_consistent")
        vehicle = self._spawn("vehicle.*")

        socket_names = vehicle.get_socket_names()
        socket_world = vehicle.get_socket_world_transforms()
        socket_rel = vehicle.get_socket_relative_transforms()

        self.assertEqual(
            len(socket_names), len(socket_world),
            "socket name count must match world transform count",
        )
        self.assertEqual(
            len(socket_world), len(socket_rel),
            "world and relative socket transform counts must match",
        )

    def test_walker_bones(self):
        print("TestActorIntrospection.test_walker_bones")
        blueprints = sorted(
            self.world.get_blueprint_library().filter("walker.pedestrian.*"),
            key=lambda bp: bp.id,
        )
        self.assertTrue(len(blueprints) > 0, "no walker.pedestrian.* blueprint")
        spawn_points = self.world.get_map().get_spawn_points()
        self.assertTrue(len(spawn_points) > 0, "map has no spawn points")

        report = []
        failures = []
        for blueprint in blueprints:
            generation = (
                blueprint.get_attribute("generation").as_int()
                if blueprint.has_attribute("generation") else None
            )
            walker = None
            for spawn_point in spawn_points:
                walker = self.world.try_spawn_actor(blueprint, spawn_point)
                if walker is not None:
                    break
            if walker is None:
                failures.append(f"{blueprint.id} (gen {generation}): could not spawn")
                continue
            try:
                self.world.wait_for_tick()
                bone_names = walker.get_bone_names()
                bone_world = walker.get_bone_world_transforms()
                bone_rel = walker.get_bone_relative_transforms()
            finally:
                walker.destroy()

            report.append(
                f"{blueprint.id} gen={generation} bones={len(bone_names)} "
                f"world={len(bone_world)} rel={len(bone_rel)}"
            )
            min_bones = WALKER_MIN_BONES_BY_GENERATION.get(generation, 1)
            if len(bone_names) < min_bones:
                failures.append(
                    f"{blueprint.id} (gen {generation}): {len(bone_names)} bones, "
                    f"expected at least {min_bones}"
                )
            if not len(bone_names) == len(bone_world) == len(bone_rel):
                failures.append(
                    f"{blueprint.id} (gen {generation}): {len(bone_names)} names, "
                    f"{len(bone_world)} world, {len(bone_rel)} relative transforms"
                )

        print("\n".join(report))
        self.assertFalse(
            failures,
            f"{len(failures)}/{len(blueprints)} walkers failed:\n" + "\n".join(failures),
        )

    def test_component_not_found_raises(self):
        print("TestActorIntrospection.test_component_not_found_raises")
        vehicle = self._spawn("vehicle.*")
        # rpclib does not preserve server-side error message text through
        # the RPC boundary (it surfaces as a generic "std::exception"),
        # so this only verifies that the server signals failure rather
        # than returning a junk transform for an unknown component.
        with self.assertRaises(RuntimeError):
            vehicle.get_component_world_transform("NonExistentComponent_zzz_42")
