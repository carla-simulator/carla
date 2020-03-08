def make_python_cc_library(name, visibility=None):
    for version in ['3.8', '3.7', '3.6', '3.5']:
        headers = native.glob(["include/python{v}/*.h".format(v=version)])
        if headers:
            native.cc_library(
                name = name,
                srcs = ["lib/python{v}/config-{v}m-x86_64-linux-gnu/libpython{v}.so".format(v=version)],
                hdrs = native.glob(["include/python{v}/*.h".format(v=version)]),
                includes = ["include/python{v}".format(v=version)],
                visibility = visibility,
            )
            # print("Using Python {v}".format(v=version))
            return

    fail("Failed to find Python")
