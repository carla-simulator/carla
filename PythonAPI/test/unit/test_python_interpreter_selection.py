# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.

"""Configure actual Boost.Python and CARLA discovery with real interpreters.

Set CARLA_TEST_BOOST_PYTHON_SOURCE to Boost's libs/python directory when it is
not present in this checkout's build cache. CARLA_TEST_OTHER_PYTHON enables the
conflicting-version control on a machine with another Python installation.
Only dependency targets and installation are inert; no library is compiled.
"""

import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[3]


def boost_python_source():
    """Find real Boost sources from an explicit path or production build cache."""
    explicit = os.environ.get("CARLA_TEST_BOOST_PYTHON_SOURCE")
    if explicit:
        return Path(explicit)
    for build in (ROOT / "Build/Release", ROOT / "Build"):
        direct = build / "_deps/boost-src/libs/python"
        if (direct / "CMakeLists.txt").is_file():
            return direct
        cache = build / "CMakeCache.txt"
        if cache.is_file():
            match = re.search(r"^FETCHCONTENT_SOURCE_DIR_BOOST:[^=]+=(.+)$",
                              cache.read_text(encoding="utf-8"), re.MULTILINE)
            if match:
                return Path(match[1].strip()) / "libs/python"
    return None


class TestPythonInterpreterSelection(unittest.TestCase):
    """The versioned Boost target must match the actual Python API selection."""

    @classmethod
    def setUpClass(cls):
        cls.boost_source = boost_python_source()
        if not shutil.which("cmake"):
            raise unittest.SkipTest("CMake is required")
        if cls.boost_source is None or not (cls.boost_source / "CMakeLists.txt").is_file():
            raise unittest.SkipTest("Real Boost.Python CMake sources are required")

    def project(self):
        """Assemble actual production sections around inert dependency targets."""
        dependencies = (ROOT / "CMake/Dependencies.cmake").read_text(encoding="utf-8")
        option_macro = re.search(r"macro \(carla_dependency_option NAME VALUE\).*?endmacro \(\)",
                                 dependencies, re.DOTALL)
        self.assertIsNotNone(option_macro)
        boost_section = dependencies.split("# ==== BOOST ====", 1)[1].split("# ==== EIGEN ====", 1)[0]
        selection = ""
        if "# ==== PYTHON ====" in dependencies:
            selection = dependencies.split("# ==== PYTHON ====", 1)[1].split("# ==== BOOST ====", 1)[0]
        api_source = (ROOT / "PythonAPI/CMakeLists.txt").read_text(encoding="utf-8")
        lookup = re.search(r"get_target_property\s*\(\s*Boost_python_TARGET_NAME[\s\S]+?\n\)", api_source)
        self.assertIsNotNone(lookup)
        api_prefix = api_source[:lookup.end()]
        boost_cmake = (self.boost_source / "CMakeLists.txt").read_text(encoding="utf-8")
        targets = set(re.findall(r"Boost::[a-zA-Z_]+", boost_cmake))
        targets.update(("Boost::asio", "Boost::geometry", "Boost::gil"))
        targets = sorted(name for name in targets if not name.startswith(("Boost::python", "Boost::numpy")))
        aliases = "".join("add_library(" + name + " ALIAS fixture_dependency)\n" for name in targets)
        project = (
            "cmake_minimum_required(VERSION 3.27)\n"
            "project(carla_python_selection_fixture LANGUAGES CXX)\n"
            f'set(CARLA_WORKSPACE_PATH "{ROOT.as_posix()}")\n'
            'include("${CARLA_WORKSPACE_PATH}/CMake/Util.cmake")\n'
            'option(BUILD_PYTHON_API "" ON)\n'
            "set(ENABLE_PEP517 ON)\n"
            "set(Python_FIND_VIRTUALENV STANDARD)\n"
            "set(Python3_FIND_VIRTUALENV STANDARD)\n"
            "set(BOOST_SUPERPROJECT_VERSION 1.90.0)\n"
            "set(CARLA_BOOST_TAG boost-1.90.0)\n"
            "function(boost_install)\nendfunction()\n"
            "add_library(fixture_dependency INTERFACE)\n"
            + aliases + option_macro.group() + "\n"
            "macro(carla_dependency_add NAME TAG ARCHIVE_URL GIT_URL)\n"
            "  if(BOOST_ENABLE_PYTHON)\n"
            f'    add_subdirectory("{self.boost_source.as_posix()}" "${{CMAKE_BINARY_DIR}}/real-boost-python")\n'
            "  endif()\nendmacro()\n"
            + selection + boost_section + "\nif(BUILD_PYTHON_API)\n" + api_prefix + "\nendif()\n"
            'file(WRITE "${CMAKE_BINARY_DIR}/selection.txt" '
            '"${Python3_EXECUTABLE}\\n${Python3_VERSION}\\n${Boost_python_TARGET_NAME}")\n'
        )
        return project

    def configure(self, hints=(), enabled=True, reconfigure=None):
        """Run production selection sections and the actual Boost CMake module."""
        with tempfile.TemporaryDirectory(prefix="carla-python-selection-") as directory:
            source = Path(directory)
            (source / "CMakeLists.txt").write_text(self.project(), encoding="utf-8")
            command = ["cmake", "-S", str(source), "-B", str(source / "build"), "-G", "Ninja"]
            if not enabled:
                command.append("-DBUILD_PYTHON_API=OFF")
            environment = os.environ.copy()
            other = environment.get("CARLA_TEST_OTHER_PYTHON")
            if other:
                environment["PATH"] = str(Path(other).parent) + os.pathsep + environment.get("PATH", "")
            result = subprocess.run(command + list(hints), env=environment, capture_output=True, text=True,
                                    encoding="utf-8", errors="replace", check=False, timeout=120)
            if reconfigure is not None:
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                result = subprocess.run(command + list(reconfigure), env=environment, capture_output=True, text=True,
                                        encoding="utf-8", errors="replace", check=False, timeout=120)
            path = source / "build/selection.txt"
            selected = path.read_text(encoding="utf-8").splitlines() if path.exists() else []
        return result, selected

    def assert_matching_selection(self, result, selected):
        """A real versioned Boost target must exist for the selected interpreter."""
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(len(selected), 3)
        version = selected[1].split(".")
        self.assertEqual(selected[2], "boost_python" + version[0] + version[1])

    def test_python3_hint_controls_boost_selection(self):
        """The documented Python3 executable hint reaches Boost's namespace."""
        result, selected = self.configure(["-DPython3_EXECUTABLE=" + sys.executable])
        self.assert_matching_selection(result, selected)
        self.assertEqual(Path(selected[0]).resolve(), Path(sys.executable).resolve())

    def test_python_hint_controls_api_selection(self):
        """A legacy FindPython executable hint also selects the API interpreter."""
        result, selected = self.configure(["-DPython_EXECUTABLE=" + sys.executable])
        self.assert_matching_selection(result, selected)
        self.assertEqual(Path(selected[0]).resolve(), Path(sys.executable).resolve())

    def test_python3_root_controls_boost_selection(self):
        """The modern directory hint also selects Boost's interpreter."""
        self.assert_matching_selection(*self.configure(["-DPython3_ROOT_DIR=" + sys.base_prefix]))

    def test_python_root_controls_api_selection(self):
        """The legacy directory hint reaches the API's discovery namespace."""
        self.assert_matching_selection(*self.configure(["-DPython_ROOT_DIR=" + sys.base_prefix]))

    def test_default_selection_control(self):
        """Unhinted discovery still creates the matching versioned target."""
        self.assert_matching_selection(*self.configure())

    def test_matching_explicit_hints_control(self):
        """Explicit matching hints retain the existing successful behavior."""
        self.assert_matching_selection(*self.configure([
            "-DPython_EXECUTABLE=" + sys.executable,
            "-DPython3_EXECUTABLE=" + sys.executable]))

    def test_conflicting_explicit_hints_fail(self):
        """Two incompatible installed interpreters must not configure a module."""
        other = os.environ.get("CARLA_TEST_OTHER_PYTHON")
        if not other:
            self.skipTest("A second installed Python is required for the conflict control")
        result, _ = self.configure([
            "-DPython_EXECUTABLE=" + other,
            "-DPython3_EXECUTABLE=" + sys.executable])
        self.assertNotEqual(result.returncode, 0)

    def test_disabled_api_does_not_require_python(self):
        """Client-only selection does not query deliberately invalid hints."""
        result, _ = self.configure([
            "-DPython_EXECUTABLE=/not/an/interpreter",
            "-DPython3_EXECUTABLE=/also/not/an/interpreter"], enabled=False)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_reconfigure_replaces_cached_default_selection(self):
        """Changing the API hint after a default configure changes both consumers."""
        self.assert_matching_selection(*self.configure(
            reconfigure=["-DPython3_EXECUTABLE=" + sys.executable]))

    def test_compatible_base_and_venv_hints_control(self):
        """Distinct venv/base executables with one ABI remain a valid input."""
        base = getattr(sys, "_base_executable", sys.executable)
        self.assert_matching_selection(*self.configure([
            "-DPython_EXECUTABLE=" + base,
            "-DPython3_EXECUTABLE=" + sys.executable]))

    def test_reconfigure_replaces_explicit_python3_selection(self):
        """Changing an explicit modern executable also replaces both libraries."""
        other = os.environ.get("CARLA_TEST_OTHER_PYTHON")
        if not other:
            self.skipTest("A second installed Python is required for the switch control")
        result, selected = self.configure(
            ["-DPython3_EXECUTABLE=" + sys.executable],
            reconfigure=["-DPython3_EXECUTABLE=" + other])
        self.assert_matching_selection(result, selected)
        self.assertEqual(Path(selected[0]).resolve(), Path(other).resolve())

    def test_reconfigure_replaces_explicit_legacy_selection(self):
        """Changing the legacy executable hint updates the API and Boost together."""
        other = os.environ.get("CARLA_TEST_OTHER_PYTHON")
        if not other:
            self.skipTest("A second installed Python is required for the switch control")
        result, selected = self.configure(
            ["-DPython_EXECUTABLE=" + sys.executable],
            reconfigure=["-DPython_EXECUTABLE=" + other])
        self.assert_matching_selection(result, selected)
        self.assertEqual(Path(selected[0]).resolve(), Path(other).resolve())
