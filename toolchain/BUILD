package(default_visibility = ["//visibility:public"])

load("@rules_cc//cc:defs.bzl", "cc_toolchain_suite")

filegroup(
    name = "empty",
    srcs = [],
)

cc_toolchain_suite(
    name = "toolchain",
    toolchains = {
        "k8|clang": ":cc-clang-linux",
        "k8": ":cc-clang-linux",
    },
)

load(":cc_toolchain_config.bzl", "cc_toolchain_config")

cc_toolchain_config(
    name = "local_linux",
    cpu = "k8",
)

toolchain(
    name = "cc-toolchain-linux",
    exec_compatible_with = [
        "@platforms//cpu:x86_64",
        "@platforms//os:linux",
    ],
    target_compatible_with = [
        "@platforms//cpu:x86_64",
        "@platforms//os:linux",
    ],
    toolchain = ":cc-clang-linux",
    toolchain_type = "@bazel_tools//tools/cpp:toolchain_type",
)

cc_toolchain(
    name = "cc-clang-linux",
    toolchain_identifier = "cc-clang-linux-toolchain",
    toolchain_config = ":local_linux",
    all_files = ":empty",
    compiler_files = ":empty",
    dwp_files = ":empty",
    linker_files = ":empty",
    objcopy_files = ":empty",
    strip_files = ":empty",
)
