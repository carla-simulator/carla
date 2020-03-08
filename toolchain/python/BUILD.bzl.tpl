package(default_visibility = ["//visibility:public"])

load("@bazel_tools//tools/python:toolchain.bzl", "py_runtime_pair")

filegroup(
    name = "interpreter",
    srcs = ["bin/python"],
)

cc_library(
    name = "dev",
    hdrs = glob(["include/**/*.h"]),
    includes = ["include"],
)

py_runtime(
    name = "runtime",
    interpreter_path = "%{python_interpreter_path}",
    python_version = "%{python_version_flag}"
)

py_runtime_pair(
    name = "runtime_pair",
    %{python_version_flag_lowercase}_runtime = ":runtime",
)

toolchain(
    name = "toolchain",
    toolchain = ":runtime_pair",
    toolchain_type = "@bazel_tools//tools/python:toolchain_type",
)
