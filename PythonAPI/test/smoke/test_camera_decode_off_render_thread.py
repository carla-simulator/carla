# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Regression test for the batched-camera readback flush (issue #9902, part of
#9897): decode/serialize/publish now run on a background task instead of
inside the render-thread-blocking flush command. Checks that moving this work
off the render thread did not break per-sensor delivery ordering, frame/
transform coherence, or sensor-destruction safety while a delivery is still
in flight.
"""

from . import SyncSmokeTest

import carla

try:
    from queue import Queue, Empty
except ImportError:
    from Queue import Queue, Empty


class TestCameraDecodeOffRenderThread(SyncSmokeTest):
    def tearDown(self):
        self.client.load_world('Town10HD_Opt')
        self.world = None
        self.client = None

    def test_multi_camera_frame_order_and_transform_coherence(self):
        print("TestCameraDecodeOffRenderThread.test_multi_camera_frame_order_and_transform_coherence")

        cameras = []
        try:
            bp_lib = self.world.get_blueprint_library()
            cam_bp = bp_lib.find('sensor.camera.rgb')
            cam_bp.set_attribute('image_size_x', '128')
            cam_bp.set_attribute('image_size_y', '128')

            queues = []
            for _ in range(2):
                start = carla.Transform(carla.Location(z=10.0))
                camera = self.world.spawn_actor(cam_bp, start)
                cameras.append(camera)
                q = Queue()
                camera.listen(q.put)
                queues.append(q)

            # Keyed by (camera.id, frame) and read back from the server right
            # after tick(), not the locally-intended value: set_transform() is
            # a separate RPC from tick(), and comparing against what the test
            # asked for (rather than what the server had actually applied by
            # that tick) is itself racy independent of anything under test.
            actual_transform_by_camera_frame = {}
            num_ticks = 30
            for i in range(num_ticks):
                t = carla.Transform(carla.Location(x=float(i), z=10.0))
                for camera in cameras:
                    camera.set_transform(t)
                self.world.tick()
                frame = self.world.get_snapshot().frame
                for camera in cameras:
                    actual_transform_by_camera_frame[(camera.id, frame)] = camera.get_transform()

            for camera, q in zip(cameras, queues):
                images = []
                while True:
                    try:
                        images.append(q.get(True, 6.0))
                    except Empty:
                        break

                self.assertEqual(
                    len(images), num_ticks,
                    "camera %s delivered %d frames for %d ticks (dropped frames)"
                    % (camera.id, len(images), num_ticks))

                frames = [image.frame for image in images]
                self.assertEqual(
                    frames, sorted(frames),
                    "camera %s delivered frames out of order: %s "
                    "(background delivery tasks must stay in per-sensor order)"
                    % (camera.id, frames))

                for image in images:
                    expected = actual_transform_by_camera_frame.get((camera.id, image.frame))
                    self.assertIsNotNone(
                        expected,
                        "camera %s delivered frame %d, which was never ticked"
                        % (camera.id, image.frame))
                    self.assertAlmostEqual(
                        image.transform.location.x, expected.location.x, delta=1e-2,
                        msg="camera %s frame %d: delivered transform x=%.3f does not "
                            "match that tick's actual x=%.3f"
                            % (camera.id, image.frame, image.transform.location.x,
                               expected.location.x))
        finally:
            for camera in cameras:
                camera.stop()
                camera.destroy()

    def test_destroy_camera_while_delivery_in_flight(self):
        print("TestCameraDecodeOffRenderThread.test_destroy_camera_while_delivery_in_flight")

        bp_lib = self.world.get_blueprint_library()
        cam_bp = bp_lib.find('sensor.camera.rgb')
        cam_bp.set_attribute('image_size_x', '256')
        cam_bp.set_attribute('image_size_y', '256')

        for _ in range(5):
            camera = self.world.spawn_actor(cam_bp, carla.Transform(carla.Location(z=10.0)))
            camera.listen(lambda image: None)
            # Capture is enqueued by this tick; destroy immediately after, while
            # its decode/publish may still be dispatched on a background task.
            self.world.tick()
            camera.stop()
            camera.destroy()

        for _ in range(10):
            self.world.tick()

        # Force a real garbage-collection pass (a level transition triggers
        # one), so this actually exercises IsValid(this) after the sensor's
        # memory may have been reclaimed, not just after EndPlay.
        self.world = self.client.load_world('Town10HD_Opt')
        self.world.apply_settings(carla.WorldSettings(
            synchronous_mode=True, fixed_delta_seconds=0.05))
        for _ in range(5):
            self.world.tick()
        self.assertIsNotNone(self.world.get_snapshot(), "server did not survive destroying cameras with deliveries in flight")
