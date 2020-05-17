load("@python//:rules.bzl", "py_nose2_test")

py_nose2_test(
    name = "unit",
    srcs = glob(["unit/*.py"]),
    import_folder = ".",
    deps = ["//PythonAPI/carla/source:carla-py%{py_version}.so"],
)

py_nose2_test(
    name = "smoke",
    srcs = glob(["smoke/*.py"]),
    import_folder = ".",
    size = "medium",
    tags = [
        "exclusive",
        "manual",
    ],
    deps = ["//PythonAPI/carla/source:carla-py%{py_version}.so"],
)
