# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke test: G-buffer calls without G-buffer support must not crash the server."""

from . import SyncSmokeTest

import carla


class TestGBuffer(SyncSmokeTest):
    """A G-buffer request on a build without G-buffers is refused, not fatal.

    The server never opens a camera's G-buffer streams (SensorFactory leaves
    them unassigned), and get_gbuffer_token, which both listen_to_gbuffer() and
    stop_gbuffer() call, used to read the token of one of them, which took the
    server down. G-buffer capture is compiled in only when the engine provides
    GBufferView.h, which the Unreal Engine 5.8 fork does not, so such a build
    now refuses the request.
    """

    def test_gbuffer_unsupported_keeps_server_alive(self):
        """listen_to_gbuffer() and stop_gbuffer() raise, nothing is left listening, the server still ticks."""
        print("TestGBuffer.test_gbuffer_unsupported_keeps_server_alive")
        bp = self.world.get_blueprint_library().find('sensor.camera.rgb')
        camera = self.world.spawn_actor(bp, carla.Transform(carla.Location(z=2.0)))
        gbuffer_id = carla.GBufferTextureID.SceneColor
        try:
            with self.assertRaisesRegex(RuntimeError, r'GBuffer capture is not available in this build'):
                camera.listen_to_gbuffer(gbuffer_id, lambda _image: None)
            self.assertFalse(camera.is_listening_gbuffer(gbuffer_id))
            with self.assertRaisesRegex(RuntimeError, r'GBuffer capture is not available in this build'):
                camera.stop_gbuffer(gbuffer_id)
            # The server is still up and answers.
            self.assertGreater(self.world.tick(), 0)
        finally:
            camera.destroy()
