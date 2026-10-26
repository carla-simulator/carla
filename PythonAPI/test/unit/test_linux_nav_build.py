# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.
"""Execute the real Linux navigation shell with inert native executables."""
import importlib.util
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import types
import unittest
from unittest import mock


ROOT = Path(os.environ.get("CARLA_NAV_TEST_ROOT", Path(__file__).resolve().parents[3]))
SHELL = ROOT / "Util/DockerUtils/dist/build.sh"


@unittest.skipUnless(os.name == "posix" and shutil.which("bash"), "Requires Linux/POSIX Bash")
class TestLinuxNavBuild(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="carla nav shell ")
        self.addCleanup(self.temporary.cleanup)
        self.folder = Path(self.temporary.name)
        self.environment = os.environ.copy()
        self.environment.pop("PYTHON", None)
        # A private PATH also exercises hosts that have python-is-python3 installed.
        commands = self.folder / "commands"
        commands.mkdir()
        for command in ("python3", "chmod", "cp", "rm"):
            (commands / command).symlink_to(shutil.which(command))
        self.environment["PATH"] = str(commands)
        self.write("Map.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n")
        self.executable("RecastBuilder", '#!/bin/bash\nprintf called > recast-called\ncp "$1" "${1%.obj}.bin"\n')
        shutil.copy2(ROOT / "Util/DockerUtils/dist/addOBJ.py", self.folder / "addOBJ.py")

    def write(self, name, data):
        path = self.folder / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(data, encoding="utf-8")
        return path

    def executable(self, name, data):
        path = self.write(name, data)
        path.chmod(0o755)
        return path

    def run_build(self):
        return subprocess.run(["/bin/bash", str(SHELL), "Map", "Roads"],
                              cwd=self.folder, env=self.environment,
                              capture_output=True, text=True, timeout=10)

    def crosswalk_helper(self, failure=None):
        self.write("Roads.xodr", "unused XML boundary fixture")
        if failure:
            code = "raise SystemExit(%d)\n" % failure
        else:
            code = ("from pathlib import Path\n"
                    "Path('crosswalks.obj').write_text('v 0 0 1\\nv 1 0 1\\nv 0 1 1\\nf 1 2 3\\n')\n")
        self.write("get_xodr_crosswalks.py", code)

    def test_python3_default_runs_helpers_without_python_alias(self):
        self.assertIsNone(shutil.which("python", path=self.environment["PATH"]))
        self.crosswalk_helper()
        result = self.run_build()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("f 4 5 6", (self.folder / "Map.bin").read_text())

    def test_explicit_interpreter_path_with_spaces_is_honored(self):
        self.crosswalk_helper()
        interpreter = self.executable(
            "selected interpreter/python executable",
            '#!/bin/bash\nprintf selected >> "' + str(self.folder / "interpreter-used") + '"\n'
            'exec "' + sys.executable + '" "$@"\n')
        self.environment["PYTHON"] = str(interpreter)
        result = self.run_build()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertTrue((self.folder / "interpreter-used").exists(), result.stderr)
        self.assertEqual((self.folder / "interpreter-used").read_text(), "selectedselected")
        self.assertIn("f 4 5 6", (self.folder / "Map.bin").read_text())

    def test_import_driver_passes_its_current_python(self):
        spec = importlib.util.spec_from_file_location("nav_shell_importer", ROOT / "Util/Tools/Import.py")
        importer = importlib.util.module_from_spec(spec)
        with mock.patch.dict(sys.modules, {"carla": types.ModuleType("carla")}):
            spec.loader.exec_module(importer)
        self.write("assets/Map.fbx", "inert FBX")
        tools = self.folder / "Util/DockerUtils/dist"
        tools.mkdir(parents=True)
        calls = []
        def build(command, **kwargs):
            if command == ["chmod +x build.sh"]:
                return 0
            calls.append((command, kwargs))
            (tools / "Map.bin").write_bytes(b"inert native output")
            return 0
        with mock.patch.object(importer, "CARLA_ROOT_PATH", str(self.folder)), \
                mock.patch.object(importer.subprocess, "call", side_effect=build), \
                mock.patch.object(importer.subprocess, "check_call", side_effect=build):
            importer.build_binary_for_navigation("Example", str(self.folder / "assets"),
                                                 [{"name": "Map", "source": "Map.fbx", "xodr": ""}])
        self.assertEqual(calls[0][1].get("env", {}).get("PYTHON"), sys.executable)

    def test_converter_failure_does_not_build_from_stale_obj(self):
        self.write("Map.fbx", "inert FBX")
        self.executable("FBX2OBJ", "#!/bin/bash\nexit 7\n")
        result = self.run_build()
        self.assertEqual(result.returncode, 7, result.stderr)
        self.assertFalse((self.folder / "recast-called").exists())

    def test_crosswalk_failure_stops_before_recast(self):
        self.crosswalk_helper(failure=9)
        alias = self.executable("aliases/python", '#!/bin/bash\nexec "' + sys.executable + '" "$@"\n')
        self.environment["PATH"] = str(alias.parent) + ":" + self.environment["PATH"]
        result = self.run_build()
        self.assertEqual(result.returncode, 9, result.stderr)
        self.assertFalse((self.folder / "recast-called").exists())

    def test_existing_recast_failure_status_is_preserved(self):
        self.executable("RecastBuilder", "#!/bin/bash\nexit 11\n")
        self.assertEqual(self.run_build().returncode, 11)

    def test_geometry_only_without_xodr_still_builds(self):
        result = self.run_build()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.folder / "Map.bin").read_bytes(), (self.folder / "Map.obj").read_bytes())


if __name__ == "__main__":
    unittest.main()
