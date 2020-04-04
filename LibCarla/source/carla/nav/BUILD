package(features = ["extra-warnings"])

cc_library(
    name = "nav",
    hdrs = glob(["**/*.h"]),
    srcs = glob(["**/*.cpp"]),
    includes = ["."],
    include_prefix = "carla/nav",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        "//LibCarla/source/carla/geom",
        "//LibCarla/source/carla/rpc",
        "//LibCarla/source/carla:thread",
        "//LibCarla/source/carla:utility",
        "@recast",
    ],
)
