from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout
from conan.tools.build import can_run
import os

class LibCarlaTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires(self.tested_reference_str)

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        if can_run(self):
            cmd = os.path.join(self.cpp.build.bindir, "test_libcarla")
            self.run(cmd, env="conanrun")
