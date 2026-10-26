"""Real skill rendering/projection with native geometry and inert RPC boundaries."""
import fnmatch
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
from types import SimpleNamespace as NS
import unittest
from unittest.mock import patch

import carla
import numpy as np

ROOT = Path(__file__).resolve().parents[3]


def skill(name, filename):
    path = ROOT / "Skills/python-api" / name / "scripts" / filename
    spec = importlib.util.spec_from_file_location(name.replace("-", "_"), path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TestSpecialCameraFrames(unittest.TestCase):
    def setUp(self):
        self.module = skill("read-sensor", "read_sensor.py")

    @staticmethod
    def dvs(events):
        return NS(width=3, height=2,
                  raw_data=b"".join(struct.pack("<HHq?", *event) for event in events))

    def test_dvs_event_payload_becomes_polarity_image(self):
        data = self.dvs([(1, 0, 100, True), (2, 1, 101, False)])
        image = self.module._render_frame(data, "sensor.camera.dvs", carla.ColorConverter.Raw)
        np.testing.assert_array_equal(image[0, 1], [0, 0, 255])
        np.testing.assert_array_equal(image[1, 2], [255, 0, 0])
        self.assertEqual(np.count_nonzero(image), 2)

    def test_empty_dvs_packet_is_a_black_image(self):
        image = self.module._render_frame(self.dvs([]), "sensor.camera.dvs", carla.ColorConverter.Raw)
        self.assertEqual(image.shape, (2, 3, 3))
        self.assertEqual(np.count_nonzero(image), 0)

    def test_optical_flow_uses_native_colour_image_boundary(self):
        # ColorCodedFlow returns FakeImage: BGRA bytes, without Image.convert.
        coded = NS(width=1, height=1, raw_data=bytes([10, 20, 30, 0]))
        flow = NS(get_color_coded_flow=lambda: coded)
        image = self.module._render_frame(flow, "sensor.camera.optical_flow", carla.ColorConverter.Raw)
        np.testing.assert_array_equal(image, [[[30, 20, 10]]])

    def test_rgb_conversion_control(self):
        image = NS(width=1, height=1, raw_data=bytes([1, 2, 3, 255]), convert=lambda _: None)
        np.testing.assert_array_equal(
            self.module._render_frame(image, "sensor.camera.rgb", carla.ColorConverter.Raw),
            [[[3, 2, 1]]])

    def test_dvs_save_command_writes_png(self):
        try:
            import cv2
        except ImportError:
            self.skipTest("OpenCV is required for PNG readback")
        data = self.dvs([(1, 0, 100, True)])
        data.frame, data.timestamp = 1, 0.05
        sensor = NS(id=7, type_id="sensor.camera.dvs", listen=lambda cb: cb(data), stop=lambda: None)
        world = NS(get_settings=lambda: carla.WorldSettings(), apply_settings=lambda _: None, tick=lambda: None)
        with tempfile.TemporaryDirectory() as tmp:
            args = NS(out=tmp, delta=0.05, no_sync=False, warmup=0, frames=1,
                      seconds=1, frame_timeout=0.01, queue_max=3)
            with patch.object(self.module, "_client", return_value=NS(get_world=lambda: world)), \
                    patch.object(self.module, "_resolve", return_value=sensor):
                self.module.cmd_save(args)
            image = cv2.imread(str(Path(tmp) / "00000001.png"))
            self.assertIsNotNone(image)
            np.testing.assert_array_equal(image[0, 1], [255, 0, 0])


class Actors(list):
    def filter(self, pattern):
        return Actors(a for a in self if fnmatch.fnmatchcase(a.type_id, pattern))

    def find(self, actor_id):
        return next((actor for actor in self if actor.id == actor_id), None)


class TestNearCameraBoxes(unittest.TestCase):
    def project(self, center, extent):
        try:
            import cv2  # noqa: F401 - actual writer must be present
        except ImportError:
            self.skipTest("OpenCV is required for annotated output")
        image = NS(width=800, height=600, raw_data=bytes(800 * 600 * 4))
        camera = NS(id=1, type_id="sensor.camera.rgb", parent=None,
                    attributes={"fov": "90"}, get_transform=lambda: carla.Transform(),
                    listen=lambda cb: cb(image), stop=lambda: None)
        actor = NS(id=2, type_id="vehicle.test", bounding_box=carla.BoundingBox(carla.Location(), extent),
                   get_transform=lambda: carla.Transform(center))
        actors = Actors([camera, actor])
        world = NS(get_actors=lambda: actors)
        module = skill("bounding-boxes", "bounding_boxes.py")
        with tempfile.TemporaryDirectory() as tmp:
            output = Path(tmp) / "boxes.png"
            args = NS(camera="1", filter="vehicle.*", out=str(output), max_dist=100)
            with patch.object(module, "_world", return_value=world):
                module.cmd_project(args)
            self.assertTrue(output.is_file())
            return json.loads(output.with_suffix(".json").read_text())

    def test_box_crossing_camera_plane_includes_visible_sides(self):
        boxes = self.project(carla.Location(1, 2, 0), carla.Vector3D(2, 1, 1))
        self.assertEqual(len(boxes), 1)
        self.assertEqual(boxes[0]["bbox"], [533, 0, 799, 599])

    def test_camera_inside_box_is_clipped_to_image(self):
        boxes = self.project(carla.Location(), carla.Vector3D(1, 1, 1))
        self.assertEqual(boxes[0]["bbox"], [0, 0, 799, 599])

    def test_ordinary_front_box_control(self):
        boxes = self.project(carla.Location(10, 0, 0), carla.Vector3D(1, 1, 1))
        self.assertEqual(boxes[0]["bbox"], [355, 255, 444, 344])

    def test_wholly_behind_box_control(self):
        self.assertEqual(self.project(carla.Location(-5, 0, 0), carla.Vector3D(1, 1, 1)), [])


if __name__ == "__main__":
    unittest.main()
