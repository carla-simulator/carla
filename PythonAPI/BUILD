load("@python//:rules.bzl", "py_wheels_and_eggs")

py_wheels_and_eggs(
    name = "dist",
    data = ["//PythonAPI/carla/source:carla-py%{py_version}.so"],
    package_name = "carla",
    version = "0.9.8",
    license = "MIT License",
    description = "Python API for communicating with the CARLA server.",
    url = "https://github.com/carla-simulator/carla",
    author = "The CARLA team",
    author_email = "carla.simulator@gmail.com",
)

test_suite(
    name = "unit_test",
    tests = [
        "//PythonAPI/test:unit",
    ],
)

test_suite(
    name = "smoke_test",
    tags = ["manual"],
    tests = [
        "//PythonAPI/test:smoke",
    ],
)
