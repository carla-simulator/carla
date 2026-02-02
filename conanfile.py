from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy
import os

class LibCarlaConan(ConanFile):
    name = "libcarla"
    version = "0.10.0"
    license = "MIT"
    author = "Computer Vision Center (CVC)"
    url = "https://github.com/carla-simulator/carla"
    description = "CARLA Simulator C++ Library"
    topics = ("autonomous-driving", "simulation", "carla")

    settings = "os", "compiler", "build_type", "arch"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_client": [True, False],
        "with_server": [True, False],
        "enable_rtti": [True, False],
        "enable_exceptions": [True, False],
    }

    default_options = {
        "shared": False,
        "fPIC": True,
        "with_client": True,
        "with_server": True,
        "enable_rtti": True,
        "enable_exceptions": True,
    }

    exports_sources = "LibCarla/*", "CMakeLists.txt", "CMake/*", "LICENSE"

    def requirements(self):
        # Fetch from ConanCenter
        self.requires("boost/1.84.0")
        self.requires("eigen/3.4.0")
        self.requires("zlib/1.3.1")
        self.requires("libpng/1.6.42")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

        # Configure Boost options
        self.options["boost"].header_only = False
        self.options["boost"].without_cobalt = True
        self.options["boost"].without_python = True  # Python disabled in Phase 1

    def config_options(self):
        # fPIC always enabled for Ubuntu
        pass

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)

        # Map CMake options
        tc.variables["BUILD_CARLA_CLIENT"] = self.options.with_client
        tc.variables["BUILD_CARLA_SERVER"] = self.options.with_server
        tc.variables["BUILD_PYTHON_API"] = False
        tc.variables["BUILD_CARLA_UNREAL"] = False
        tc.variables["BUILD_EXAMPLES"] = False
        tc.variables["BUILD_LIBCARLA_TESTS"] = False
        tc.variables["ENABLE_RTTI"] = self.options.enable_rtti
        tc.variables["ENABLE_EXCEPTIONS"] = self.options.enable_exceptions
        tc.variables["INSTALL_LIBCARLA"] = True

        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

        copy(self, "LICENSE",
             src=self.source_folder,
             dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "carla")

        # carla-server component
        if self.options.with_server:
            self.cpp_info.components["server"].set_property(
                "cmake_target_name", "carla::carla-server"
            )
            self.cpp_info.components["server"].libs = ["carla-server"]
            self.cpp_info.components["server"].includedirs = ["include"]

        # carla-client component
        if self.options.with_client:
            self.cpp_info.components["client"].set_property(
                "cmake_target_name", "carla::carla-client"
            )
            self.cpp_info.components["client"].libs = ["carla-client"]
            self.cpp_info.components["client"].includedirs = ["include"]
