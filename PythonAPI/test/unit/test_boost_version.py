# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Verify that the Boost dependency used to build carla.libcarla is 1.90.0.

ue5-dev builds Boost via CMake FetchContent, so the version pin lives in
CMake/Options.cmake (CARLA_BOOST_VERSION). A regression here means the
FetchContent download is pointing at a pre-1.90 Boost (which lacks the
NumPy 2 ABI fix and still ships the deprecated boost::asio APIs LibCarla
no longer compiles against).
"""

import os
import re
import unittest

EXPECTED_BOOST_VERSION = "1.90.0"
STALE_BOOST_VERSIONS = ("1.84.0", "1.89.0")

REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", ".."))

REQUIREMENTS_FILES = (
    os.path.join(REPO_ROOT, "requirements.txt"),
    os.path.join(REPO_ROOT, "PythonAPI", "carla", "requirements.txt"),
    os.path.join(REPO_ROOT, "PythonAPI", "util", "requirements.txt"),
    os.path.join(REPO_ROOT, "PythonAPI", "examples", "requirements.txt"),
)


class TestBoostInstallVersion(unittest.TestCase):
    """Static check: CMake/Options.cmake must pin Boost 1.90.0."""

    def test_cmake_options_pins_correct_version(self):
        """CARLA_BOOST_VERSION in CMake/Options.cmake must be 1.90.0."""
        options_cmake = os.path.join(REPO_ROOT, "CMake", "Options.cmake")
        self.assertTrue(
            os.path.exists(options_cmake),
            f"Expected CMake/Options.cmake at {options_cmake}")
        with open(options_cmake) as f:
            contents = f.read()

        # The option is declared as:
        #   carla_string_option (
        #     CARLA_BOOST_VERSION
        #     "Target boost version."
        #     1.90.0
        #   )
        pattern = re.compile(
            r"CARLA_BOOST_VERSION\s+\"[^\"]*\"\s+(\S+)\s*\)",
            re.MULTILINE)
        match = pattern.search(contents)
        self.assertIsNotNone(
            match,
            "Could not locate CARLA_BOOST_VERSION default in CMake/Options.cmake")
        self.assertEqual(
            match.group(1),
            EXPECTED_BOOST_VERSION,
            f"CARLA_BOOST_VERSION must be {EXPECTED_BOOST_VERSION}, "
            f"found {match.group(1)}")

    def test_no_stale_boost_version_in_options(self):
        """CMake/Options.cmake must not mention any pre-1.90 Boost version."""
        options_cmake = os.path.join(REPO_ROOT, "CMake", "Options.cmake")
        with open(options_cmake) as f:
            contents = f.read()
        for stale in STALE_BOOST_VERSIONS:
            self.assertNotIn(
                stale,
                contents,
                f"CMake/Options.cmake still references stale Boost {stale}")


class TestNumpyRequirementsCeiling(unittest.TestCase):
    """Static check: no requirements file may pin numpy<2.0.

    The whole point of upgrading Boost to 1.90 is to allow NumPy 2; a stale
    `numpy<2.0` ceiling defeats it.
    """

    def test_no_numpy_2_ceiling_in_requirements(self):
        for path in REQUIREMENTS_FILES:
            self.assertTrue(
                os.path.exists(path),
                f"Expected requirements file at {path}")
            with open(path) as f:
                contents = f.read()
            self.assertNotIn(
                "numpy<2.0",
                contents,
                f"{path} still pins numpy<2.0 (incompatible with the Boost 1.90 / "
                f"NumPy 2 upgrade)")


class TestCarlaImports(unittest.TestCase):
    """Sanity check that the built wheel is importable.

    Skipped when the wheel is not installed (so the suite still runs in
    environments without a finished CMake build).
    """

    def test_carla_imports(self):
        try:
            import carla  # noqa: F401
        except ModuleNotFoundError:
            self.skipTest("carla wheel not installed in this environment")
        else:
            self.assertIsNotNone(carla.__file__)


if __name__ == "__main__":
    unittest.main()
