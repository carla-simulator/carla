cc_library(
    name = "libcarla.so.includes",
    hdrs = glob(["libcarla/*.cpp"], exclude=["libcarla/libcarla.cpp"]),
    defines = ["LIBCARLA_WITH_PYTHON_SUPPORT"],
    deps = [
        "//LibCarla/source:client",
        "//LibCarla/source:python",
        "@boost//:python",
    ],
)

cc_binary(
    name = "carla.so",
    srcs = ["libcarla/libcarla.cpp"],
    copts = ["-Wno-self-assign-overloaded"],
    linkstatic = True,
    linkshared = True,
    defines = ["LIBCARLA_PYTHON_MODULE_NAME=carla"],
    linkopts = [
        "-ljpeg",
        "-lpng",
        "-ltiff",
    ],
    deps = [
        ":libcarla.so.includes",
    ],
)

py_library(
    name = "carla",
    data = [":carla.so"],
    imports = ["."],
    visibility = ["//visibility:public"],
)

genrule(
    name = "dist",
    srcs = ["carla.so"],
    outs = [
        "setup.py.log",
        "carla-0.9.7-cp36-cp36m-linux_x86_64.whl",
        "carla-0.9.7-py3.6-linux-x86_64.egg",
    ],
    cmd_bash = """
mkdir -p build/lib
cp -v $< build/lib/carla.so >> $(location setup.py.log)

cat >setup.py <<EOL
from setuptools import setup, Distribution, find_packages

class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True

setup(name='carla',
    version='0.9.7',
    license='MIT License',
    description='Python API for communicating with the CARLA server.',
    url='https://github.com/carla-simulator/carla',
    author='The CARLA team',
    author_email='carla.simulator@gmail.com',
    #package_dir={'': 'src'},
    packages=find_packages(),
    include_package_data=True,
    package_data={'': ['carla.so']},
    distclass=BinaryDistribution,
    zip_safe=True,
)
EOL

python3 setup.py bdist_wheel bdist_egg >> $(location setup.py.log)

cp -v dist/* $$(dirname $(location setup.py.log)) >> $(location setup.py.log)
""",
    cmd_bat = "", # @todo
)
