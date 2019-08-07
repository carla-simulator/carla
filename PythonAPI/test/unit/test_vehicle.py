# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
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

        wheels = [carla.WheelPhysicsControl(tire_friction=2, damping_rate=0, max_steer_angle=30, radius=10),
                  carla.WheelPhysicsControl(tire_friction=3, damping_rate=1, max_steer_angle=40, radius=20),
                  carla.WheelPhysicsControl(tire_friction=4, damping_rate=2, max_steer_angle=50, radius=30),
                  carla.WheelPhysicsControl(tire_friction=5, damping_rate=3, max_steer_angle=60, radius=40)]

        pc = carla.VehiclePhysicsControl(
            torque_curve=torque_curve,
            max_rpm=5729,
            moi=1,
            damping_rate_full_throttle=0.15,
            damping_rate_zero_throttle_clutch_engaged=2,
            damping_rate_zero_throttle_clutch_disengaged=0.35,

            use_gear_autobox=1,
            gear_switch_time=0.5,
            clutch_strength=10,

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
        self.assertTrue(abs(pc.moi - 1) <= error)
        self.assertTrue(abs(pc.damping_rate_full_throttle - 0.15) <= error)
        self.assertTrue(abs(pc.damping_rate_zero_throttle_clutch_engaged - 2) <= error)
        self.assertTrue(abs(pc.damping_rate_zero_throttle_clutch_disengaged - 0.35) <= error)

        self.assertTrue(abs(pc.use_gear_autobox - 1) <= error)
        self.assertTrue(abs(pc.gear_switch_time - 0.5) <= error)
        self.assertTrue(abs(pc.clutch_strength - 10) <= error)

        self.assertTrue(abs(pc.mass - 5500) <= error)
        self.assertTrue(abs(pc.drag_coefficient - 0.3) <= error)

        self.assertTrue(abs(pc.center_of_mass.x - 0.5) <= error)
        self.assertTrue(abs(pc.center_of_mass.y - 1) <= error)
        self.assertTrue(abs(pc.center_of_mass.z - 1) <= error)

        for i in range(0, len(steering_curve)):
            self.assertTrue(abs(pc.steering_curve[i].x - steering_curve[i].x) <= error)
            self.assertTrue(abs(pc.steering_curve[i].y - steering_curve[i].y) <= error)

        for i in range(0, len(wheels)):
            self.assertTrue(abs(pc.wheels[i].tire_friction - wheels[i].tire_friction) <= error)
            self.assertTrue(abs(pc.wheels[i].damping_rate - wheels[i].damping_rate) <= error)
            self.assertTrue(abs(pc.wheels[i].max_steer_angle - wheels[i].max_steer_angle) <= error)
            self.assertTrue(abs(pc.wheels[i].radius - wheels[i].radius) <= error)

            self.assertTrue(abs(pc.wheels[i].position.x - wheels[i].position.x) <= error)
            self.assertTrue(abs(pc.wheels[i].position.y - wheels[i].position.y) <= error)
            self.assertTrue(abs(pc.wheels[i].position.z - wheels[i].position.z) <= error)