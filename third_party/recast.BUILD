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
        "CMAKE_CXX_FLAGS": "-Wno-error",
    },
    cmake_options = [
        "-GNinja"
    ],
    make_commands = [
        "ninja",
        "ninja install",
    ],
    # postfix_script = "tree ; exit 1",
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
    include_prefix = "recast/",
    includes = [
        "Recast/Include",
    ],
    deps = [":lib"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "Detour",
    hdrs = glob([
        "Detour/Include/**/*.h",
    ]),
    strip_include_prefix = "Detour/Include",
    include_prefix = "recast/",
    includes = [
        "Detour/Include",
    ],
    deps = [":lib"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "DetourCrowd",
    hdrs = glob([
        "DetourCrowd/Include/**/*.h",
    ]),
    strip_include_prefix = "DetourCrowd/Include",
    include_prefix = "recast/",
    includes = [
        "DetourCrowd/Include",
    ],
    deps = [":lib"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "recast",
    deps = [
      ":Recast",
      ":Detour",
      ":DetourCrowd",
    ],
    visibility = ["//visibility:public"],
)
