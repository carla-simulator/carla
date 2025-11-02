# Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla
import time

from . import SyncSmokeTest


class TestSpawnpoints(SyncSmokeTest):
    @staticmethod
    def diff_msg(name, exp, got):
        return f"{name}: expected={exp:.4f}, got={got:.4f}, diff={abs(exp-got):.4f}"


    def test_spawn_points(self):
        print("TestSpawnpoints.test_spawn_points")
        self.world = self.client.get_world()
        blueprints = self.world.get_blueprint_library().filter("vehicle.*")
        blueprints = self.filter_vehicles_for_old_towns(blueprints)

        # get all available maps
        maps = ['Town01', 'Town01_Opt', 'Town02', 'Town02_Opt', 'Town03', 'Town03_Opt', 'Town04', 'Town04_Opt', 'Town05', 'Town05_Opt', 'Town10HD', 'Town10HD_Opt']
        for m in maps:
            # load the map
            self.client.load_world(m)
            # workaround: give time to UE4 to clean memory after loading (old assets)
            time.sleep(5)
            
            self.world.tick()
            self.world.tick()

            self.world = self.client.get_world()

            # get all spawn points
            spawn_points = self.world.get_map().get_spawn_points()
            


            # Check why the world settings aren't applied after a reload
            self.settings = self.world.get_settings()
            settings = carla.WorldSettings(
                no_rendering_mode=False,
                synchronous_mode=True,
                fixed_delta_seconds=0.05)
            self.world.apply_settings(settings)

            # spawn all kind of vehicle
            for vehicle in blueprints:
                batch = [(vehicle, t) for t in spawn_points]
                batch = [carla.command.SpawnActor(*args) for args in batch]
                response = self.client.apply_batch_sync(batch, False)

                # Detailed spawn errors (index, bp id, actor_id, transform, error)
                spawn_errors = []
                for i, (resp, (bp, t)) in enumerate(zip(response, [(vehicle, t) for t in spawn_points])):
                    if resp.error:
                        spawn_errors.append(
                            f"idx={i}, bp={getattr(bp, 'id', str(bp))}, "
                            f"actor_id={resp.actor_id}, "
                            f"loc=({t.location.x:.3f},{t.location.y:.3f},{t.location.z:.3f}), "
                            f"rot=({t.rotation.pitch:.2f},{t.rotation.yaw:.2f},{t.rotation.roll:.2f}), "
                            f"error={resp.error}"
                        )
                        self.world.get_spectator().set_transform(t)

                self.assertFalse(
                    bool(spawn_errors),
                    "Spawn errors detected:\n" + "\n".join(f"  - {e}" for e in spawn_errors)
                    if spawn_errors else "Spawn errors detected (no details)"
                )

                ids = [x.actor_id for x in response]
                self.assertEqual(
                    len(ids), len(spawn_points),
                    (
                        "Mismatch in number of spawned actors.\n"
                        f"Expected (spawn points): {len(spawn_points)}\n"
                        f"Got (actor ids): {len(ids)}\n"
                        f"IDs: {ids}"
                    )
                )

                frame = self.world.tick()
                snapshot = self.world.get_snapshot()
                self.assertEqual(
                    frame, snapshot.timestamp.frame,
                    (
                        "Frame mismatch between world.tick() and snapshot.\n"
                        f"tick frame={frame}, snapshot frame={snapshot.timestamp.frame}"
                    )
                )

                actors = self.world.get_actors()
                missing = [a.id for a in actors if not snapshot.has_actor(a.id)]
                self.assertTrue(
                    len(missing) == 0,
                    (
                        "Some actors are missing from snapshot.\n"
                        f"Missing IDs: {missing}\n"
                        f"Total actors in world: {len(actors)}"
                    )
                )

                for actor_id, t0 in zip(ids, spawn_points):
                    actor_snapshot = snapshot.find(actor_id)
                    self.assertIsNotNone(
                        actor_snapshot,
                        (
                            "Actor not found in snapshot.\n"
                            f"actor_id={actor_id}\n"
                            f"spawn loc=({t0.location.x:.4f},{t0.location.y:.4f},{t0.location.z:.4f}), "
                            f"rot=({t0.rotation.pitch:.2f},{t0.rotation.yaw:.2f},{t0.rotation.roll:.2f})"
                        )
                    )
                    if actor_snapshot:
                        t1 = actor_snapshot.get_transform()


                        # Ignore Z cause vehicle is falling.
                        self.assertAlmostEqual(
                            t0.location.x, t1.location.x, places=2,
                            msg=(
                                "X position mismatch.\n"
                                f"actor_id={actor_id}\n"
                                f"{TestSpawnpoints.diff_msg('x', t0.location.x, t1.location.x)}"
                            )
                        )
                        self.assertAlmostEqual(
                            t0.location.x, t1.location.x, places=2,
                            msg=(
                                "Y position mismatch.\n"
                                f"actor_id={actor_id}\n"
                                f"{TestSpawnpoints.diff_msg('x', t0.location.y, t1.location.y)}"
                            )
                        )
                        self.assertAlmostEqual(
                            t0.rotation.pitch, t1.rotation.pitch, places=2,
                            msg=(
                                "Pitch mismatch.\n"
                                f"actor_id={actor_id}\n"
                                f"{TestSpawnpoints.diff_msg('pitch', t0.rotation.pitch, t1.rotation.pitch)}"
                            )
                        )
                        self.assertAlmostEqual(
                            t0.rotation.yaw, t1.rotation.yaw, places=2,
                            msg=(
                                "Yaw mismatch.\n"
                                f"actor_id={actor_id}\n"
                                f"{TestSpawnpoints.diff_msg('yaw', t0.rotation.yaw, t1.rotation.yaw)}"
                            )
                        )
                        self.assertAlmostEqual(
                            t0.rotation.roll, t1.rotation.roll, places=2,
                            msg=(
                                "Roll mismatch.\n"
                                f"actor_id={actor_id}\n"
                                f"{TestSpawnpoints.diff_msg('roll', t0.rotation.roll, t1.rotation.roll)}"
                            )
                        )

                destroy_resp = self.client.apply_batch_sync([carla.command.DestroyActor(x) for x in ids], True)
                destroy_errors = [f"actor_id={r.actor_id}, error={r.error}" for r in destroy_resp if r.error]
                self.assertFalse(
                    bool(destroy_errors),
                    "Errors while destroying actors:\n" + "\n".join(f"  - {e}" for e in destroy_errors)
                    if destroy_errors else "Errors while destroying actors (no details)"
                )
                frame = self.world.tick()
                frame = self.world.tick()
