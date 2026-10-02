import unittest

from alpamayo_display import SimulationVideoClock


class SimulationVideoClockTest(unittest.TestCase):
    def test_twenty_fps_duplicates_each_ten_hz_state(self):
        clock = SimulationVideoClock(20.0)
        self.assertEqual([clock.advance(0.1) for _ in range(4)], [2, 2, 2, 2])

    def test_fractional_frame_credit_is_preserved(self):
        clock = SimulationVideoClock(24.0)
        counts = [clock.advance(0.1) for _ in range(5)]
        self.assertEqual(counts, [2, 2, 3, 2, 3])
        self.assertEqual(sum(counts), 12)

    def test_zero_simulation_time_adds_no_pause_frames(self):
        clock = SimulationVideoClock(20.0)
        self.assertEqual(clock.advance(0.1), 2)
        self.assertEqual(clock.advance(0.0), 0)
        self.assertEqual(clock.advance(0.0), 0)


if __name__ == "__main__":
    unittest.main()
