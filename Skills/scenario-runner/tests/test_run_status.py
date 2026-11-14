"""Execute the runner wrappers with inert external runner and CARLA processes."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]
WRAPPERS = {
    "scenario": "run-scenario/scripts/run_scenario.sh",
    "osc1": "run-openscenario/scripts/run_openscenario.sh",
    "osc2": "run-openscenario/scripts/run_openscenario.sh",
    "route": "run-route-scenario/scripts/run_route.sh",
}
RUNNER = """import json, os, sys
from pathlib import Path
args = sys.argv[1:]
Path(os.environ['CHILD_LOG']).write_text(json.dumps(args), encoding='utf-8')
if os.environ.get('CHECK_OSC_PATH') == '1':
    value = args[args.index('--openscenario2') + 1]
    # This reproduces the external OSC2 preprocessor's root-prefix contract.
    actual = Path('{}/{}'.format(os.environ['SCENARIO_RUNNER_ROOT'], value))
    if not actual.is_file() or actual.resolve() != Path(os.environ['EXPECTED_OSC']).resolve():
        raise SystemExit(9)
mode = os.environ['RUNNER_MODE']
if mode == 'nonzero':
    print("Scenario 'Smoke' not supported ... Exiting")
    raise SystemExit(7)
if mode in ('complete', 'report_then_exception') and '--output' in args:
    print('Criteria Information')
    print('GLOBAL RESULT: ' + os.environ.get('VERDICT', 'SUCCESS'))
if mode in ('exception', 'report_then_exception'):
    print('Traceback (most recent call last):', file=sys.stderr)
    print('TypeError: inert runner construction failure', file=sys.stderr)
"""
CARLA = """import os
from pathlib import Path
from types import SimpleNamespace
def record(value):
    with Path(os.environ['RESET_LOG']).open('a', encoding='utf-8') as stream:
        stream.write(value + '\\n')
class World:
    def get_map(self): return SimpleNamespace(name='/Game/Carla/Maps/Town01')
    def get_settings(self): return SimpleNamespace(synchronous_mode=True, fixed_delta_seconds=0.05)
    def apply_settings(self, settings):
        assert settings.synchronous_mode is False
        assert settings.fixed_delta_seconds is None
        record('world')
class TrafficManager:
    def set_synchronous_mode(self, value):
        assert value is False
        record('traffic_manager')
class Client:
    def __init__(self, host, port): pass
    def set_timeout(self, timeout): pass
    def get_world(self): return World()
    def get_trafficmanager(self, port): return TrafficManager()
"""
FAILED_CD = """cd() {
    if [ "$1" = "${SCENARIO_RUNNER_ROOT}" ]; then return 1; fi
    builtin cd "$@"
}
source "$1" "${@:2}"
"""


def bash_executable():
    """Use native Git Bash on Windows, rather than the WSL launcher."""
    if os.name == "nt":
        return str(Path(shutil.which("git")).parents[1] / "bin/bash.exe")
    return shutil.which("bash")


class TestRunStatus(unittest.TestCase):
    """External process status, reports and paths define wrapper completion."""

    def setUp(self):
        # The resource spans the test method and is released by addCleanup.
        # pylint: disable-next=consider-using-with
        temporary = tempfile.TemporaryDirectory(prefix="carla runner ")
        self.addCleanup(temporary.cleanup)
        self.folder = Path(temporary.name).resolve()
        shadow = self.folder / "source"
        # Keep the real env scripts and their relative imports. Normalize only
        # fixture copies because a Windows Git checkout can contain CRLF.
        paths = list((ROOT / "Skills/scenario-runner").rglob("*.sh"))
        paths.append(ROOT / "Skills/_common/env_common.sh")
        for source in paths:
            target = shadow / source.relative_to(ROOT)
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(source.read_text(encoding="utf-8"), encoding="utf-8", newline="\n")
        self.checkout = self.folder / "scenario runner"
        examples = self.checkout / "srunner/examples"
        examples.mkdir(parents=True)
        (examples / "smoke.xml").write_text(
            '<scenarios><scenario name="Smoke" type="Smoke" town="Town01"/></scenarios>', encoding="utf-8")
        (self.checkout / "scenario_runner.py").write_text(RUNNER, encoding="utf-8")
        (self.checkout / "carla.py").write_text(CARLA, encoding="utf-8")
        self.agent = self.checkout / "npc_agent.py"
        self.agent.write_text("class NpcAgent: pass\n", encoding="utf-8")
        self.routes = self.folder / "routes with spaces.xml"
        self.routes.write_text("<routes/>", encoding="utf-8")
        self.osc = self.folder / "outside scenarios" / "sample with spaces.osc"
        self.osc.parent.mkdir()
        self.osc.write_text("inert OSC input", encoding="utf-8")
        self.child_log = self.folder / "child.json"
        self.reset_log = self.folder / "reset.txt"

    def run_wrapper(self, kind, mode="complete", verdict="SUCCESS", path=None, check_path=False):
        """Only replace the external process boundaries, retaining real Bash."""
        for artifact in (self.child_log, self.reset_log):
            artifact.unlink(missing_ok=True)
        environment = dict(os.environ)
        for key in ("OUTPUT", "PYTHONPATH", "CARLA_ROOT", "AGENT_CONFIG", "SCENARIO_RUNNER_ROOT"):
            environment.pop(key, None)
        environment.update(SCENARIO_RUNNER_ROOT=self.checkout.as_posix(), PYTHON=sys.executable,
                           CARLA_ROOT="", SYNC="1", RELOAD="0", MAX_WALL="0",
                           AGENT=self.agent.as_posix(), RUNNER_MODE=mode, VERDICT=verdict,
                           CHILD_LOG=str(self.child_log), RESET_LOG=str(self.reset_log),
                           CARLA_TOOLS_CONFIG=str(self.folder / "absent-config"),
                           EXPECTED_OSC=str(self.osc), CHECK_OSC_PATH=str(int(check_path)),
                           PYTHONIOENCODING="utf-8")
        script = self.folder / "source/Skills/scenario-runner" / WRAPPERS[kind]
        arguments = {"scenario": ["Smoke"], "osc1": ["run", path or self.osc.as_posix()],
                     "osc2": ["run2", path or self.osc.as_posix()],
                     "route": ["run", self.routes.as_posix(), "0"]}[kind]
        command = [bash_executable(), script.as_posix(), *arguments]
        if mode == "failed_cd":
            environment["LOG"] = (self.folder / "unrelated.log").as_posix()
            command = [bash_executable(), "-c", FAILED_CD, "carla-test", script.as_posix(), *arguments]
        return subprocess.run(command, cwd=self.folder,
                              env=environment, capture_output=True, text=True, timeout=30, check=False)

    def assert_cleanup(self):
        """Both asynchronous resets must survive every runner outcome."""
        self.assertEqual(self.reset_log.read_text(encoding="utf-8").splitlines(), ["world", "traffic_manager"])

    def test_completed_verdicts_always_have_a_report(self):
        """An unsuccessful criterion is still a completed execution."""
        cases = [(kind, "SUCCESS") for kind in WRAPPERS]
        cases += [("scenario", "TIMEOUT"), ("route", "FAILURE")]
        for kind, verdict in cases:
            with self.subTest(kind=kind, verdict=verdict):
                result = self.run_wrapper(kind, verdict=verdict)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertIn("Criteria Information", result.stdout)
                self.assertEqual(json.loads(self.child_log.read_text(encoding="utf-8")).count("--output"), 1)
                self.assert_cleanup()

    def test_zero_exit_without_report_is_failure(self):
        """A runner which never completed cannot report wrapper success."""
        for kind in WRAPPERS:
            with self.subTest(kind=kind):
                result = self.run_wrapper(kind, mode="no_report")
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assert_cleanup()

    def test_caught_exception_is_failure_even_after_an_earlier_report(self):
        """An earlier repetition's report cannot hide a later exception."""
        for kind in WRAPPERS:
            for mode in ("exception", "report_then_exception"):
                with self.subTest(kind=kind, mode=mode):
                    result = self.run_wrapper(kind, mode=mode)
                    self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                    self.assert_cleanup()

    def test_child_nonzero_status_is_preserved(self):
        """Diagnostic classification must preserve the actual failure code."""
        for kind in WRAPPERS:
            with self.subTest(kind=kind):
                result = self.run_wrapper(kind, mode="nonzero")
                self.assertEqual(result.returncode, 7, result.stdout + result.stderr)
                self.assert_cleanup()

    def test_osc2_paths_are_relative_to_the_runner_root(self):
        """The external preprocessor must open the intended file exactly once."""
        spellings = dict.fromkeys((str(self.osc), self.osc.as_posix(), os.path.relpath(self.osc, self.folder)))
        for path in spellings:
            with self.subTest(path=path):
                result = self.run_wrapper("osc2", path=path, check_path=True)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                arguments = json.loads(self.child_log.read_text(encoding="utf-8"))
                received = arguments[arguments.index("--openscenario2") + 1]
                self.assertFalse(os.path.isabs(received))
                self.assertNotIn("\r", received)
                self.assert_cleanup()

    def test_missing_osc2_file_stops_before_runner(self):
        """Input validation must finish before changing the simulator."""
        result = self.run_wrapper("osc2", path=str(self.folder / "absent.osc"))
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertFalse(self.child_log.exists())

    def test_failed_checkout_cd_preserves_inherited_log_file(self):
        """A failure before mktemp must never delete an inherited LOG path."""
        sentinel = self.folder / "unrelated.log"
        for kind in ("scenario", "osc1", "route"):
            with self.subTest(kind=kind):
                sentinel.write_text("unrelated user data", encoding="utf-8")
                result = self.run_wrapper(kind, mode="failed_cd")
                self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertFalse(self.child_log.exists())
                self.assertTrue(sentinel.is_file(), "Cleanup deleted the inherited LOG file")
                self.assertEqual(sentinel.read_text(encoding="utf-8"), "unrelated user data")


if __name__ == "__main__":
    unittest.main()
