package(features = ["extra-warnings"])

cc_library(
    name = "image",
    hdrs = glob(["*.h"]),
    srcs = glob(["*.cpp"]),
    includes = ["."],
    include_prefix = "carla/image",
    defines = [
        "LIBCARLA_IMAGE_WITH_PNG_SUPPORT=true",
        "LIBCARLA_IMAGE_WITH_JPEG_SUPPORT=true",
        "LIBCARLA_IMAGE_WITH_TIFF_SUPPORT=false",
    ],
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        "//LibCarla/source/carla:client",
        "//LibCarla/source/carla:filesystem",
        "@libjpeg-turbo",
        "@libpng",
    ],
)
