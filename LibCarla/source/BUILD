package(features = ["paranoid"])

cc_library(
    name = "utility",
    hdrs = [
        "carla/Buffer.h",
        "carla/BufferPool.h",
        "carla/Debug.h",
        "carla/Exception.h",
        "carla/Functional.h",
        "carla/Iterator.h",
        "carla/ListView.h",
        "carla/Logging.h",
        "carla/Memory.h",
        "carla/MoveHandler.h",
        "carla/NonCopyable.h",
        "carla/Platform.h",
        "carla/StopWatch.h",
        "carla/Time.h",
        "carla/TypeTraits.h",
    ],
    srcs = [
        "carla/Exception.cpp",
        "carla/Buffer.cpp",
    ],
    includes = ["."],
    deps = [
        "//LibCarla/source/third-party:moodycamel",
        "@boost//:headers",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "thread",
    hdrs = [
        "carla/AtomicList.h",
        "carla/AtomicSharedPtr.h",
        "carla/RecurrentSharedFuture.h",
        "carla/ThreadGroup.h",
        "carla/ThreadPool.h",
    ],
    includes = ["."],
    deps = [
        "@boost//:headers",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "string",
    hdrs = [
        "carla/StringUtil.h",
    ],
    srcs = [
        "carla/StringUtil.cpp",
    ],
    includes = ["."],
    deps = [
        "//LibCarla/source/third-party:moodycamel",
        ":utility",
        "@boost//:headers",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "python",
    hdrs = [
        "carla/PythonUtil.h",
    ],
    includes = ["."],
    deps = [
        "@boost//:headers",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "msgpack",
    hdrs = [
        "carla/MsgPack.h",
        "carla/MsgPackAdaptors.h",
    ],
    includes = ["."],
    deps = [
        ":utility",
        "@rpclib",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "filesystem",
    hdrs = ["carla/FileSystem.h"],
    srcs = ["carla/FileSystem.cpp"],
    includes = ["."],
    deps = [
        "//LibCarla/source/third-party:moodycamel",
        ":string",
        ":utility",
        "@boost//:filesystem",
    ],
    visibility = ["//visibility:public"],
)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

cc_library(
    name = "client",
    hdrs = glob([
        "carla/client/**/*.h",
        "carla/Sockets.h",
        # because circular dependencies...
        "carla/image/**/*.h",
        "carla/nav/**/*.h",
        "carla/sensor/data/*.h",
        "carla/trafficmanager/**/*.h",
    ]),
    srcs = glob([
        "carla/client/**/*.cpp",
        # because circular dependencies...
        "carla/image/**/*.cpp",
        "carla/nave/**/*.cpp",
        "carla/sensor/**/*.cpp",
        "carla/trafficmanager/**/*.cpp",
    ]),
    includes = ["."],
    deps = [
        "//:version",
        ":geom",
        ":opendrive",
        ":pointcloud",
        ":python",
        ":road",
        ":rpc",
        ":sensor_s11n_headers",
        ":streaming",
        ":thread",
        ":utility",
        "@recast",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "geom",
    hdrs = glob(["carla/geom/**/*.h"]),
    srcs = glob(["carla/geom/**/*.cpp"]),
    includes = ["."],
    deps = [
        ":utility",
        ":msgpack",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "opendrive",
    hdrs = glob(["carla/opendrive/**/*.h"]),
    srcs = glob(["carla/opendrive/**/*.cpp"]),
    includes = ["."],
    deps = [
        "//LibCarla/source/third-party:pugixml",
        ":road",
        ":utility",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "pointcloud",
    hdrs = glob(["carla/pointcloud/**/*.h"]),
    srcs = glob(["carla/pointcloud/**/*.cpp"]),
    includes = ["."],
    deps = [
        ":filesystem",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "profiler",
    hdrs = glob(["carla/profiler/**/*.h"]),
    srcs = glob(["carla/profiler/**/*.cpp"]),
    includes = ["."],
    deps = [
        ":utility",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "road",
    hdrs = glob(["carla/road/**/*.h"]),
    srcs = glob(["carla/road/**/*.cpp"]),
    includes = ["."],
    deps = [
        ":geom",
        ":string",
        ":utility",
        "//LibCarla/source/third-party:odrSpiral",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "rpc",
    hdrs = glob(["carla/rpc/**/*.h"]),
    srcs = glob(["carla/rpc/**/*.cpp"]),
    includes = ["."],
    deps = [
        ":geom",
        ":msgpack",
        ":streaming",
    ],
    visibility = ["//visibility:public"],
)

# cc_library(
#     name = "rss",
#     hdrs = glob(["carla/rss/**/*.h"]),
#     srcs = glob(["carla/rss/**/*.cpp"]),
#     includes = ["."],
#     deps = [
#     ],
#     visibility = ["//visibility:public"],
# )

# Headers only, compatible with server build.
cc_library(
    name = "sensor_s11n_headers",
    hdrs = glob([
        "carla/sensor/*.h",
        "carla/sensor/s11n/*.h",
    ]),
    includes = ["."],
    deps = [
        # ":geom",
        # ":rpc",
        # ":utility",
        # ":streaming",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "streaming",
    hdrs = glob(["carla/streaming/**/*.h"]),
    srcs = glob(["carla/streaming/**/*.cpp"]),
    includes = ["."],
    deps = [
        ":utility",
        ":msgpack",
        ":profiler",
        ":thread",
        "@boost//:headers",
    ],
    visibility = ["//visibility:public"],
)

# ==============================================================================
# -- Test ----------------------------------------------------------------------
# ==============================================================================

load("@//Util/Bazel:macros.bzl", "cc_gtest_library", "cc_gtest_test")

cc_gtest_library(
    name = "test_common",
    hdrs = glob(["test/*.h"]),
    srcs = glob(["test/*.cpp"]),
    includes = ["test"],
    deps = [
        ":geom",
        ":profiler",
    ],
)

cc_gtest_test(
    name = "test_misc",
    srcs = [
        "test/client/test_image.cpp",
        "test/client/test_recurrent_shared_future.cpp",
        "test/common/test_buffer.cpp",
        "test/common/test_listview.cpp",
        "test/common/test_miscellaneous.cpp",
        "test/common/test_msgpack.cpp",
        "test/common/test_vector3D.cpp",
    ],
    deps = [
        ":test_common",
        ":client"
    ],
)

cc_gtest_test(
    name = "test_opendrive",
    srcs = [
        "test/client/OpenDrive.cpp",
        "test/client/OpenDrive.h",
        "test/client/test_opendrive.cpp",
    ],
    copts = [
        "-DLIBCARLA_TEST_CONTENT_FOLDER=\\\"external/xodr_files\\\""
    ],
    data = ["@xodr_files"],
    deps = [
        ":test_common",
        ":filesystem",
        ":opendrive",
        ":thread",
    ],
)

cc_gtest_test(
    name = "test_rpc",
    srcs = ["test/client/test_rpc.cpp"],
    deps = [
        ":test_common",
        ":rpc",
        ":thread",
    ],
)

cc_gtest_test(
    name = "test_streaming",
    srcs = ["test/common/test_streaming.cpp"],
    deps = [
        ":test_common",
        ":streaming"
    ],
)

cc_gtest_test(
    name = "test_benchmark_streaming",
    srcs = ["test/server/test_benchmark_streaming.cpp"],
    deps = [
        ":test_common",
        ":streaming"
    ],
)
