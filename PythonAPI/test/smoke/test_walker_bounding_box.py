# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import time

import carla

from . import SyncSmokeTest


WORLD = "Town10HD_Opt"


class TestWalkerBoundingBox(SyncSmokeTest):
    """Regression coverage for issue #9610.

    Before the fix, ``walker.bounding_box.location`` was always (0, 0, 0):
    ``UBoundingBoxCalculator::GetActorBoundingBox`` returned the skeletal-mesh
    AABB without lifting it through the mesh component's relative transform,
    so the box ended up centred on the actor pivot instead of on the mesh.
    """

    def setUp(self):
        super(TestWalkerBoundingBox, self).setUp()
        if self.world.get_map().name.split("/")[-1] != WORLD:
            self.client.load_world(WORLD)
            time.sleep(5)
            self.world = self.client.get_world()
            settings = carla.WorldSettings(
                no_rendering_mode=False,
                synchronous_mode=True,
                fixed_delta_seconds=0.05)
            self.world.apply_settings(settings)
            self.world.tick()

    def tearDown(self):
        # SmokeTest.tearDown() loads Town03, which is not shipped in the
        # packaged build. Reload Town10HD_Opt instead so the next test starts
        # from a known good state.
        self.world.apply_settings(self.settings)
        self.world.tick()
        self.settings = None
        self.client.load_world(WORLD)
        time.sleep(5)
        self.world = None
        self.client = None

    def _spawn_walker(self):
        walker_blueprints = self.world.get_blueprint_library().filter(
            "walker.pedestrian.*")
        self.assertTrue(
            len(walker_blueprints) > 0,
            "No walker blueprints available in the loaded world")
        walker_bp = walker_blueprints[0]
        if walker_bp.has_attribute("is_invincible"):
            walker_bp.set_attribute("is_invincible", "false")

        spawn_location = self.world.get_random_location_from_navigation()
        self.assertIsNotNone(
            spawn_location,
            "Navigation mesh is unavailable; cannot spawn a walker")
        spawn_location.z += 2.0
        spawn_transform = carla.Transform(spawn_location)
        walker = self.world.spawn_actor(walker_bp, spawn_transform)
        self.assertIsNotNone(walker, "Failed to spawn a walker actor")
        # Tick a few frames so the skeletal mesh evaluates its current pose
        # and the cached bounds reflect a real animation frame.
        for _ in range(3):
            self.world.tick()
        return walker

    def test_walker_bounding_box_has_relative_offset(self):
        walker = self._spawn_walker()
        try:
            bounding_box = walker.bounding_box

            self.assertGreater(
                bounding_box.extent.x, 0.0,
                "Walker bounding box has degenerate X extent")
            self.assertGreater(
                bounding_box.extent.y, 0.0,
                "Walker bounding box has degenerate Y extent")
            self.assertGreater(
                bounding_box.extent.z, 0.5,
                "Walker bounding box Z half-extent is implausibly small for a "
                "human-sized pedestrian")

            # Regression guard for issue #9610: before the fix the walker
            # bounding box was centred on the actor pivot, i.e. (0, 0, 0) in
            # actor-local space. After the fix the mesh-component offset
            # (typically the capsule half-height on Z) must be visible here.
            self.assertGreater(
                abs(bounding_box.location.z), 0.01,
                "Walker bounding box location.z is essentially zero; the "
                "relative transform of the skeletal mesh component is being "
                "discarded (issue #9610)")

            # World-space sanity: the box centre, once placed via the walker
            # transform, must sit above the walker's feet and below ~2.5 m.
            world_centre = walker.get_transform().transform(
                bounding_box.location)
            foot_z = walker.get_location().z
            self.assertGreaterEqual(
                world_centre.z, foot_z - 0.1,
                "Walker bounding box centre is below the walker's feet")
            self.assertLessEqual(
                world_centre.z, foot_z + 2.5,
                "Walker bounding box centre is unrealistically high above the "
                "walker's feet")

            # After the Copilot review fix on PR #9729 the walker BB rotation
            # field must reflect the mesh-component rotation: stock walkers
            # carry roughly a -90 deg yaw on the skeletal mesh component, so
            # bb.rotation.yaw must be non-trivial. A zero rotation is the
            # pre-fix state and would mean the mesh frame is being silently
            # dropped.
            self.assertGreater(
                abs(bounding_box.rotation.yaw), 1.0,
                "Walker bounding box rotation.yaw is zero; the mesh-component "
                "rotation is not being lifted into the box (Copilot finding "
                "on PR #9729)")

            # Corner-consistency: the 8 world-space vertices of
            # actor.bounding_box, lifted through walker.get_transform(), must
            # span a plausible humanoid envelope (>1 m tall, narrower than
            # 1.5 m on the horizontal axes).
            corners = bounding_box.get_world_vertices(walker.get_transform())
            xs = [c.x for c in corners]
            ys = [c.y for c in corners]
            zs = [c.z for c in corners]
            self.assertGreater(
                max(zs) - min(zs), 1.0,
                "Walker bounding box world-space height is implausibly small")
            self.assertLess(
                max(xs) - min(xs), 1.5,
                "Walker bounding box world-space X-span is unrealistically "
                "wide")
            self.assertLess(
                max(ys) - min(ys), 1.5,
                "Walker bounding box world-space Y-span is unrealistically "
                "wide")
        finally:
            walker.destroy()
            self.world.tick()
