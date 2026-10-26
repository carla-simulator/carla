# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB). This work is licensed under the MIT license.

import importlib.util
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import carla


def load_utility(name):
    source = Path(__file__).resolve().parents[2] / ('util/' + name + '.py')
    spec = importlib.util.spec_from_file_location(name, source)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TestTrafficLightDebugBoxes(unittest.TestCase):
    def check_boxes(self, actor_rotation, box_rotation, trigger_rotation):
        transform = carla.Transform(carla.Location(12, -8, 3), actor_rotation)
        box = carla.BoundingBox(carla.Location(1, 2, 3), carla.Vector3D(0.5, 1, 2))
        box.rotation = box_rotation
        trigger = carla.BoundingBox(carla.Location(-2, 1, 0.5), carla.Vector3D(3, 1, 0.5))
        trigger.rotation = trigger_rotation
        def copy_box(source):
            result = carla.BoundingBox(source.location, source.extent)
            result.rotation = source.rotation
            return result
        # Actor bindings return copies of these local boxes. Keep that boundary
        # contract while using real native boxes/transforms for all geometry.
        actor_type = type('TrafficLight', (), {
            'get_transform': lambda self: transform,
            'bounding_box': property(lambda self: copy_box(box)),
            'trigger_volume': property(lambda self: copy_box(trigger))})
        actor = actor_type()
        actual = load_utility('show_traffic_lights').get_traffic_light_bbs(actor)
        for source, world_box in zip((box, trigger), actual):
            expected = source.get_world_vertices(transform)
            vertices = world_box.get_world_vertices(carla.Transform())
            for original, converted in zip(expected, vertices):
                self.assertLess(original.distance(converted), 3e-5)
        self.assertEqual(box.rotation, box_rotation)
        self.assertEqual(trigger.rotation, trigger_rotation)

    def test_local_yaw_is_composed_with_actor_yaw(self):
        self.check_boxes(carla.Rotation(yaw=90), carla.Rotation(yaw=45), carla.Rotation(yaw=-30))

    def test_full_three_dimensional_rotations(self):
        self.check_boxes(carla.Rotation(23, -71, 17), carla.Rotation(-31, 42, 12), carla.Rotation(11, 52, -29))

    def test_unrotated_boxes_control(self):
        self.check_boxes(carla.Rotation(), carla.Rotation(), carla.Rotation())


class TestCrosswalkDebugPolygons(unittest.TestCase):
    @staticmethod
    def native_map(outlines):
        objects = []
        for index, outline in enumerate(outlines):
            corners = ''.join('<cornerLocal u="{}" v="{}" z="0"/>'.format(*point) for point in outline)
            objects.append('<object id="{}" name="crosswalk" type="crosswalk" s="{}" t="0" '
                           'zOffset="0" hdg="0" pitch="0" roll="0" length="4" width="3" '
                           'orientation="+"><outline>{}</outline></object>'.format(index + 1, 10 + index * 10, corners))
        xodr = ('<OpenDRIVE><header revMajor="1" revMinor="4"/><road id="1" name="road" length="100" junction="-1">'
                '<planView><geometry s="0" x="0" y="0" hdg="0" length="100"><line/></geometry></planView>'
                '<elevationProfile><elevation s="0" a="0" b="0" c="0" d="0"/></elevationProfile>'
                '<lanes><laneSection s="0"><center><lane id="0" type="none" level="false"/></center><right>'
                '<lane id="-1" type="driving" level="false"><width sOffset="0" a="3.5" b="0" c="0" d="0"/>'
                '</lane></right></laneSection></lanes><objects>' + ''.join(objects) + '</objects></road></OpenDRIVE>')
        return carla.Map('crosswalk-shapes', xodr)

    def check_outlines(self, outlines):
        tmap = self.native_map(outlines)
        points = tmap.get_crosswalks()
        self.assertEqual(len(points), sum(map(len, outlines)))
        lines = []
        debug = SimpleNamespace(draw_string=lambda *args, **kwargs: None,
                                draw_line=lambda first, second, **kwargs: lines.append((first, second)))
        world = SimpleNamespace(get_map=lambda: tmap, debug=debug)
        client = SimpleNamespace(get_world=lambda: world)
        module = load_utility('show_crosswalks')
        with patch.object(module.carla, 'Client', return_value=client), \
                patch.object(sys, 'argv', ['show_crosswalks.py']):
            module.main()
        expected = []
        offset = 0
        for outline in outlines:
            polygon = points[offset:offset + len(outline)]
            expected.extend(zip(polygon, polygon[1:]))
            offset += len(outline)
        self.assertEqual(len(lines), len(expected))
        for actual, original in zip(lines, expected):
            for vertex, source in zip(actual, original):
                self.assertLess(vertex.distance(source + carla.Location(z=0.5)), 1e-5)

    def test_triangle(self):
        self.check_outlines([[(-2, -1), (2, -1), (0, 2), (-2, -1)]])

    def test_mixed_polygons_do_not_connect_neighbors(self):
        self.check_outlines([[(-2, -1), (2, -1), (0, 2), (-2, -1)],
                             [(-2, -1), (1, -2), (2, 0), (1, 2), (-2, 1), (-2, -1)]])

    def test_rectangle_control(self):
        self.check_outlines([[(-2, -1), (2, -1), (2, 1), (-2, 1), (-2, -1)]])


if __name__ == '__main__':
    unittest.main()
