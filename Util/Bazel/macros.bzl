load("@//Util/Bazel:new_file.bzl", "new_file")
load("@python//:defs.bzl", "PYTHON_VERSION_FLAG")


def cc_gtest_library(**kwargs):
    kwargs["defines"] = [
        "LIBCARLA_WITH_GTEST",
        "LIBCARLA_ENABLE_PROFILER"
    ] + kwargs.get("defines", [])
    kwargs["deps"] = kwargs.get("deps", []) + ["@gtest//:gtest"]
    kwargs["testonly"] = True
    native.cc_library(**kwargs)


def cc_gtest_test(**kwargs):
    kwargs["defines"] = [
        "LIBCARLA_WITH_GTEST",
        "LIBCARLA_ENABLE_PROFILER"
    ] + kwargs.get("defines", [])
    kwargs["tags"] = kwargs.get("tags", []) + ["cpp", "gtest"]
    kwargs["deps"] = kwargs.get("deps", []) + ["@gtest//:gtest_main"]
    kwargs["args"] = [
        "--gtest_color=true",
        "--gtest_shuffle",
    ] + kwargs.get("args", [])
    kwargs["size"] = kwargs.get("size", "small")
    native.cc_test(**kwargs)


def py_nose2_test(**kwargs):
    if "main" in kwargs:
        fail("py_nose2_test does not accept 'main' attribute.")
    main = "{}_main".format(kwargs["name"])
    new_file(
        name = main,
        out = "__nose2_main__.py",
        content = "import nose2; nose2.main(module=__package__)",
    )
    kwargs["main"] = "__nose2_main__.py"
    kwargs["tags"] = kwargs.get("tags", []) + ["python", "nose2"]
    kwargs["srcs"] = kwargs.get("srcs", []) + ["__nose2_main__.py"]
    kwargs["data"] = kwargs.get("data", []) + [(":" + main)]
    kwargs["size"] = kwargs.get("size", "small")
    kwargs["python_version"] = kwargs.get("python_version", PYTHON_VERSION_FLAG)
    native.py_test(**kwargs)
