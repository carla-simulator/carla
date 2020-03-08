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
    name = "libcarla.so",
    srcs = ["libcarla/libcarla.cpp"],
    copts = ["-Wno-self-assign-overloaded"],
    linkstatic = True,
    linkshared = True,
    deps = [
        ":libcarla.so.includes",
    ],
)

py_library(
    name = "carla",
    srcs = glob(["carla/**/*.py"]),
    data = [":libcarla.so"],
    imports = ["."],
    visibility = ["//visibility:public"],
)
