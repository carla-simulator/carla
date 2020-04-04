package(features = ["extra-warnings"])

cc_library(
    name = "image",
    hdrs = glob(["*.h"]),
    srcs = glob(["*.cpp"]),
    includes = ["."],
    include_prefix = "carla/image",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        "//LibCarla/source/carla:client",
        "//LibCarla/source/carla:filesystem",
    ],
)
