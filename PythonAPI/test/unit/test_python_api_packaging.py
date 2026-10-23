"""Build the Python packaging configuration with a small native client fixture.

Requires CMake, Ninja, a C++ compiler, Python development headers, and the
Python packages build, scikit-build-core, and wheel. No simulator is needed.
"""

import importlib.machinery
import importlib.util
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import sysconfig
import tempfile
import unittest
import zipfile


ROOT = Path(__file__).resolve().parents[3]
FIXTURE_PROJECT = r"""
cmake_minimum_required(VERSION 3.27)
project(carla_packaging_fixture LANGUAGES C CXX)
set(CARLA_CMAKE_MINIMUM_REQUIRED_VERSION 3.27.2)
set(CARLA_VERSION 0.10.0)
set(CARLA_WORKSPACE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
include("${CARLA_WORKSPACE_PATH}/CMake/Util.cmake")
find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
add_library(packaging_stub STATIC empty.cpp)
target_include_directories(packaging_stub INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/DependencyHeaders")
foreach(name IN ITEMS carla-client rpc png_static zlibstatic)
  add_library(${name} ALIAS packaging_stub)
endforeach()
foreach(name IN ITEMS asio python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR} geometry gil)
  add_library(Boost::${name} ALIAS packaging_stub)
endforeach()
foreach(name IN ITEMS Recast Detour DetourCrowd)
  add_library(RecastNavigation::${name} ALIAS packaging_stub)
endforeach()
set(libpng_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/PythonAPI/carla/include")
set(libpng_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(zlib_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/PythonAPI/carla/include")
set(zlib_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
add_subdirectory(PythonAPI)
"""
NATIVE_MODULE = r"""
#include <PythonAPI.h>
static PyObject *marker(PyObject *, PyObject *) {
  return PyUnicode_FromString("native-wheel");
}
static PyMethodDef methods[] = {
  {"marker", marker, METH_NOARGS, nullptr}, {nullptr, nullptr, 0, nullptr}
};
static PyModuleDef module = {
  PyModuleDef_HEAD_INIT, "carla", nullptr, -1, methods
};
PyMODINIT_FUNC PyInit_carla() { return PyModule_Create(&module); }
"""


@unittest.skipUnless(shutil.which("cmake") and shutil.which("ninja"),
                     "CMake and Ninja are required")
class TestPythonApiPackaging(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        for package in ("build", "scikit_build_core", "wheel"):
            if importlib.util.find_spec(package) is None:
                raise unittest.SkipTest("Python build package is required: " + package)
        compilers = (os.environ.get("CXX", ""), "cl", "clang-cl", "c++", "g++", "clang++")
        if not any(compiler and shutil.which(compiler) for compiler in compilers):
            raise unittest.SkipTest("A native C++ compiler environment is required")
        if not (Path(sysconfig.get_path("include")) / "Python.h").exists():
            raise unittest.SkipTest("Python development headers are required")

    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix="carla-packaging-")
        self.addCleanup(temporary.cleanup)
        self.directory = Path(temporary.name)

    def fixture(self, name):
        source = self.directory / name
        api = source / "PythonAPI/carla"
        api.mkdir(parents=True)
        (source / "CMake").mkdir()
        shutil.copyfile(ROOT / "CMake/Util.cmake", source / "CMake/Util.cmake")
        shutil.copyfile(ROOT / "PythonAPI/CMakeLists.txt", source / "PythonAPI/CMakeLists.txt")
        for filename in ("CMakeLists.txt.in", "pyproject.toml.in", "README.md"):
            shutil.copyfile(ROOT / "PythonAPI/carla" / filename, api / filename)
        shutil.copyfile(ROOT / "LICENSE", source / "LICENSE")
        (source / "CMakeLists.txt").write_text(FIXTURE_PROJECT)
        (source / "empty.cpp").write_text("int packaging_fixture() { return 0; }\n")
        (api / "include").mkdir()
        (api / "include/PythonAPI.h").write_text(
            "#include <Python.h>\n#include <PackagingDependency.h>\n")
        (source / "DependencyHeaders").mkdir()
        (source / "DependencyHeaders/PackagingDependency.h").write_text(
            "#define PACKAGING_DEPENDENCY_PRESENT 1\n")
        (api / "src").mkdir()
        template = (api / "CMakeLists.txt.in").read_text()
        filenames = re.findall(r"\$\{CARLA_PYTHON_API_SOURCE_PATH\}/(\w+\.cpp)", template)
        self.assertIn("PythonAPI.cpp", filenames)
        for filename in filenames:
            (api / "src" / filename).write_text("// Packaging fixture.\n")
        (api / "src/PythonAPI.cpp").write_text(NATIVE_MODULE)
        return source

    def run_command(self, command):
        return subprocess.run(command, capture_output=True, text=True, timeout=180)

    def configure(self, source, build, pep517=True):
        return self.run_command([
            "cmake", "-S", str(source), "-B", str(build), "-G", "Ninja",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DPython3_EXECUTABLE=" + sys.executable,
            "-DENABLE_PEP517=" + ("ON" if pep517 else "OFF"),
        ])

    def assert_native_wheel(self, source, build):
        configure = self.configure(source, build)
        self.assertEqual(configure.returncode, 0, configure.stdout + configure.stderr)
        result = self.run_command([
            "cmake", "--build", str(build), "--target", "carla-python-api", "-j", "2",
        ])
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        wheels = list((build / "PythonAPI/dist").glob("*.whl"))
        self.assertEqual(len(wheels), 1)
        self.assertNotIn("none-any.whl", wheels[0].name)
        installed = self.directory / "installed"
        with zipfile.ZipFile(wheels[0]) as wheel:
            self.assertTrue(any(
                filename.endswith(tuple(importlib.machinery.EXTENSION_SUFFIXES))
                for filename in wheel.namelist()), wheel.namelist())
            wheel.extractall(installed)
        result = self.run_command([
            sys.executable, "-I", "-c",
            "import sys; sys.path.insert(0, " + repr(str(installed))
            + "); import carla; print(carla.marker())",
        ])
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(result.stdout.strip(), "native-wheel")

    def test_legacy_backend_is_rejected_before_build(self):
        source = self.fixture("source")
        result = self.configure(source, self.directory / "build", pep517=False)
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("ENABLE_PEP517=ON", result.stdout + result.stderr)
        self.assertFalse((source / "PythonAPI/carla/setup.py").exists())

    def test_supported_backend_builds_a_native_wheel(self):
        self.assert_native_wheel(self.fixture("source"), self.directory / "build")

    def test_source_and_build_paths_with_spaces(self):
        self.assert_native_wheel(
            self.fixture("Source With Spaces"), self.directory / "Build With Spaces")


if __name__ == "__main__":
    unittest.main()
