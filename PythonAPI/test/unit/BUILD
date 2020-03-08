load("@//Util/Bazel:macros.bzl", "py_nose2_test")

py_nose2_test(
    name = "unit",
    srcs = glob(["*.py"]),
    imports = ["."],
    deps = ["//PythonAPI/carla/source:carla"],
)
