package(features = ["extra-warnings"])

cc_library(
    name = "profiler",
    hdrs = glob(["*.h"]),
    srcs = glob(["*.cpp"]),
    includes = ["."],
    include_prefix = "carla/profiler",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        "//LibCarla/source/carla:utility",
    ],
)
