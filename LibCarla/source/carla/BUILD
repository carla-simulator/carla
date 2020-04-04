package(features = ["extra-warnings"])

load("@python//:rules.bzl", "for_each_py_version")

# ==============================================================================
# -- Public targets ------------------------------------------------------------
# ==============================================================================

# This library has to compile many folders together due to circular dependencies
# among them.
cc_library(
    name = "client",
    hdrs = glob([
        "Sockets.h",
        "client/**/*.h",
        "sensor/data/*.h",
        "trafficmanager/**/*.h",
    ]),
    srcs = glob([
        "client/**/*.cpp",
        "sensor/**/*.cpp",
        "trafficmanager/**/*.cpp",
    ]),
    includes = ["."],
    include_prefix = "carla/",
    visibility = ["//visibility:public"],
    deps = [
        "//:version",
        "//LibCarla/source/carla/geom",
        "//LibCarla/source/carla/nav",
        "//LibCarla/source/carla/opendrive",
        "//LibCarla/source/carla/road",
        "//LibCarla/source/carla/rpc",
        "//LibCarla/source/carla/streaming",
        ":sensor_s11n_headers",
        ":thread",
        ":utility",
    ],
)

# Separating this target we allow people to compile the client-side without
# the extra dependencies: boost_filesystem, png, and jpeg.
cc_library(
    name = "client_io",
    visibility = ["//visibility:public"],
    deps = [
        "//LibCarla/source/carla/image",
        "//LibCarla/source/carla/pointcloud",
        ":filesystem",
    ],
)

for_each_py_version(
    rule = cc_library,
    name = "python%{py_version}",
    hdrs = [
        "PythonUtil.h",
    ],
    includes = ["."],
    include_prefix = "carla/",
    # This file should not be compiled in LibCarla since it depends on having
    # Python enabled!
    visibility = ["//PythonAPI:__subpackages__"],
    deps = [
        "@boost//:python%{py_version}",
    ],
)

# ==============================================================================
# -- Private targets -----------------------------------------------------------
# ==============================================================================

cc_library(
    name = "utility",
    hdrs = [
        "Buffer.h",
        "BufferPool.h",
        "Debug.h",
        "Exception.h",
        "Functional.h",
        "Iterator.h",
        "ListView.h",
        "Logging.h",
        "Memory.h",
        "MoveHandler.h",
        "NonCopyable.h",
        "Platform.h",
        "StopWatch.h",
        "Time.h",
        "TypeTraits.h",
    ],
    srcs = [
        "Exception.cpp",
        "Buffer.cpp",
    ],
    includes = ["."],
    include_prefix = "carla/",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        "//LibCarla/source/third-party:moodycamel",
        "@boost//:headers",
    ],
)

cc_library(
    name = "thread",
    hdrs = [
        "AtomicList.h",
        "AtomicSharedPtr.h",
        "RecurrentSharedFuture.h",
        "ThreadGroup.h",
        "ThreadPool.h",
    ],
    includes = ["."],
    include_prefix = "carla/",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        "@boost//:headers",
    ],
)

cc_library(
    name = "string",
    hdrs = [
        "StringUtil.h",
    ],
    srcs = [
        "StringUtil.cpp",
    ],
    includes = ["."],
    include_prefix = "carla/",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        ":utility",
        "@boost//:headers",
    ],
)

cc_library(
    name = "msgpack",
    hdrs = [
        "MsgPack.h",
        "MsgPackAdaptors.h",
    ],
    includes = ["."],
    include_prefix = "carla/",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        ":utility",
        "@rpclib",
    ],
)

cc_library(
    name = "filesystem",
    hdrs = ["FileSystem.h"],
    srcs = ["FileSystem.cpp"],
    includes = ["."],
    include_prefix = "carla/",
    visibility = ["//LibCarla/source:__subpackages__"],
    deps = [
        ":string",
        ":utility",
        "@boost//:filesystem",
    ],
)

# Headers only, compatible with server build.
cc_library(
    name = "sensor_s11n_headers",
    hdrs = glob([
        "sensor/*.h",
        "sensor/s11n/*.h",
    ]),
    includes = ["."],
    include_prefix = "carla/",
    visibility = ["//LibCarla/source:__subpackages__"],
)
