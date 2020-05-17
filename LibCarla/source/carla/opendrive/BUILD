package(features = ["extra-warnings"])

cc_library(
    name = "opendrive",
    hdrs = glob(["**/*.h"]),
    srcs = glob(["**/*.cpp"]),
    include_prefix = "carla/opendrive",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        "//LibCarla/source/carla/road",
        "//LibCarla/source/carla:utility",
        "//LibCarla/source/third-party:pugixml",
    ],
)
