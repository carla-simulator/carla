import unittest

import numpy as np

from alpamayo_controller import TrajectoryTracker


class ControllerTest(unittest.TestCase):
    def setUp(self):
        self.tracker = TrajectoryTracker()

    def test_straight_accelerating_path(self):
        path = np.column_stack(
            (np.arange(0.5, 10.5, 0.5), np.zeros(20), np.zeros(20))
        )
        command = self.tracker.command(path, np.zeros(3), 0.0, 0.0)
        self.assertGreater(command.throttle, 0.0)
        self.assertEqual(command.brake, 0.0)
        self.assertAlmostEqual(command.steer, 0.0)

    def test_carla_right_turn_has_positive_steer(self):
        path = np.array([[x, 0.08 * x * x, 0.0] for x in np.linspace(0.5, 10, 20)])
        command = self.tracker.command(path, np.zeros(3), 0.0, 2.0)
        self.assertGreater(command.steer, 0.0)

    def test_brakes_when_above_predicted_speed(self):
        path = np.column_stack(
            (np.arange(0.1, 2.1, 0.1), np.zeros(20), np.zeros(20))
        )
        command = self.tracker.command(path, np.zeros(3), 0.0, 8.0)
        self.assertEqual(command.throttle, 0.0)
        self.assertGreater(command.brake, 0.0)

    def test_longitudinal_speed_uses_time_not_steering_lookahead(self):
        # Segment speeds rise from 1 to 10 m/s. Pure pursuit must look several
        # waypoints ahead for steering, while longitudinal control must use
        # the first 0.1-second trajectory interval.
        segment_speeds = np.arange(1.0, 11.0)
        x = np.cumsum(segment_speeds * 0.1)
        path = np.column_stack((x, np.zeros(10), np.zeros(10)))
        command = self.tracker.command(
            path,
            np.zeros(3),
            0.0,
            2.0,
            trajectory_time_index=0,
        )
        self.assertGreater(command.target_index, command.speed_index)
        self.assertEqual(command.speed_index, 0)
        self.assertAlmostEqual(command.target_speed_mps, 1.0)
        self.assertEqual(command.throttle, 0.0)
        self.assertGreater(command.brake, 0.0)

    def test_longitudinal_time_index_advances_with_control_step(self):
        segment_speeds = np.arange(1.0, 11.0)
        x = np.cumsum(segment_speeds * 0.1)
        path = np.column_stack((x, np.zeros(10), np.zeros(10)))
        command = self.tracker.command(
            path,
            np.zeros(3),
            0.0,
            0.0,
            trajectory_time_index=4,
        )
        self.assertEqual(command.speed_index, 4)
        self.assertAlmostEqual(command.target_speed_mps, 5.0)

    def test_integral_term_accumulates_speed_error(self):
        tracker = TrajectoryTracker(speed_kp=0.0, speed_ki=1.0, speed_kd=0.0)
        path = np.column_stack(
            (np.arange(0.2, 4.2, 0.2), np.zeros(20), np.zeros(20))
        )
        first = tracker.command(path, np.zeros(3), 0.0, 0.0)
        second = tracker.command(path, np.zeros(3), 0.0, 0.0)
        self.assertGreater(second.throttle, first.throttle)

    def test_stop_target_engages_stationary_hold(self):
        path = np.column_stack(
            (np.arange(0.01, 0.21, 0.01), np.zeros(20), np.zeros(20))
        )
        command = self.tracker.command(path, np.zeros(3), 0.0, 0.05)
        self.assertEqual(command.target_speed_mps, 0.0)
        self.assertEqual(command.throttle, 0.0)
        self.assertEqual(command.brake, 1.0)
        self.assertTrue(command.hold_active)

    def test_moving_stop_uses_minimum_brake_before_hold(self):
        tracker = TrajectoryTracker(minimum_stop_brake=0.4)
        path = np.column_stack(
            (np.arange(0.01, 0.21, 0.01), np.zeros(20), np.zeros(20))
        )
        command = tracker.command(path, np.zeros(3), 0.0, 0.5)
        self.assertEqual(command.throttle, 0.0)
        self.assertGreaterEqual(command.brake, 0.4)
        self.assertFalse(command.hold_active)

    def test_steering_gain_strengthens_lateral_response(self):
        path = np.array([[x, 0.03 * x * x, 0.0] for x in np.linspace(0.5, 8, 20)])
        baseline = TrajectoryTracker(
            steering_gain=1.0, max_steer_rate_per_s=100.0
        ).command(
            path, np.zeros(3), 0.0, 2.0
        )
        stronger = TrajectoryTracker(
            steering_gain=2.0, max_steer_rate_per_s=100.0
        ).command(
            path, np.zeros(3), 0.0, 2.0
        )
        self.assertAlmostEqual(stronger.steer, baseline.steer * 2.0)

    def test_steering_rate_is_limited(self):
        path = np.array([[x, x, 0.0] for x in np.linspace(0.5, 8, 20)])
        tracker = TrajectoryTracker(max_steer_rate_per_s=1.0)
        first = tracker.command(path, np.zeros(3), 0.0, 2.0, time_step=0.1)
        second = tracker.command(path, np.zeros(3), 0.0, 2.0, time_step=0.1)
        self.assertAlmostEqual(first.steer, 0.1)
        self.assertAlmostEqual(second.steer, 0.2)
        self.assertGreater(first.requested_steer, first.steer)

    def test_invalid_path_is_rejected(self):
        with self.assertRaises(ValueError):
            self.tracker.command(np.empty((0, 3)), np.zeros(3), 0.0, 0.0)


if __name__ == "__main__":
    unittest.main()
