load("@python//:rules.bzl", "py_nose2_test")

py_nose2_test(
    name = "unit",
    srcs = glob(["*.py"]),
    import_folder = ".",
    deps = ["//PythonAPI/carla/source:carla-py%{py_version}.so"],
)
