package(features = ["extra-warnings"])

cc_library(
    name = "geom",
    hdrs = glob(["*.h"]),
    srcs = glob(["*.cpp"]),
    include_prefix = "carla/geom",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        "//LibCarla/source/carla:utility",
        "//LibCarla/source/carla:msgpack",
    ],
)
