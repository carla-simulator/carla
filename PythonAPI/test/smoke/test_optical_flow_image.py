# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""File-writing regression for the color-coded optical-flow image."""

from pathlib import Path
from queue import Empty, Queue
import tempfile

import carla
import cv2
import numpy as np

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
            expected = np.frombuffer(image.raw_data, dtype=np.uint8).reshape(
                image.height, image.width, 4)[:, :, :3].copy()
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
                        actual = cv2.imread(path, cv2.IMREAD_UNCHANGED)
                        self.assertIsNotNone(actual)
                        np.testing.assert_array_equal(actual[:, :, :3], expected)
                        if actual.shape[2] == 4:
                            self.assertTrue(np.all(actual[:, :, 3] == 255))
        finally:
            try:
                camera.stop()
            finally:
                camera.destroy()
