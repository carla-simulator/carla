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
