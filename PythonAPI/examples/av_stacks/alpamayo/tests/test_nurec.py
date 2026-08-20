import unittest

from alpamayo_nurec import NurecEpisodeComplete, _tick_and_capture


class _FinishedScenario:
    def __init__(self):
        self.running = True

    def tick(self):
        self.running = False

    def is_running(self):
        return self.running


class _NoFrames:
    def has_complete_batch_after(self, frame):
        return False


class NurecEpisodeTest(unittest.TestCase):
    def test_finite_timeline_ends_without_sensor_timeout(self):
        with self.assertRaises(NurecEpisodeComplete):
            _tick_and_capture(
                _FinishedScenario(),
                vehicle=None,
                sensors=_NoFrames(),
                history=None,
                previous_frame=10,
                timeout=1.0,
            )


if __name__ == "__main__":
    unittest.main()
