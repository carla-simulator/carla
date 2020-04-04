package(features = ["extra-warnings"])

load("@python//:rules.bzl", "for_each_py_version")

for_each_py_version(
    rule = cc_library,
    name = "libcarla-py%{py_version}.so.includes",
    hdrs = glob([
        "libcarla/*.cpp",
    ], exclude=[
        "libcarla/libcarla.cpp",
    ]),
    defines = ["LIBCARLA_WITH_PYTHON_SUPPORT"],
    deps = [
        "//LibCarla/source/carla:client",
        "//LibCarla/source/carla:client_io",
        "//LibCarla/source/carla:python%{py_version}",
        "@boost//:python%{py_version}",
    ],
)

for_each_py_version(
    rule = cc_binary,
    name = "carla-py%{py_version}.so",
    srcs = [
        "libcarla/libcarla.cpp",
    ],
    copts = [
        "-Wno-self-assign-overloaded",
        "-Wno-shadow",
    ],
    linkstatic = True,
    linkshared = True,
    defines = ["LIBCARLA_PYTHON_MODULE_NAME=carla"],
    linkopts = [
        "-ljpeg",
        "-lpng",
        "-ltiff",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":libcarla-py%{py_version}.so.includes",
    ],
)

for_each_py_version(
    rule = py_library,
    name = "carla-py%{py_version}",
    data = [":carla-py%{py_version}.so"],
    imports = ["."],
    visibility = ["//visibility:public"],
)
