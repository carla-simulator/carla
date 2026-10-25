# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""File-writing regression for the color-coded optical-flow image."""

from pathlib import Path
from queue import Empty, Queue
import struct
import tempfile

import carla

from . import SyncSmokeTest


class TestOpticalFlowImage(SyncSmokeTest):
    def test_color_coded_flow_save_to_disk(self):
        blueprint = self.world.get_blueprint_library().find(
            'sensor.camera.optical_flow')
        blueprint.set_attribute('image_size_x', '32')
        blueprint.set_attribute('image_size_y', '24')
        camera = self.world.spawn_actor(
            blueprint, self.world.get_spectator().get_transform())
        try:
            frames = Queue()
            camera.listen(frames.put)
            flow = None
            for _ in range(10):
                self.world.tick()
                try:
                    flow = frames.get(timeout=2.0)
                    break
                except Empty:
                    pass
            self.assertIsNotNone(flow, 'optical-flow camera produced no frame')
            image = flow.get_color_coded_flow()
            before = bytes(image.raw_data)
            with tempfile.TemporaryDirectory() as directory:
                for name, explicit_converter in [('default.png', False),
                                                 ('raw.png', True)]:
                    with self.subTest(filename=name):
                        path = str(Path(directory) / name)
                        if explicit_converter:
                            saved = image.save_to_disk(
                                path=path, color_converter=carla.ColorConverter.Raw)
                        else:
                            saved = image.save_to_disk(path)
                        self.assertEqual(saved, path)
                        contents = Path(path).read_bytes()
                        self.assertEqual(contents[:8], b'\x89PNG\r\n\x1a\n')
                        self.assertEqual(contents[12:16], b'IHDR')
                        width, height, bits, color = struct.unpack('>IIBB', contents[16:26])
                        self.assertEqual((width, height), (image.width, image.height))
                        self.assertEqual((bits, color), (8, 2))  # RGB, no unused alpha.
                        self.assertEqual(bytes(image.raw_data), before)
        finally:
            try:
                camera.stop()
            finally:
                camera.destroy()
