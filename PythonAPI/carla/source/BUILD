load("@python//:dist.bzl", "py_wheel_and_egg")

cc_library(
    name = "libcarla.so.includes",
    hdrs = glob(["libcarla/*.cpp"], exclude=["libcarla/libcarla.cpp"]),
    defines = ["LIBCARLA_WITH_PYTHON_SUPPORT"],
    deps = [
        "//LibCarla/source:client",
        "//LibCarla/source:python",
        "@boost//:python",
    ],
)

cc_binary(
    name = "carla.so",
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
        ":libcarla.so.includes",
    ],
)

py_library(
    name = "carla",
    data = [":carla.so"],
    imports = ["."],
    visibility = ["//visibility:public"],
)

py_wheel_and_egg(
    name = "dist",
    srcs = [":carla.so"],
    package_name = "carla",
    version = "0.9.7",
    license = "MIT License",
    description = "Python API for communicating with the CARLA server.",
    url = "https://github.com/carla-simulator/carla",
    author = "The CARLA team",
    author_email = "carla.simulator@gmail.com",
)
