# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Verify that the installed Boost used to build carla.libcarla is 1.89.0.

Guards against silent cache hits from the old Boost 1.84.0 install directory
that would otherwise pass the NumPy 2 test suite while still linking against
the pre-upgrade Boost.
"""

import os
import re
import subprocess
import sys
import unittest

EXPECTED_BOOST_VERSION = "1.89.0"
EXPECTED_BOOST_DIR = "boost-{}-".format(EXPECTED_BOOST_VERSION)


def _carla_so_path():
    """Return the absolute path of the compiled carla .so/.pyd extension."""
    import carla
    carla_pkg_dir = os.path.dirname(carla.__file__)
    # The compiled extension sits next to __init__.py, named libcarla*.so
    for name in os.listdir(carla_pkg_dir):
        if name.startswith("libcarla") and (name.endswith(".so") or name.endswith(".pyd")):
            return os.path.join(carla_pkg_dir, name)
    return None


class TestBoostInstallVersion(unittest.TestCase):
    """Static check: Setup.sh must pin Boost 1.89.0."""

    def test_setup_sh_pins_correct_version(self):
        """BOOST_VERSION in Setup.sh must be 1.89.0."""
        repo_root = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "..", "..", ".."))
        setup_sh = os.path.join(repo_root, "Util", "BuildTools", "Setup.sh")
        if not os.path.exists(setup_sh):
            self.skipTest("Setup.sh not found at {}".format(setup_sh))
        with open(setup_sh, "r", encoding="utf-8") as fh:
            content = fh.read()
        match = re.search(r"^BOOST_VERSION=(.+)$", content, re.MULTILINE)
        self.assertIsNotNone(match, "BOOST_VERSION not found in Setup.sh")
        self.assertEqual(
            match.group(1).strip(),
            EXPECTED_BOOST_VERSION,
            "Setup.sh pins Boost {} but expected {}".format(
                match.group(1).strip(), EXPECTED_BOOST_VERSION),
        )

    def test_setup_sh_has_no_patch_call(self):
        """Setup.sh must not contain the now-removed patch invocation."""
        repo_root = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "..", "..", ".."))
        setup_sh = os.path.join(repo_root, "Util", "BuildTools", "Setup.sh")
        if not os.path.exists(setup_sh):
            self.skipTest("Setup.sh not found at {}".format(setup_sh))
        with open(setup_sh, "r", encoding="utf-8") as fh:
            content = fh.read()
        self.assertNotIn(
            "dtype.cpp.patch",
            content,
            "Setup.sh still references dtype.cpp.patch; the patch was superseded by the upgrade",
        )

    def test_patch_directory_does_not_exist(self):
        """Util/Patches/boost-1.84.0/ must not exist after the upgrade."""
        repo_root = os.path.abspath(
            os.path.join(os.path.dirname(__file__), "..", "..", ".."))
        patch_dir = os.path.join(repo_root, "Util", "Patches", "boost-1.84.0")
        self.assertFalse(
            os.path.isdir(patch_dir),
            "Patch directory {} still exists; remove it".format(patch_dir),
        )


class TestBoostRuntimeVersion(unittest.TestCase):
    """Runtime check: carla.libcarla must link against Boost 1.89.0.

    This catches the case where the build cache held a stale
    boost-1.84.0-install/ tree and the linker used that instead of 1.89.0.
    """

    def _get_linked_libs(self, so_path):
        """Return the ldd output for so_path, or '' on unsupported platforms."""
        if sys.platform.startswith("linux"):
            result = subprocess.run(
                ["ldd", so_path], capture_output=True, text=True)
            return result.stdout
        return ""

    def test_libcarla_imports(self):
        """carla module must be importable (basic sanity)."""
        import carla  # noqa: F401 — import is the assertion
        self.assertIsNotNone(carla.__file__)

    def test_no_boost_184_in_linked_libs(self):
        """libcarla.so must not be linked to a boost-1.84.0 library."""
        so_path = _carla_so_path()
        if so_path is None:
            self.skipTest("Could not locate libcarla .so file")
        linked = self._get_linked_libs(so_path)
        self.assertNotIn(
            "boost-1.84.0",
            linked,
            "libcarla is still linked against boost-1.84.0 (stale build cache?)",
        )