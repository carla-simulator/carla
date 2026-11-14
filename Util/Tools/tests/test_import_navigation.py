# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise navigation process failures through the actual importer and Bash script."""

import contextlib
import importlib.util
import json
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


@unittest.skipUnless(os.name == "posix" and shutil.which("bash"), "Requires POSIX Bash")
class TestImportNavigation(unittest.TestCase):
    """Use inert converter and Recast processes with real importer file operations."""

    def setUp(self):
        """Give each case private native boundaries, assets and command search paths."""
        self.folder = Path(tempfile.mkdtemp(prefix="carla navigation "))
        self.addCleanup(shutil.rmtree, self.folder)
        self.tools = self.folder / "Util/DockerUtils/dist"
        self.tools.mkdir(parents=True)
        self.assets = self.folder / "assets"
        self.assets.mkdir()
        commands = self.folder / "commands"
        commands.mkdir()
        for command in ("python3", "bash", "chmod", "cp", "rm"):
            (commands / command).symlink_to(shutil.which(command))
        self.environment = os.environ.copy()
        self.environment.pop("PYTHON", None)
        self.environment["PATH"] = str(commands)
        self._write("build.sh", (ROOT / "Util/DockerUtils/dist/build.sh").read_text(encoding="utf-8"))
        self._write("addOBJ.py", (ROOT / "Util/DockerUtils/dist/addOBJ.py").read_text(encoding="utf-8"))
        self._write("Map.obj", "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n")
        self._write("geometry.obj", (self.tools / "Map.obj").read_text(encoding="utf-8"))
        self._executable("FBX2OBJ", '#!/bin/bash\ncp geometry.obj "$2"\n')
        self._executable("RecastBuilder", '#!/bin/bash\nprintf called > recast-called\ncp "$1" "${1%.obj}.bin"\n')
        spec = importlib.util.spec_from_file_location("navigation_importer", ROOT / "Util/Tools/Import.py")
        self.importer = importlib.util.module_from_spec(spec)
        with mock.patch.dict(sys.modules, {"carla": types.ModuleType("carla")}):
            spec.loader.exec_module(self.importer)
        self.importer.CARLA_ROOT_PATH = str(self.folder)
        (self.assets / "Map.fbx").write_text("inert FBX", encoding="utf-8")
        self.maps = [{"name": "Map", "source": "Map.fbx", "xodr": ""}]
        self.destination = self.folder / "Unreal/CarlaUnreal/Content/Example/Maps/Map/Nav/Map.bin"

    def _write(self, name, text):
        """Write a native boundary input in the temporary tool directory."""
        path = self.tools / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
        return path

    def _executable(self, name, text):
        """Create a process boundary that runs independently of the importer."""
        path = self._write(name, text)
        path.chmod(0o755)
        return path

    def _run_build(self):
        """Execute the complete production Bash script with a private PATH."""
        return subprocess.run(["/bin/bash", str(self.tools / "build.sh"), "Map", "Roads"],
                              cwd=self.tools, env=self.environment, check=False,
                              capture_output=True, text=True, timeout=10)

    def _crosswalk_helper(self, failure=None):
        """Make crosswalk extraction either write a real OBJ or exit with its status."""
        self._write("Roads.xodr", "inert OpenDRIVE")
        if failure is not None:
            code = f"raise SystemExit({failure})\n"
        else:
            code = ("from pathlib import Path\n"
                    "Path('crosswalks.obj').write_text('v 0 0 1\\nv 1 0 1\\nv 0 1 1\\nf 1 2 3\\n')\n")
        self._write("get_xodr_crosswalks.py", code)

    def _import_package(self):
        """Run the actual package driver while isolating Unreal and TM boundaries."""
        descriptor = self.assets / "Example.json"
        descriptor.write_text(json.dumps({"maps": self.maps, "props": []}), encoding="utf-8")
        with contextlib.ExitStack() as stack:
            stack.enter_context(mock.patch.dict(os.environ, self.environment, clear=True))
            for name in ("import_assets", "move_assets_commandlet", "prepare_maps_commandlet_for_cooking",
                         "load_asset_materials_commandlet", "build_binary_for_tm"):
                stack.enter_context(mock.patch.object(self.importer, name))
            self.importer.import_assets_from_json_list([(str(self.assets), descriptor.name)], 1)

    def _stage_failed_crosswalk(self):
        """Preserve a stale binary while making an actual child process exit nine."""
        self._crosswalk_helper(failure=9)
        (self.assets / "Map.xodr").write_text("inert OpenDRIVE", encoding="utf-8")
        self.maps[0]["xodr"] = "Map.xodr"
        (self.folder / "commands/python").symlink_to(sys.executable)
        self._write("Map.bin", "stale navigation")

    def test_python3_default_without_python_alias(self):
        """The default interpreter extracts and merges crosswalks without a python alias."""
        self.assertIsNone(shutil.which("python", path=self.environment["PATH"]))
        self._crosswalk_helper()
        result = self._run_build()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("f 4 5 6", (self.tools / "Map.bin").read_text(encoding="utf-8"))

    def test_quoted_interpreter_override(self):
        """An executable path with spaces is used for both Python helpers."""
        self._crosswalk_helper()
        interpreter = self._executable("selected interpreter/python executable",
                                      '#!/bin/bash\nprintf selected >> interpreter-used\n'
                                      'exec "' + sys.executable + '" "$@"\n')
        self.environment["PYTHON"] = str(interpreter)
        result = self._run_build()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertTrue((self.tools / "interpreter-used").exists(), result.stderr)
        self.assertEqual((self.tools / "interpreter-used").read_text(encoding="utf-8"), "selectedselected")
        self.assertIn("f 4 5 6", (self.tools / "Map.bin").read_text(encoding="utf-8"))

    def test_importer_interpreter_handoff(self):
        """The navigation child receives the interpreter used to import CARLA."""
        self._crosswalk_helper()
        (self.assets / "Map.xodr").write_text("inert OpenDRIVE", encoding="utf-8")
        self.maps[0]["xodr"] = "Map.xodr"
        interpreter = self._executable("python environment/python executable",
                                       '#!/bin/bash\nprintf selected >> interpreter-used\n'
                                       'exec "' + sys.executable + '" "$@"\n')
        (self.folder / "commands/python").symlink_to(sys.executable)
        with mock.patch.dict(os.environ, self.environment, clear=True), \
                mock.patch.object(self.importer.sys, "executable", str(interpreter)):
            self.importer.build_binary_for_navigation("Example", str(self.assets), self.maps)
        self.assertTrue((self.tools / "interpreter-used").exists())
        self.assertEqual((self.tools / "interpreter-used").read_text(encoding="utf-8"), "selectedselected")
        self.assertIn("f 4 5 6", self.destination.read_text(encoding="utf-8"))

    def test_converter_failure_stops_stale_obj(self):
        """Converter exit seven prevents Recast from consuming the existing OBJ."""
        self._write("Map.fbx", "inert FBX")
        self._executable("FBX2OBJ", "#!/bin/bash\nexit 7\n")
        result = self._run_build()
        self.assertEqual(result.returncode, 7, result.stderr)
        self.assertFalse((self.tools / "recast-called").exists())

    def test_missing_converter_does_not_use_stale_obj(self):
        """FBX input requires its converter before Recast can consume an old OBJ."""
        self._write("Map.fbx", "inert FBX")
        (self.tools / "FBX2OBJ").unlink()
        result = self._run_build()
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse((self.tools / "recast-called").exists())

    def test_missing_recast_fails(self):
        """An available OBJ cannot produce a successful build without Recast."""
        (self.tools / "RecastBuilder").unlink()
        self.assertNotEqual(self._run_build().returncode, 0)

    def test_crosswalk_failure_stops_recast(self):
        """Crosswalk exit nine reaches the shell caller before Recast executes."""
        self._crosswalk_helper(failure=9)
        (self.folder / "commands/python").symlink_to(sys.executable)
        result = self._run_build()
        self.assertEqual(result.returncode, 9, result.stderr)
        self.assertFalse((self.tools / "recast-called").exists())

    def test_recast_failure_status(self):
        """A Recast failure retains its existing exit status."""
        self._executable("RecastBuilder", "#!/bin/bash\nexit 11\n")
        self.assertEqual(self._run_build().returncode, 11)

    def test_geometry_only_input(self):
        """Navigation generation still accepts geometry without OpenDRIVE."""
        result = self._run_build()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.tools / "Map.bin").read_bytes(), (self.tools / "Map.obj").read_bytes())

    def test_navigation_failure_does_not_publish_stale_binary(self):
        """The actual failed navigation child cannot overwrite an installed binary."""
        self._stage_failed_crosswalk()
        self.destination.parent.mkdir(parents=True)
        self.destination.write_text("installed navigation", encoding="utf-8")
        with mock.patch.dict(os.environ, self.environment, clear=True), \
                self.assertRaises(subprocess.CalledProcessError) as caught:
            self.importer.build_binary_for_navigation("Example", str(self.assets), self.maps)
        self.assertEqual(caught.exception.returncode, 9)
        self.assertEqual(self.destination.read_text(encoding="utf-8"), "installed navigation")

    def test_navigation_failure_reaches_package_caller(self):
        """The actual worker's failed child reaches the package import caller."""
        self._stage_failed_crosswalk()
        with self.assertRaises(subprocess.CalledProcessError) as caught:
            self._import_package()
        self.assertEqual(caught.exception.returncode, 9)
        self.assertFalse(self.destination.exists())

    def test_staging_failure_reaches_package_caller(self):
        """An actual worker filesystem error cannot disappear in a joined thread."""
        shutil.rmtree(self.tools)
        with self.assertRaises(OSError):
            self._import_package()
        self.assertFalse(self.destination.exists())

    def test_successful_worker_publishes_before_return(self):
        """A successful package import waits for its generated navigation binary."""
        self._import_package()
        self.assertEqual(self.destination.read_bytes(), (self.tools / "Map.obj").read_bytes())
        self.assertFalse((self.tools / "Map.bin").exists())

    def test_alternate_opendrive_filename_with_spaces(self):
        """The caller passes the alternate OpenDRIVE stem without losing spaces."""
        self._crosswalk_helper()
        (self.assets / "Alternate Roads.xodr").write_text("inert OpenDRIVE", encoding="utf-8")
        self.maps[0]["xodr"] = "Alternate Roads.xodr"
        with mock.patch.dict(os.environ, self.environment, clear=True):
            self.importer.build_binary_for_navigation("Example", str(self.assets), self.maps)
        self.assertIn("f 4 5 6", self.destination.read_text(encoding="utf-8"))

    def test_missing_output_cannot_publish_stale_binary(self):
        """A successful process without a fresh output cannot publish an older binary."""
        self._write("Map.bin", "stale navigation")
        self._executable("RecastBuilder", "#!/bin/bash\nexit 0\n")
        with self.assertRaises(FileNotFoundError):
            self._import_package()
        self.assertFalse(self.destination.exists())
        self.assertFalse((self.tools / "Map.bin").exists())


if __name__ == "__main__":
    unittest.main()
