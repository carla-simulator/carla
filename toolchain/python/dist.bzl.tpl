load("@//Util/Bazel:new_file.bzl", "new_file")

_SETUP_PY="""
import os
import shutil

from setuptools import setup, Distribution, find_packages

package_data = []

os.makedirs('{lib_folder}')
for src in {srcs}:
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


def _impl(ctx):
    if "%{python_version_flag}" == "PY2":
        lib_folder = "build/lib.linux-x86_64-%{python_version}"
        v_append = "mu"
    else:
        lib_folder = "build/lib"
        v_append = "m"

    wheel_name = "{name}-{v}-cp{pv}-cp{pv}{ap}-linux_x86_64.whl".format(
        name=ctx.attr.package_name,
        v=ctx.attr.version,
        pv="%{python_version}".replace(".", ""),
        ap=v_append)

    egg_name = "{name}-{v}-py{pv}-linux-x86_64.egg".format(
        name=ctx.attr.package_name,
        v=ctx.attr.version,
        pv="%{python_version}")

    setup = ctx.actions.declare_file("setup.py")
    wheel = ctx.actions.declare_file(wheel_name)
    egg = ctx.actions.declare_file(egg_name)
    log = ctx.actions.declare_file("setup.log")

    setup_content = _SETUP_PY.format(
        srcs=str([f.path for f in ctx.files.srcs]),
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

    cmd = "%{python_interpreter_path} {src} bdist_wheel bdist_egg > {out}"

    ctx.actions.run_shell(
        inputs = [setup] + ctx.files.srcs,
        outputs = [wheel, egg, log],
        progress_message = "{} wheels and eggs".format(ctx.attr.package_name),
        command = cmd.format(src=setup.path, out=log.path),
    )

    return [DefaultInfo(files = depset([wheel, egg, log]))]


py_wheel_and_egg = rule(
    implementation=_impl,
    attrs = {
        "srcs": attr.label_list(mandatory=True, allow_files=True),
        "package_name": attr.string(mandatory=True),
        "version": attr.string(mandatory=True),
        "license": attr.string(mandatory=True),
        "description": attr.string(mandatory=True),
        "url": attr.string(mandatory=True),
        "author": attr.string(mandatory=True),
        "author_email": attr.string(mandatory=True),
    },
)
