package(features = ["extra-warnings"])

load("@//Util/Bazel:macros.bzl", "cc_gtest_library", "cc_gtest_test")

cc_gtest_library(
    name = "common",
    hdrs = glob(["*.h"]),
    srcs = glob(["*.cpp"]),
    include_prefix = "",
    deps = [
        "//LibCarla/source/carla/geom",
        "//LibCarla/source/carla/profiler",
    ],
)

cc_gtest_test(
    name = "version",
    srcs = [
        "common/test_miscellaneous.cpp",
    ],
    deps = [
        "//:version",
        ":common",
    ],
)

cc_gtest_test(
    name = "utility",
    srcs = [
        "common/test_buffer.cpp",
        "common/test_listview.cpp",
    ],
    deps = [
        "//LibCarla/source/carla:utility",
        ":common",
    ],
)

cc_gtest_test(
    name = "rpc",
    srcs = [
        "client/test_rpc.cpp",
        "common/test_msgpack.cpp",
    ],
    deps = [
        "//LibCarla/source/carla/rpc",
        ":common",
    ],
)

cc_gtest_test(
    name = "geom",
    srcs = [
        "common/test_geom.cpp",
        "common/test_vector3D.cpp",
    ],
    deps = [
        "//LibCarla/source/carla/geom",
        ":common",
    ],
)

cc_gtest_test(
    name = "thread",
    srcs = [
        "client/test_recurrent_shared_future.cpp",
    ],
    deps = [
        "//LibCarla/source/carla:thread",
        ":common",
    ],
)

cc_gtest_test(
    name = "image",
    srcs = [
        "client/test_image.cpp",
    ],
    deps = [
        ":common",
        "//LibCarla/source/carla/image",
    ],
)

cc_gtest_test(
    name = "opendrive",
    srcs = [
        "client/OpenDrive.cpp",
        "client/OpenDrive.h",
        "client/test_opendrive.cpp",
    ],
    copts = [
        "-DLIBCARLA_TEST_CONTENT_FOLDER=\\\"external/xodr_files\\\""
    ],
    data = ["@xodr_files"],
    deps = [
        "//LibCarla/source/carla/opendrive",
        "//LibCarla/source/carla:filesystem",
        "//LibCarla/source/carla:thread",
        ":common",
    ],
)

cc_gtest_test(
    name = "streaming",
    srcs = [
        "common/test_streaming.cpp",
    ],
    deps = [
        "//LibCarla/source/carla/streaming",
        ":common",
    ],
)

cc_gtest_test(
    name = "streaming_benchmark",
    srcs = [
        "server/test_benchmark_streaming.cpp",
    ],
    deps = [
        "//LibCarla/source/carla/streaming",
        ":common",
    ],
)
