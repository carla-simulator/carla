# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke test: a client that stops reading a stream must not stall the server."""

import threading
import time

import carla

from . import SyncSmokeTest


class TestStreamSlowClient(SyncSmokeTest):
    """A client that stops reading a sensor stream must not stall the server.

    In synchronous mode the server used to wait without a limit for the
    previous message of a stream to be sent before writing the next one. A
    listen() callback that never returned therefore froze the render thread,
    then the game thread, and every RPC of every client timed out (#10233).
    """

    def test_blocked_camera_callback_does_not_stall_server(self):
        """Ticks and other clients keep working while a callback is stuck."""
        print("TestStreamSlowClient.test_blocked_camera_callback_does_not_stall_server")
        bp = self.world.get_blueprint_library().find('sensor.camera.rgb')
        # Larger than the socket buffers, so an unread frame blocks the next one.
        bp.set_attribute('image_size_x', '1920')
        bp.set_attribute('image_size_y', '1080')
        camera = self.world.spawn_actor(bp, carla.Transform(carla.Location(z=50)))
        release = threading.Event()
        blocked = threading.Event()

        def callback(_image):
            blocked.set()
            release.wait()

        try:
            camera.listen(callback)
            for _ in range(5):
                self.world.tick(30.0)
            self.assertTrue(blocked.wait(30.0), "the camera never delivered an image")

            # Each tick below would have hung for good. The server now drops
            # the stuck session once the streaming timeout (10 s) expires.
            start = time.time()
            for _ in range(10):
                self.world.tick(30.0)
            self.assertLess(time.time() - start, 40.0)

            # Another client still gets answers.
            other = carla.Client(*self.testing_address)
            other.set_timeout(10.0)
            self.assertTrue(other.get_world().get_settings().synchronous_mode)
        finally:
            release.set()
            camera.stop()
            camera.destroy()
