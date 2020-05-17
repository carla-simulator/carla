package(features = ["extra-warnings"])

cc_library(
    name = "road",
    hdrs = glob(["**/*.h"]),
    srcs = glob(["**/*.cpp"]),
    include_prefix = "carla/road",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        "//LibCarla/source/carla/geom",
        "//LibCarla/source/carla:string",
        "//LibCarla/source/carla:utility",
        "//LibCarla/source/third-party:odrSpiral",
    ],
)
