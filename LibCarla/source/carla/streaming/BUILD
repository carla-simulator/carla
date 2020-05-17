package(features = ["extra-warnings"])

cc_library(
    name = "streaming",
    hdrs = glob(["**/*.h"]),
    srcs = glob(["**/*.cpp"]),
    include_prefix = "carla/streaming",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        "//LibCarla/source/carla/profiler",
        "//LibCarla/source/carla:msgpack",
        "//LibCarla/source/carla:thread",
        "//LibCarla/source/carla:utility",
        "@boost//:headers",
    ],
)
