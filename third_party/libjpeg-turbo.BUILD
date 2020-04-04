load("@rules_foreign_cc//tools/build_defs:cmake.bzl", "cmake_external")

filegroup(
    name = "all",
    srcs = glob(["**"])
)

cmake_external(
    name = "lib",
    generate_crosstool_file = True,
    lib_source = ":all",
    cache_entries = {
        "CMAKE_CXX_FLAGS": "-Wno-error -fPIC",
        "CMAKE_SYSTEM_PROCESSOR": "x86_64",
    },
    cmake_options = [
        "-GNinja"
    ],
    make_commands = [
        "ninja",
        "ninja install",
    ],
    static_libraries = [
        "libjpeg.a",
    ],
)

cc_library(
    name = "libjpeg-turbo",
    hdrs = glob([
      "include/jpeg.h",
    ]),
    includes = [
        "include",
    ],
    deps = [":lib"],
    visibility = ["//visibility:public"],
)
