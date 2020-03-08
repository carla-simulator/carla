cc_library(
    name = "headers",
    hdrs = glob([
        "boost/**/*.h",
        "boost/**/*.hpp",
        "boost/**/*.ipp",
    ]),
    includes = ["."],
    defines = ["BOOST_ERROR_CODE_HEADER_ONLY"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "system",
    srcs = glob(["libs/system/src/**/**"]),
    hdrs = glob(["boost/system/**/**"]),
    includes = ["."],
    deps = [":headers"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "filesystem",
    srcs = glob(["libs/filesystem/src/**/**"]),
    hdrs = glob(["boost/filesystem/**/**"]),
    includes = ["."],
    deps = [
        ":headers",
        ":system",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "python",
    srcs = glob(["libs/python/src/**/*.cpp"], exclude=["libs/python/src/numpy/**/*"]),
    hdrs = glob(["boost/python/**/**"]),
    includes = ["."],
    copts = [
        "-Wno-return-std-move",
        "-Wno-tautological-constant-out-of-range-compare",
    ],
    deps = [
        ":headers",
        "@python//:dev",
    ],
    visibility = ["//visibility:public"],
)
