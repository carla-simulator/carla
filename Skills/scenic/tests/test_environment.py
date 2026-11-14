"""Check Scenic interpreter selection through the real installer and Bash scripts."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
import venv


ROOT = Path(__file__).resolve().parents[3]
SKILLS = ("run-scenic-scenario", "create-scenic-scenario")
CLIENT = """import os
from types import SimpleNamespace
__version__ = '1.0.0'
class Client:
    def __init__(self, *args): pass
    def set_timeout(self, value): pass
    def get_client_version(self): return __version__
    def get_server_version(self): return os.environ.get('TEST_SERVER_VERSION', __version__)
    def get_world(self):
        return SimpleNamespace(get_map=lambda: SimpleNamespace(name='/Game/Town01'),
                               get_settings=lambda: SimpleNamespace(synchronous_mode=False))
"""
SIMULATOR = """import os
import carla
if os.environ.get('TEST_BROKEN_INTERFACE'):
    raise ImportError('inert incompatible simulator boundary')
"""


class TestScenicEnvironment(unittest.TestCase):
    """Inert dependencies retain real Python imports and process boundaries."""

    @classmethod
    def setUpClass(cls):
        # This resource spans the class and is released by addClassCleanup.
        # pylint: disable-next=consider-using-with
        temporary = tempfile.TemporaryDirectory(prefix="carla scenic ")
        cls.addClassCleanup(temporary.cleanup)
        cls.folder = Path(temporary.name).resolve()
        environment = cls.folder / "selected interpreter"
        venv.EnvBuilder(with_pip=False).create(environment)
        cls.python = environment / ("Scripts/python.exe" if os.name == "nt" else "bin/python")
        config = subprocess.run(
            [str(cls.python), "-c", "import json,sysconfig;print(json.dumps(sysconfig.get_paths()))"],
            check=True, capture_output=True, text=True)
        paths = json.loads(config.stdout)
        cls.packages = Path(paths["purelib"])
        cls.packages.resolve().relative_to(cls.folder)
        cls.cli = Path(paths["scripts"]) / ("scenic.exe" if os.name == "nt" else "scenic")
        for package in ("scenic", "scenic/simulators", "scenic/simulators/carla", "carla"):
            directory = cls.packages / package
            directory.mkdir(parents=True, exist_ok=True)
            (directory / "__init__.py").write_text("", encoding="utf-8")
        (cls.packages / "scenic/simulators/carla/simulator.py").write_text(SIMULATOR, encoding="utf-8")
        for name in ("shapely", "trimesh", "scipy", "numpy", "pygame"):
            (cls.packages / (name + ".py")).write_text("", encoding="utf-8")
        for name, version in (("scenic", "3.1.1"), ("carla", "1.0.0")):
            directory = cls.packages / (name + "-" + version + ".dist-info")
            directory.mkdir()
            (directory / "METADATA").write_text(
                f"Metadata-Version: 2.1\nName: {name}\nVersion: {version}\n", encoding="utf-8")
        cls.shadow = cls.folder / "source"
        source_paths = ["Skills/_common/env_common.sh"]
        source_paths += [f"Skills/scenic/{skill}/scripts/{name}.sh"
                         for skill in SKILLS for name in ("env", "check_env")]
        for relative in source_paths:
            target = cls.shadow / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text((ROOT / relative).read_text(encoding="utf-8"), encoding="utf-8", newline="\n")
        if os.name == "nt":
            git = Path(shutil.which("git"))
            cls.bash = git.parent.parent / "bin/bash.exe"
        else:
            cls.bash = Path(shutil.which("bash"))

    def setUp(self):
        self.cli.write_text("inert CLI lookup boundary", encoding="utf-8")
        self.cli.chmod(0o755)
        (self.packages / "carla/__init__.py").write_text(CLIENT, encoding="utf-8")
        self.tables("1.0.0")
        self.environment = dict(os.environ, PYTHON=self.python.as_posix(),
                                CARLA_TOOLS_CONFIG=str(self.folder / "absent.env"),
                                CARLA_ROOT="", SCENARIO_RUNNER_ROOT="", SCENIC_BIN="",
                                SCENIC_ROOT=str(self.packages / "scenic"), SCENIC_EXAMPLES="",
                                PYTHONPATH="", PYTHONIOENCODING="utf-8")
        self.environment.pop("TEST_BROKEN_INTERFACE", None)
        self.environment.pop("TEST_SERVER_VERSION", None)

    def tables(self, version):
        """Set only the external package data used by the preflight."""
        (self.packages / "scenic/simulators/carla/_blueprintData.py").write_text(
            f"_IDS = {{{version!r}: {{'carModels': ['vehicle.test']}}}}\n", encoding="utf-8")
        cache = self.packages / "scenic/simulators/carla/__pycache__"
        cache.resolve().relative_to(self.folder)
        shutil.rmtree(cache, ignore_errors=True)

    def installer(self):
        """Run the production installer with a different selected interpreter."""
        return subprocess.run(
            [sys.executable, str(ROOT / "Skills/setup/install-scenic/scripts/install_scenic.py"),
             "--python", str(self.python), "verify"], env=self.environment,
            capture_output=True, text=True, check=False, timeout=30)

    def shell(self, skill, check=True):
        """Execute actual scripts with LF checkout bytes for Bash portability."""
        script = self.shadow / f"Skills/scenic/{skill}/scripts/{'check_env' if check else 'env'}.sh"
        command = [str(self.bash), script.as_posix()] if check else [
            str(self.bash), "-c", 'source "$1"; printf "__CLI__=%s\\n" "$SCENIC_BIN"', "test", script.as_posix()]
        return subprocess.run(command, env=self.environment, cwd=self.folder, check=False,
                              capture_output=True, text=True, timeout=30)

    def test_verify_accepts_regular_client_package(self):
        """A normal package initializer is a valid CARLA module file."""
        result = self.installer()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn(str(self.cli), result.stdout)

    def test_verify_warns_when_exact_table_is_absent(self):
        """A usable interface can select recorded fallback blueprint data."""
        self.tables("0.10.0")
        result = self.installer()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("WARN", result.stdout)

    def test_verify_rejects_namespace_shadowing(self):
        """Distribution metadata cannot make an empty namespace usable."""
        if os.name == "nt":
            legacy_cli = self.cli.with_name("scenic")
            legacy_cli.write_text("inert legacy CLI name", encoding="utf-8")
            self.addCleanup(legacy_cli.unlink)
        (self.packages / "carla/__init__.py").unlink()
        cache = self.packages / "carla/__pycache__"
        cache.resolve().relative_to(self.folder)
        shutil.rmtree(cache, ignore_errors=True)
        self.assertNotEqual(self.installer().returncode, 0)

    def test_verify_rejects_foreign_cli(self):
        """A launcher found on PATH must belong to the selected interpreter."""
        self.cli.unlink()
        foreign = self.folder / "foreign scripts"
        foreign.mkdir(exist_ok=True)
        candidate = foreign / self.cli.name
        candidate.write_text("inert foreign CLI", encoding="utf-8")
        candidate.chmod(0o755)
        self.environment["PATH"] = str(foreign) + os.pathsep + os.environ["PATH"]
        result = self.installer()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("different environment", result.stdout)

    def test_verify_rejects_incompatible_simulator(self):
        """An import failure remains a hard verification error."""
        self.environment["TEST_BROKEN_INTERFACE"] = "1"
        result = self.installer()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("simulator import failed", result.stdout)

    def test_runtime_derives_cli_from_selected_interpreter(self):
        """Both runtime skills find the selected environment without PATH."""
        for skill in SKILLS:
            with self.subTest(skill=skill):
                result = self.shell(skill, check=False)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                selected = next(line.removeprefix("__CLI__=") for line in result.stdout.splitlines()
                                if line.startswith("__CLI__="))
                self.assertEqual(Path(selected).resolve(), self.cli.resolve())

    def test_preflight_allows_fallback_with_usable_interface(self):
        """Both preflights accept matching clients with advisory table gaps."""
        self.tables("0.10.0")
        for skill in SKILLS:
            with self.subTest(skill=skill):
                result = self.shell(skill)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertIn("WARN", result.stdout)

    def test_runtime_derives_root_for_pip_only_installation(self):
        """Native Python output must remain usable by both Bash skills."""
        self.environment["SCENIC_ROOT"] = ""
        for skill in SKILLS:
            with self.subTest(skill=skill):
                script = self.shadow / f"Skills/scenic/{skill}/scripts/env.sh"
                result = subprocess.run(
                    [str(self.bash), "-c", 'source "$1"; printf "__ROOT__=%s\\n" "$SCENIC_ROOT"',
                     "test", script.as_posix()], env=self.environment, cwd=self.folder,
                    capture_output=True, text=True, check=False, timeout=30)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                selected = next(line.removeprefix("__ROOT__=") for line in result.stdout.splitlines()
                                if line.startswith("__ROOT__="))
                self.assertEqual(Path(selected).resolve(), (self.packages / "scenic").resolve())
                self.assertNotIn("\\", selected)

    def test_preflight_retains_interface_and_server_failure_gates(self):
        """An advisory table check cannot mask interface or server failures."""
        for variable in ("TEST_BROKEN_INTERFACE", "TEST_SERVER_VERSION"):
            self.environment[variable] = "incompatible"
            for skill in SKILLS:
                with self.subTest(skill=skill, failure=variable):
                    result = self.shell(skill)
                    self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
            self.environment.pop(variable)


if __name__ == "__main__":
    unittest.main()
