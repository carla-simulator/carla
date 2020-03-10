load("@python//:rules.bzl", "for_each_py_version", "py_wheels_and_eggs")

for_each_py_version(
    rule = cc_library,
    name = "libcarla-py%{py_version}.so.includes",
    hdrs = glob(["libcarla/*.cpp"], exclude=["libcarla/libcarla.cpp"]),
    defines = ["LIBCARLA_WITH_PYTHON_SUPPORT"],
    deps = [
        "//LibCarla/source:client",
        "//LibCarla/source:python%{py_version}",
        "@boost//:python%{py_version}",
    ],
)

for_each_py_version(
    rule = cc_binary,
    name = "carla-py%{py_version}.so",
    srcs = ["libcarla/libcarla.cpp"],
    copts = ["-Wno-self-assign-overloaded"],
    linkstatic = True,
    linkshared = True,
    defines = ["LIBCARLA_PYTHON_MODULE_NAME=carla"],
    linkopts = [
        "-ljpeg",
        "-lpng",
        "-ltiff",
    ],
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

py_wheels_and_eggs(
    name = "dist",
    data = [":carla-py%{py_version}.so"],
    package_name = "carla",
    version = "0.9.8",
    license = "MIT License",
    description = "Python API for communicating with the CARLA server.",
    url = "https://github.com/carla-simulator/carla",
    author = "The CARLA team",
    author_email = "carla.simulator@gmail.com",
)
