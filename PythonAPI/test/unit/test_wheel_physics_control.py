# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import unittest

import carla


class TestWheelPhysicsControl(unittest.TestCase):
    def assert_graph_equal(self, graph, points):
        self.assertIsInstance(graph, list)
        self.assertEqual(len(graph), len(points))
        for point, (x, y) in zip(graph, points):
            self.assertAlmostEqual(point.x, x, places=5)
            self.assertAlmostEqual(point.y, y, places=5)

    def test_lateral_slip_graph_constructor(self):
        for graph in ([carla.Vector2D(0.0, 1.0), carla.Vector2D(20.0, 0.5)],
                      [(0.0, 1.0), (20.0, 0.5)],
                      ((0.0, 1.0), (20.0, 0.5))):
            with self.subTest(graph=graph):
                wheel = carla.WheelPhysicsControl(lateral_slip_graph=graph)
                self.assert_graph_equal(wheel.lateral_slip_graph,
                                        [(0.0, 1.0), (20.0, 0.5)])

    def test_lateral_slip_graph_assignment_and_empty(self):
        wheel = carla.WheelPhysicsControl()
        self.assert_graph_equal(wheel.lateral_slip_graph, [])
        wheel.lateral_slip_graph = [[1.0, 0.75], [2.0, 0.25]]
        self.assert_graph_equal(wheel.lateral_slip_graph,
                                [(1.0, 0.75), (2.0, 0.25)])
        snapshot = wheel.lateral_slip_graph
        snapshot.append(carla.Vector2D(3.0, 0.0))
        self.assert_graph_equal(wheel.lateral_slip_graph,
                                [(1.0, 0.75), (2.0, 0.25)])
        wheel.lateral_slip_graph = []
        self.assert_graph_equal(wheel.lateral_slip_graph, [])

    def test_lateral_slip_graph_invalid_assignment_is_atomic(self):
        wheel = carla.WheelPhysicsControl(lateral_slip_graph=[(1.0, 0.5)])
        for graph in ([(2.0, 0.25), ('invalid', 1.0)], [(1.0,)]):
            with self.subTest(graph=graph):
                with self.assertRaises((TypeError, IndexError)):
                    wheel.lateral_slip_graph = graph
                self.assert_graph_equal(wheel.lateral_slip_graph, [(1.0, 0.5)])

    def test_lateral_slip_graph_vehicle_control_round_trip(self):
        wheel = carla.WheelPhysicsControl(
            axle_type=2, max_hand_brake_torque=4200.0,
            lateral_slip_graph=[(0.0, 1.0), (15.0, 0.25)])
        control = carla.VehiclePhysicsControl(wheels=[wheel])
        restored = control.wheels[0]
        self.assertEqual(restored, wheel)
        self.assertEqual(restored.axle_type, 2)
        self.assertAlmostEqual(restored.max_hand_brake_torque, 4200.0)
        self.assert_graph_equal(restored.lateral_slip_graph,
                                [(0.0, 1.0), (15.0, 0.25)])

    def test_lateral_slip_graph_accepts_existing_vector_wrapper(self):
        graph = carla.VectorOfVector2D()
        graph.append(carla.Vector2D(0.0, 1.0))
        wheel = carla.WheelPhysicsControl(lateral_slip_graph=graph)
        self.assert_graph_equal(wheel.lateral_slip_graph, [(0.0, 1.0)])


if __name__ == '__main__':
    unittest.main()
