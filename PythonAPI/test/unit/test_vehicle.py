# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

import unittest


class TestVehicleControl(unittest.TestCase):
    def test_default_values(self):
        c = carla.VehicleControl()
        self.assertEqual(c.throttle, 0.0)
        self.assertEqual(c.steer, 0.0)
        self.assertEqual(c.brake, 0.0)
        self.assertEqual(c.hand_brake, False)
        self.assertEqual(c.reverse, False)
        c = carla.VehicleControl(1.0, 2.0, 3.0, True, True)
        self.assertEqual(c.throttle, 1.0)
        self.assertEqual(c.steer, 2.0)
        self.assertEqual(c.brake, 3.0)
        self.assertEqual(c.hand_brake, True)
        self.assertEqual(c.reverse, True)

    def test_named_args(self):
        c = carla.VehicleControl(
            throttle=1.0,
            steer=2.0,
            brake=3.0,
            hand_brake=True,
            reverse=True)
        self.assertEqual(c.throttle, 1.0)
        self.assertEqual(c.steer, 2.0)
        self.assertEqual(c.brake, 3.0)
        self.assertEqual(c.hand_brake, True)
        self.assertEqual(c.reverse, True)


class TestVehiclePhysicsControl(unittest.TestCase):
    def test_named_args(self):

        torque_curve = [[0, 400],
                        [24, 56],
                        [24, 56],
                        [1315.47, 654.445],
                        [5729, 400]]

        steering_curve = [carla.Vector2D(x=0, y=1),
                          carla.Vector2D(x=20.0, y=0.9),
                          carla.Vector2D(x=63.0868, y=0.703473),
                          carla.Vector2D(x=119.12, y=0.573047)]

        wheels = [carla.WheelPhysicsControl(cornering_stiffness=1000, max_steer_angle=30, wheel_radius=10,
                                             location=carla.Location(x=1, y=1, z=0)),
                  carla.WheelPhysicsControl(cornering_stiffness=1100, max_steer_angle=40, wheel_radius=20,
                                             location=carla.Location(x=1, y=-1, z=0)),
                  carla.WheelPhysicsControl(cornering_stiffness=1200, max_steer_angle=50, wheel_radius=30,
                                             location=carla.Location(x=-1, y=1, z=0)),
                  carla.WheelPhysicsControl(cornering_stiffness=1300, max_steer_angle=60, wheel_radius=40,
                                             location=carla.Location(x=-1, y=-1, z=0))]

        pc = carla.VehiclePhysicsControl(
            torque_curve=torque_curve,
            max_rpm=5729,
            rev_up_moi=1,

            use_automatic_gears=1,
            gear_change_time=0.5,

            mass=5500,
            drag_coefficient=0.3,

            center_of_mass=carla.Vector3D(x=0.5, y=1, z=1),
            steering_curve=steering_curve,
            wheels=wheels)

        error = .001
        for i in range(0, len(torque_curve)):
            self.assertTrue(abs(pc.torque_curve[i].x - torque_curve[i][0]) <= error)
            self.assertTrue(abs(pc.torque_curve[i].y - torque_curve[i][1]) <= error)

        self.assertTrue(abs(pc.max_rpm - 5729) <= error)
        self.assertTrue(abs(pc.rev_up_moi - 1) <= error)

        self.assertTrue(abs(pc.use_automatic_gears - 1) <= error)
        self.assertTrue(abs(pc.gear_change_time - 0.5) <= error)

        self.assertTrue(abs(pc.mass - 5500) <= error)
        self.assertTrue(abs(pc.drag_coefficient - 0.3) <= error)

        self.assertTrue(abs(pc.center_of_mass.x - 0.5) <= error)
        self.assertTrue(abs(pc.center_of_mass.y - 1) <= error)
        self.assertTrue(abs(pc.center_of_mass.z - 1) <= error)

        for i in range(0, len(steering_curve)):
            self.assertTrue(abs(pc.steering_curve[i].x - steering_curve[i].x) <= error)
            self.assertTrue(abs(pc.steering_curve[i].y - steering_curve[i].y) <= error)

        for i in range(0, len(wheels)):
            self.assertTrue(abs(pc.wheels[i].cornering_stiffness - wheels[i].cornering_stiffness) <= error)
            self.assertTrue(abs(pc.wheels[i].max_steer_angle - wheels[i].max_steer_angle) <= error)
            self.assertTrue(abs(pc.wheels[i].wheel_radius - wheels[i].wheel_radius) <= error)

            self.assertTrue(abs(pc.wheels[i].location.x - wheels[i].location.x) <= error)
            self.assertTrue(abs(pc.wheels[i].location.y - wheels[i].location.y) <= error)
            self.assertTrue(abs(pc.wheels[i].location.z - wheels[i].location.z) <= error)


class TestVehicleGearRatios(unittest.TestCase):
    def assert_ratios_equal(self, actual, expected):
        self.assertIsInstance(actual, list)
        self.assertEqual(len(actual), len(expected))
        for actual_ratio, expected_ratio in zip(actual, expected):
            self.assertAlmostEqual(actual_ratio, expected_ratio, places=6)

    def test_default_ratios_are_readable(self):
        control = carla.VehiclePhysicsControl()
        for name in ("forward_gear_ratios", "reverse_gear_ratios"):
            with self.subTest(property=name):
                ratios = getattr(control, name)
                self.assertIsInstance(ratios, list)
                self.assertGreater(len(ratios), 0)
                self.assertTrue(all(isinstance(ratio, float) for ratio in ratios))

    def test_ratio_setters_accept_numbers_and_empty_lists(self):
        control = carla.VehiclePhysicsControl()
        for name in ("forward_gear_ratios", "reverse_gear_ratios"):
            for ratios in ([3, 2.5, 1], [], [1.25]):
                with self.subTest(property=name, ratios=ratios):
                    setattr(control, name, ratios)
                    self.assert_ratios_equal(getattr(control, name), ratios)

    def test_constructor_accepts_ratio_lists(self):
        for forward, reverse in (([3, 2.5, 1], [2, 1.25]), ([], [])):
            with self.subTest(forward=forward, reverse=reverse):
                control = carla.VehiclePhysicsControl(
                    forward_gear_ratios=forward,
                    reverse_gear_ratios=reverse)
                self.assert_ratios_equal(control.forward_gear_ratios, forward)
                self.assert_ratios_equal(control.reverse_gear_ratios, reverse)

    def test_invalid_ratio_preserves_previous_values(self):
        control = carla.VehiclePhysicsControl()
        for name in ("forward_gear_ratios", "reverse_gear_ratios"):
            with self.subTest(property=name):
                setattr(control, name, [3, 2.5, 1])
                with self.assertRaises(TypeError):
                    setattr(control, name, [4, object(), 2])
                self.assert_ratios_equal(getattr(control, name), [3, 2.5, 1])

    def test_constructor_rejects_invalid_ratios(self):
        for name in ("forward_gear_ratios", "reverse_gear_ratios"):
            with self.subTest(property=name):
                with self.assertRaises(TypeError):
                    carla.VehiclePhysicsControl(**{name: [1, object()]})
