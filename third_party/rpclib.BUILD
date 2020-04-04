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
    },
    cmake_options = [
        "-GNinja"
    ],
    make_commands = [
        "ninja",
        "ninja install",
    ],
    static_libraries = [
        "librpc.a",
    ],
)

cc_library(
    name = "rpclib",
    hdrs = glob([
      "include/**/*.h",
      "include/**/*.hpp",
      "include/**/*.inl",
    ]),
    includes = [
        "include",
    ],
    deps = [":lib"],
    visibility = ["//visibility:public"],
)
