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
        "libRecast.a",
        "libDetour.a",
        "libDetourCrowd.a",
    ],
)

cc_library(
    name = "Recast",
    hdrs = glob([
        "Recast/Include/**/*.h",
    ]),
    strip_include_prefix = "Recast/Include",
    includes = [
        "Recast/Include",
    ],
    include_prefix = "recast/",
    visibility = ["//visibility:public"],
    deps = [":lib"],
)

cc_library(
    name = "Detour",
    hdrs = glob([
        "Detour/Include/**/*.h",
    ]),
    strip_include_prefix = "Detour/Include",
    includes = [
        "Detour/Include",
    ],
    include_prefix = "recast/",
    visibility = ["//visibility:public"],
    deps = [":lib"],
)

cc_library(
    name = "DetourCrowd",
    hdrs = glob([
        "DetourCrowd/Include/**/*.h",
    ]),
    strip_include_prefix = "DetourCrowd/Include",
    includes = [
        "DetourCrowd/Include",
    ],
    include_prefix = "recast/",
    visibility = ["//visibility:public"],
    deps = [":lib"],
)

cc_library(
    name = "recast",
    visibility = ["//visibility:public"],
    deps = [
      ":Recast",
      ":Detour",
      ":DetourCrowd",
    ],
)
