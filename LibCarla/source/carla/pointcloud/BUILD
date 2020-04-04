package(features = ["extra-warnings"])

cc_library(
    name = "pointcloud",
    hdrs = glob(["*.h"]),
    srcs = glob(["*.cpp"]),
    includes = ["."],
    include_prefix = "carla/pointcloud",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        "//LibCarla/source/carla:filesystem",
    ],
)
