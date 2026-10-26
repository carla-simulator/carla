# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise OBJ append text I/O in an actual non-UTF-8 Python process."""

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


OBJ_SCRIPT = Path(__file__).resolve().parents[3] / "Util/DockerUtils/dist/addOBJ.py"
TRIANGLE = b"v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n"


class TestObjTextEncoding(unittest.TestCase):
    """OBJ names retain their bytes regardless of the host text encoding."""

    def setUp(self):
        # TestCase cleanup also runs when setup or an assertion fails.
        temporary = tempfile.TemporaryDirectory(  # pylint: disable=consider-using-with
            prefix="carla-obj-encoding-")
        self.addCleanup(temporary.cleanup)
        self.directory = Path(temporary.name)
        self.environment = os.environ.copy()
        self.environment.update(PYTHONUTF8="0", PYTHONCOERCECLOCALE="0", LC_ALL="C")

    def append(self, source, destination=b"", material=None, utf8=False):
        """Run the actual CLI with controlled text bytes and locale settings."""
        source_path = self.directory / "source.obj"
        destination_path = self.directory / "destination.obj"
        source_path.write_bytes(source)
        destination_path.write_bytes(destination)
        command = [sys.executable, "-X", "utf8=" + str(int(utf8)), str(OBJ_SCRIPT),
                   str(destination_path), str(source_path)]
        if material is not None:
            command.append(material)
        result = subprocess.run(command, env=self.environment, capture_output=True,
                                check=False, timeout=20)
        self.assertEqual(result.returncode, 0, result.stderr.decode("utf-8", errors="replace"))
        return destination_path.read_bytes().replace(b"\r\n", b"\n")

    def test_utf8_object_name_in_source(self):
        """Blender-style UTF-8 object names survive a legacy host locale."""
        object_name = "o 植物\n".encode("utf-8")
        output = self.append(object_name + TRIANGLE)
        self.assertEqual(output, object_name + TRIANGLE)

    def test_utf8_existing_destination(self):
        """A pre-existing UTF-8 file is read successfully and retained."""
        destination = "o 植物\nv -1 -1 -1\n".encode("utf-8")
        output = self.append(TRIANGLE, destination)
        self.assertEqual(output, destination + TRIANGLE.replace(b"f 1 2 3", b"f 2 3 4"))

    def test_utf8_material_override(self):
        """Unicode material arguments are written as UTF-8 OBJ records."""
        output = self.append(b"usemtl old\n" + TRIANGLE, material="植物")
        self.assertEqual(output, "usemtl 植物\n".encode("utf-8") + TRIANGLE)

    def test_opaque_legacy_name_bytes(self):
        """Existing non-UTF-8 names remain accepted and retain their bytes."""
        records = b"o legacy-\xe9\ng opaque-\xff\n"
        self.assertEqual(self.append(records + TRIANGLE), records + TRIANGLE)

    def test_ascii_geometry_control(self):
        """ASCII geometry and ordinary material replacement are unchanged."""
        output = self.append(b"usemtl old\n" + TRIANGLE, material="road")
        self.assertEqual(output, b"usemtl road\n" + TRIANGLE)

    def test_utf8_mode_control(self):
        """The same UTF-8 source remains valid with UTF-8 mode enabled."""
        source = "o 植物\n".encode("utf-8") + TRIANGLE
        self.assertEqual(self.append(source, utf8=True), source)


if __name__ == "__main__":
    unittest.main()
