load("@//Util/Bazel:new_file.bzl", "new_file")

# Dictionary containing info about all available Python version, keys are
# formatted as two-number semantic version, e.g. ["2.7", "3.6"]
PYTHON_VERSIONS = %{python_all_versions_info}


# ==============================================================================
# -- _py_wheel_and_egg rule ----------------------------------------------------
# ==============================================================================


def _py_wheel_and_egg_impl(ctx):
    py_version = ctx.attr.py_version

    wheel_name = "{name}-{v}-{suffix}".format(
        name=ctx.attr.package_name,
        v=ctx.attr.version,
        suffix=PYTHON_VERSIONS[py_version]["wheel_suffix"])

    egg_name = "{name}-{v}-{suffix}".format(
        name=ctx.attr.package_name,
        v=ctx.attr.version,
        suffix=PYTHON_VERSIONS[py_version]["egg_suffix"])

    setup = ctx.actions.declare_file("setup-py{}.py".format(py_version))
    wheel = ctx.actions.declare_file(wheel_name)
    egg = ctx.actions.declare_file(egg_name)
    log = ctx.actions.declare_file("setup-py{}.log".format(py_version))

    substitutions = {
        "%{py_version}": py_version,
        "%{data}": str([f.path for f in ctx.files.data]),
        "%{name}": ctx.attr.package_name,
        "%{version}": ctx.attr.version,
        "%{license}": ctx.attr.license,
        "%{description}": ctx.attr.description,
        "%{url}": ctx.attr.url,
        "%{author}": ctx.attr.author,
        "%{author_email}": ctx.attr.author_email,
        "%{wheel}": wheel_name,
        "%{egg}": egg_name,
        "%{outpath}": wheel.dirname,
    }

    ctx.actions.expand_template(
        template=ctx.file._setup_template,
        output=setup,
        substitutions=substitutions)

    cmd = "{py} {src} bdist_wheel --universal bdist_egg > {out}"

    ctx.actions.run_shell(
        inputs = [setup] + ctx.files.data,
        tools = ctx.files.py_interpreter,
        outputs = [wheel, egg, log],
        progress_message = "{} wheels and eggs".format(ctx.attr.package_name),
        command = cmd.format(py=ctx.file.py_interpreter.path, src=setup.path, out=log.path),
    )

    return [DefaultInfo(files = depset([wheel, egg, log]))]


_py_wheel_and_egg = rule(
    implementation = _py_wheel_and_egg_impl,
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
        "_setup_template": attr.label(
            default = Label("@//toolchain/python:scripts/setup-tpl.py"),
            allow_single_file = True),
    },
)


# ==============================================================================
# -- _py_nose2_test rule -------------------------------------------------------
# ==============================================================================


_UNITTEST_CFG="""
[unittest]
plugins = nose2.plugins.junitxml
[junit-xml]
path = test.xml
"""


_NOSE2_SCRIPT_SH="""
# @todo Hacky, hacky... find a better way.
SO_FILES=$(find . -iname '*-py{py_version}.so')
for i in $SO_FILES ; do mv -v "$i" "${{i/-py*.so/.so}}" ; done

{py} --version
export PYTHONPATH={pythonpath}
{py} -m nose2 -s {folder} --config={cfg} -X
"""


def _py_nose2_test_impl(ctx):
    py_version = ctx.attr.py_version

    sh = ctx.actions.declare_file(ctx.attr.name + ".sh")
    cfg = ctx.actions.declare_file("unittest.cfg")
    ctx.actions.write(output=cfg, content=_UNITTEST_CFG)

    pythonpath = ";".join([t.label.package for t in ctx.attr.deps])

    cmd = _NOSE2_SCRIPT_SH.format(
        py_version=py_version,
        py=ctx.file.py_interpreter_abspath.path,
        folder=ctx.file.import_folder.path,
        cfg=cfg.short_path,
        pythonpath=pythonpath)

    ctx.actions.write(output=sh, content=cmd)

    runfiles = ctx.runfiles(files = depset(
        ctx.files.srcs +
        ctx.files.data +
        ctx.files.deps +
        ctx.files.py_interpreter_abspath +
        [cfg]).to_list())

    return [DefaultInfo(
        executable = sh,
        runfiles = runfiles,
        files = depset([cfg, ctx.file.py_interpreter_abspath]))]


_py_nose2_test = rule(
    implementation = _py_nose2_test_impl,
    test = True,
    attrs = {
        "import_folder": attr.label(mandatory=True, allow_single_file=True),
        "srcs": attr.label_list(mandatory=True, allow_files=True),
        "deps": attr.label_list(mandatory=True, allow_files=True),
        "data": attr.label_list(mandatory=False, allow_files=True),
        "py_version": attr.string(mandatory=True),
        "py_interpreter_abspath": attr.label(
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
    for version in PYTHON_VERSIONS.keys():
        rule(**_replace_version(version, **kwargs))


def py_wheels_and_eggs(name, **kwargs):
    for version in PYTHON_VERSIONS.keys():
        kw = _replace_version(version, name + "-py%{py_version}", **kwargs)
        _py_wheel_and_egg(
            py_version = version,
            py_interpreter = "@python//python{}:interpreter".format(version),
            **kw,
        )
    native.filegroup(
        name = name,
        srcs = [":{}-py{}".format(name, v) for v in PYTHON_VERSIONS.keys()],
    )


def py_nose2_test(name, **kwargs):
    kwargs["tags"] = kwargs.get("tags", []) + ["python", "nose2"]
    kwargs["size"] = kwargs.get("size", "small")
    for version in PYTHON_VERSIONS.keys():
        kw = _replace_version(version, name + "-py%{py_version}", **kwargs)
        _py_nose2_test(
            py_version = version,
            py_interpreter_abspath = "@python//python{}:interpreter".format(version),
            **kw,
        )
    native.test_suite(
        name = name,
        tags = kwargs["tags"],
        tests = [":{}-py{}".format(name, v) for v in PYTHON_VERSIONS.keys()]
    )
