# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Verify that the installed Boost used to build carla.libcarla is 1.90.0.

Guards against silent cache hits from any pre-1.90.0 Boost install directory
(1.84.0 or 1.89.0) that would otherwise pass the NumPy 2 test suite while
still linking against an old Boost.
"""

import os
import re
import unittest

EXPECTED_BOOST_VERSION = "1.90.0"
EXPECTED_BOOST_DIR = "boost-{}-".format(EXPECTED_BOOST_VERSION)
STALE_BOOST_VERSIONS = ("1.84.0", "1.89.0")


class TestBoostInstallVersion(unittest.TestCase):
    """Static check: Setup.sh must pin Boost 1.90.0."""

    def test_setup_sh_pins_correct_version(self):
        """BOOST_VERSION in Setup.sh must be 1.90.0."""
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
    """Runtime check: PythonAPI dependencies must use Boost 1.90.0.

    Boost is statically linked into libcarla.so, so ldd does not show it.
    We inspect the versioned .so symlinks in PythonAPI/carla/dependencies/lib/
    instead, which are the authoritative record of which Boost the PythonAPI
    build consumed.  This catches the case where the build cache held a stale
    Boost install tree (1.84.0 or 1.89.0) and the linker used it instead of
    1.90.0.
    """

    def _deps_lib(self):
        """Return the absolute path of PythonAPI/carla/dependencies/lib/."""
        return os.path.abspath(
            os.path.join(os.path.dirname(__file__), "..", "..", "carla", "dependencies", "lib"))

    def test_libcarla_imports(self):
        """carla module must be importable (basic sanity)."""
        import carla  # noqa: F401 — import is the assertion
        self.assertIsNotNone(carla.__file__)

    def test_libcarla_links_expected_boost_version(self):
        """PythonAPI dependencies/lib must contain libboost_*.so.1.90.0 files."""
        deps_lib = self._deps_lib()
        if not os.path.isdir(deps_lib):
            self.skipTest("PythonAPI dependencies/lib not found (not built yet?)")
        found = any(
            f.startswith("libboost_") and ".so.{}".format(EXPECTED_BOOST_VERSION) in f
            for f in os.listdir(deps_lib)
        )
        self.assertTrue(
            found,
            "No libboost_*.so.{} found in {} (wrong or missing Boost install?)".format(
                EXPECTED_BOOST_VERSION, deps_lib),
        )

    def test_no_stale_boost_in_linked_libs(self):
        """PythonAPI dependencies/lib must not contain any pre-1.90.0 Boost library."""
        deps_lib = self._deps_lib()
        if not os.path.isdir(deps_lib):
            self.skipTest("PythonAPI dependencies/lib not found (not built yet?)")
        for stale_version in STALE_BOOST_VERSIONS:
            stale_files = [
                f for f in os.listdir(deps_lib)
                if f.startswith("libboost_") and ".so.{}".format(stale_version) in f
            ]
            self.assertEqual(
                stale_files,
                [],
                "Found stale boost-{} libraries in {} (stale build cache?): {}".format(
                    stale_version, deps_lib, stale_files),
            )
