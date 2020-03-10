workspace(name = "CARLA")

load("@//toolchain/ue4:config.bzl", "unreal_engine_repository")
load("@//toolchain/python:config.bzl", "find_local_python_repositories")

# Determines the location based on UE4_ROOT environment variable.
unreal_engine_repository(
    name = "local_unreal_engine",
    branch = "4.22",
)

# Adds a new local repository for each python-dev version installed.
find_local_python_repositories(
    name = "python",
    exclude_versions = [],
)

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "boost",
    build_file = "@//:third_party/boost.BUILD",
    strip_prefix = "boost_1_72_0",
    sha256 = "c66e88d5786f2ca4dbebb14e06b566fb642a1a6947ad8cc9091f9f445134143f",
    url = "https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz",
)

http_archive(
    name = "gtest",
    strip_prefix = "googletest-release-1.10.0",
    sha256 = "9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb",
    url = "https://github.com/google/googletest/archive/release-1.10.0.tar.gz",
)

http_archive(
    name = "recast",
    build_file = "@//:third_party/recast.BUILD",
    strip_prefix = "recastnavigation-cdce4e1a270fdf1f3942d4485954cc5e136df1df",
    sha256 = "26fcf840ab2c4b695fc5333ec02757fc7b3feb19fab23d360c9f928e745c789d",
    url = "https://github.com/carla-simulator/recastnavigation/archive/cdce4e1a270fdf1f3942d4485954cc5e136df1df.tar.gz",
)

http_archive(
    name = "rpclib",
    build_file = "@//:third_party/rpclib.BUILD",
    strip_prefix = "rpclib-2.2.1_c2",
    sha256 = "647ae9d6ad5e008974d2ec3a7ffeec0058bc457e88cc9d18bd75186ba191b8ea",
    url = "https://github.com/carla-simulator/rpclib/archive/v2.2.1_c2.tar.gz",
)

http_archive(
    name = "rules_foreign_cc",
    strip_prefix = "rules_foreign_cc-8b477ca9cb248fc472f152aa1a44c55ab71c4636",
    sha256 = "c7cd62dc965ee3ee4f513fb6ce1d8d0d3dfcbb6ed5baa138bbea62d9dd5653fd",
    url = "https://github.com/bazelbuild/rules_foreign_cc/archive/8b477ca9cb248fc472f152aa1a44c55ab71c4636.tar.gz",
)

http_archive(
    name = "xodr_files",
    build_file = "@//:third_party/xodr_files.BUILD",
    strip_prefix = "opendrive-test-files-0.1.4",
    sha256 = "7de92865a9d945e802e54850302fc797148e24536f3d6410b0032bd608ca24b8",
    url = "https://github.com/carla-simulator/opendrive-test-files/archive/0.1.4.tar.gz",
)

load("@rules_foreign_cc//:workspace_definitions.bzl", "rules_foreign_cc_dependencies")

rules_foreign_cc_dependencies()
