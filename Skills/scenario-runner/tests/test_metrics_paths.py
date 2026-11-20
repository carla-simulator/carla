"""Exercise recorder path handoff through the real metrics adapter."""

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
ADAPTER = ROOT / "Skills/scenario-runner/analyze-scenario-results/scripts/analyze_results.py"
MANAGER = """import argparse, json, os
from pathlib import Path
parser = argparse.ArgumentParser()
for flag in ('log', 'metric', 'host', 'port', 'criteria'):
    parser.add_argument('--' + flag)
args = parser.parse_args()
# The external manager prefixes its checkout root, including for absolute input.
recorder = Path('{}/{}'.format(os.environ['SCENARIO_RUNNER_ROOT'], args.log))
if not recorder.is_file():
    raise SystemExit(9)
print(json.dumps({'recorder': str(recorder.resolve()), 'criteria': args.criteria}))
raise SystemExit(int(os.environ.get('METRICS_EXIT_CODE', '0')))
"""


class TestMetricsPaths(unittest.TestCase):
    """Recorder paths must reach a separate manager process exactly once."""

    def setUp(self):
        # The resource spans the test method and is released by addCleanup.
        # pylint: disable-next=consider-using-with
        self.temporary = tempfile.TemporaryDirectory(prefix="carla metrics ")
        self.addCleanup(self.temporary.cleanup)
        self.folder = Path(self.temporary.name)
        self.checkout = self.folder / "scenario runner"
        self.checkout.mkdir()
        (self.checkout / "metrics_manager.py").write_text(MANAGER, encoding="utf-8")
        self.recordings = self.checkout / "recordings"
        self.recordings.mkdir()
        self.log = self.recordings / "Control Loss.log"
        self.log.write_bytes(b"inert recorder boundary")
        self.log.with_suffix(".json").write_text("{}", encoding="utf-8")

    def run_metrics(self, log, status=0):
        """Keep the external manager process and its documented path contract."""
        environment = dict(os.environ, SCENARIO_RUNNER_ROOT=str(self.checkout),
                           METRICS_EXIT_CODE=str(status), PYTHONIOENCODING="utf-8")
        return subprocess.run([sys.executable, str(ADAPTER), "metrics", "--metric", "metric.py",
                               "--log", str(log)], cwd=self.folder, env=environment,
                              capture_output=True, text=True, check=False, timeout=15)

    def check_recorder(self, result):
        """The existing input must be the input actually opened by the child."""
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        payload = next(json.loads(line) for line in result.stdout.splitlines() if line.startswith("{"))
        self.assertEqual(Path(payload["recorder"]), self.log.resolve())
        self.assertEqual(Path(payload["criteria"]), self.log.with_suffix(".json").resolve())

    def test_relative_recorder_is_relative_to_checkout(self):
        """The default recorder location is relative to ScenarioRunner."""
        self.check_recorder(self.run_metrics(Path("recordings") / self.log.name))

    def test_absolute_recorder_with_spaces(self):
        """An absolute recording path must reach the intended file."""
        self.check_recorder(self.run_metrics(self.log))

    def test_recorder_outside_checkout_on_same_volume(self):
        """The external manager must support a parent-relative recording."""
        self.log = self.folder / "external.log"
        self.log.write_bytes(b"inert recorder boundary")
        self.log.with_suffix(".json").write_text("{}", encoding="utf-8")
        self.check_recorder(self.run_metrics(self.log))

    def test_relative_recorder_can_resolve_from_calling_directory(self):
        """The caller's existing relative file must retain its location."""
        self.log = self.folder / "external.log"
        self.log.write_bytes(b"inert recorder boundary")
        self.log.with_suffix(".json").write_text("{}", encoding="utf-8")
        self.check_recorder(self.run_metrics(self.log.name))

    def test_missing_recorder_stops_before_manager(self):
        """A missing input must fail before launching the manager."""
        self.assertEqual(self.run_metrics("absent.log").returncode, 1)

    def test_manager_failure_status_reaches_caller(self):
        """The adapter must preserve the external manager's status."""
        self.assertEqual(self.run_metrics(self.log, status=7).returncode, 7)


if __name__ == "__main__":
    unittest.main()
