# package(features = ["no-exceptions", "no-rtti"])

load("@rules_foreign_cc//tools/build_defs:boost_build.bzl", "boost_build")

# filegroup(
#     name = "all",
#     srcs = glob(["**"]),
#     visibility = ["//visibility:private"],
# )

cc_library(
    name = "headers",
    hdrs = glob([
        "boost/*.hpp",
        "boost/algorithm/**/**",
        "boost/asio/**/**",
        "boost/bind/**/**",
        "boost/concept/**/**",
        "boost/config/**/**",
        "boost/container/**/**",
        "boost/container_hash/**/**",
        "boost/core/**/**",
        "boost/date_time/**/**",
        "boost/detail/**/**",
        "boost/exception/**/**",
        "boost/function/**/**",
        "boost/function_types/**/**",
        "boost/functional/**/**",
        "boost/geometry/**/**",
        "boost/gil/**/**",
        "boost/integer/**/**",
        "boost/intrusive/**/**",
        "boost/io/**/**",
        "boost/iterator/**/**",
        "boost/lexical_cast/**/**",
        "boost/math/**/**",
        "boost/move/**/**",
        "boost/mp11/**/**",
        "boost/mpl/**/**",
        "boost/numeric/**/**",
        "boost/optional/**/**",
        "boost/predef/**/**",
        "boost/predef/**/**",
        "boost/preprocessor/**/**",
        "boost/qvm/**/**",
        "boost/random/**/**",
        "boost/range/**/**",
        "boost/smart_ptr/**/**",
        "boost/system/**/**",
        "boost/tuple/**/**",
        "boost/type_index/**/**",
        "boost/type_traits/**/**",
        "boost/utility/**/**",
        "boost/variant/**/**",
    ]),
    includes = ["."],
    copts = ["-DBOOST_ERROR_CODE_HEADER_ONLY"],
    visibility = ["//visibility:public"],
)

boost_build(
    name = "libs",
    lib_source = "@boost//:all",
    static_libraries = ["libboost_filesystem.a"],
    bootstrap_options = ["--with-libraries=filesystem,python"],
    user_options = ["cxxflags=-DBOOST_ERROR_CODE_HEADER_ONLY"],
    visibility = ["//visibility:public"],
    deps = [],
)

cc_library(
    name = "system",
    srcs = glob(["libs/system/src/**/**"]),
    hdrs = glob([
        "boost/system/**/**",
    ]),
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = [":headers"],
)

cc_library(
    name = "filesystem",
    srcs = glob(["libs/filesystem/src/**/**"]),
    hdrs = glob([
        "boost/filesystem/**/**",
    ]),
    includes = ["."],
    visibility = ["//visibility:public"],
    deps = [
        ":headers",
        ":system",
    ],
)

# cc_library(
#     name = "python",
#     copts = ["-DBOOST_ERROR_CODE_HEADER_ONLY"],
#     deps = [":libs"],
#     visibility = ["//visibility:public"],
# )
