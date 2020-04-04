package(features = ["extra-warnings"])

cc_library(
    name = "rpc",
    hdrs = glob(["**/*.h"]),
    srcs = glob(["**/*.cpp"]),
    includes = ["."],
    include_prefix = "carla/rpc",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        "//LibCarla/source/carla/geom",
        "//LibCarla/source/carla/streaming",
        "//LibCarla/source/carla:msgpack",
    ],
)
