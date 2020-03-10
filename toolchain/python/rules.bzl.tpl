load("@//Util/Bazel:new_file.bzl", "new_file")

# List of strings, two-number semantic version, e.g. ["2.7", "3.6"]
PYTHON_VERSIONS = %{python_all_versions}

# ==============================================================================
# -- _py_wheel_and_egg rule ----------------------------------------------------
# ==============================================================================

_SETUP_PY="""
import os
import shutil

from setuptools import setup, Distribution, find_packages

package_data = []

os.makedirs('{lib_folder}')
for src in {data}:
    dst = os.path.join('{lib_folder}', os.path.basename(src))
    shutil.copyfile(src, dst)
    package_data.append(dst)

class BinaryDistribution(Distribution):
    def has_ext_modules(foo):
        return True

setup(
    name='{name}',
    version='{version}',
    license='{license}',
    description='{description}',
    url='{url}',
    author='{author}',
    author_email='{author_email}',
    packages=find_packages(),
    include_package_data=True,
    package_data={{'': package_data}},
    distclass=BinaryDistribution,
    zip_safe=True,
)

shutil.move('dist/{wheel}', '{outpath}/{wheel}')
shutil.move('dist/{egg}', '{outpath}/{egg}')
"""


def _py_wheel_and_egg_impl(ctx):
    py_version = ctx.attr.py_version
    py_interpreter = "{}".format(ctx.file.py_interpreter.path)
    # py_interpreter = ctx.expand_location("$(location {})".format(ctx.attr.py_interpreter))

    print("Version {} at {}".format(py_version, py_interpreter))

    if py_version.startswith("2"):
        lib_folder = "build/lib.linux-x86_64-{}".format(py_version)
        v_append = "mu" # @todo this may vary depending on installation?
    else:
        lib_folder = "build/lib"
        v_append = "m" # @todo this may vary depending on installation?

    wheel_name = "{name}-{v}-cp{pv}-cp{pv}{ap}-linux_x86_64.whl".format(
        name=ctx.attr.package_name,
        v=ctx.attr.version,
        pv=py_version.replace(".", ""),
        ap=v_append)

    egg_name = "{name}-{v}-py{pv}-linux-x86_64.egg".format(
        name=ctx.attr.package_name,
        v=ctx.attr.version,
        pv=py_version)

    setup = ctx.actions.declare_file("setup-py{}.py".format(py_version))
    wheel = ctx.actions.declare_file(wheel_name)
    egg = ctx.actions.declare_file(egg_name)
    log = ctx.actions.declare_file("setup-py{}.log".format(py_version))

    setup_content = _SETUP_PY.format(
        data=str([f.path for f in ctx.files.data]),
        lib_folder=lib_folder,
        name=ctx.attr.package_name,
        version=ctx.attr.version,
        license=ctx.attr.license,
        description=ctx.attr.description,
        url=ctx.attr.url,
        author=ctx.attr.author,
        author_email=ctx.attr.author_email,
        wheel=wheel_name,
        egg=egg_name,
        outpath=wheel.dirname,
    )

    ctx.actions.write(output=setup, content=setup_content)

    cmd = "{py} {src} bdist_wheel bdist_egg > {out}"

    ctx.actions.run_shell(
        inputs = [setup] + ctx.files.data,
        tools = ctx.files.py_interpreter,
        outputs = [wheel, egg, log],
        progress_message = "{} wheels and eggs".format(ctx.attr.package_name),
        command = cmd.format(py=py_interpreter, src=setup.path, out=log.path),
    )

    return [DefaultInfo(files = depset([wheel, egg, log]))]


_py_wheel_and_egg = rule(
    implementation=_py_wheel_and_egg_impl,
    attrs = {
        "data": attr.label_list(mandatory=True, allow_files=True),
        "package_name": attr.string(mandatory=True),
        "version": attr.string(mandatory=True),
        "license": attr.string(mandatory=True),
        "description": attr.string(mandatory=True),
        "url": attr.string(mandatory=True),
        "author": attr.string(mandatory=True),
        "author_email": attr.string(mandatory=True),
        "py_version": attr.string(mandatory=True),
        "py_interpreter": attr.label(
            mandatory=True,
            allow_single_file=True,
            executable=True,
            cfg="target"),
    },
)

# ==============================================================================
# -- Macros --------------------------------------------------------------------
# ==============================================================================


def _replace_version(py_version, name, deps=[], data=[], **kwargs):
    kw = dict(kwargs)
    kw["name"] = name.replace("%{py_version}", py_version)
    if deps:
        kw["deps"] = [x.replace("%{py_version}", py_version) for x in deps]
    if data:
        kw["data"] = [x.replace("%{py_version}", py_version) for x in data]
    return kw


def for_each_py_version(rule, **kwargs):
    """
    Produces the given rule for each Python version available, replaces
    occurrences of %{py_version} by the Python version number.

    @warning Only the fields 'name', 'deps', and 'data' are replaced.
    """
    for version in PYTHON_VERSIONS:
        rule(**_replace_version(version, **kwargs))


def py_wheels_and_eggs(name, **kwargs):
    for version in PYTHON_VERSIONS:
        kw = _replace_version(version, name + "-py%{py_version}", **kwargs)
        _py_wheel_and_egg(
            py_version = version,
            py_interpreter = "@python//python{}:interpreter".format(version),
            **kw,
        )
    native.filegroup(
        name = name,
        srcs = [":{}-py{}".format(name, v) for v in PYTHON_VERSIONS]
    )
