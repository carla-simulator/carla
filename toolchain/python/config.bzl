def _find_pythons_linux(rctx):
    """
    Return a list of dictionaries. For instance:

    [
        {
            "version": "3.6",
            "include": "/usr/include/python3.6",
            "interpreter": "/usr/bin/python3.6",
        }
    ]

    @note: Version prefixes are ignored, 3.6{m,d,u}.
    """
    cmd = "import glob, re; " +                                        \
        "p = re.compile('[23]\\.[0-9]+'); " +                          \
        "print(' '.join(set(" +                                        \
            "p.search(path).group() " +                                \
            "for path in glob.glob('/usr/include/python*/Python.h')" + \
        ")))"
    result = rctx.execute(["python", "-c", cmd])
    if result.return_code == 0:
        return [{
            "version": version,
            "include": "/usr/include/python{}".format(version),
            "interpreter": "/usr/bin/python{}".format(version),
        } for version in result.stdout.strip().split(" ")]


def _get_wheel_suffix(rctx, python_interpreter):
    script = rctx.path(Label("//toolchain/python:scripts/print_wheel_suffix.py"))
    result = rctx.execute([python_interpreter, script])
    if result.return_code != 0:
        fail("failed to run {}".format(script))
    return result.stdout.strip()


def _impl(rctx):
    pythons = _find_pythons_linux(rctx) # @todo Linux only.
    pythons = [p for p in pythons if p["version"] not in rctx.attr.exclude_versions]
    if not pythons:
        fail("Failed to find Python installation (excluding = {}).".format(rctx.attr.exclude_versions))

    all_versions_info = {}

    for python in pythons:
        version = python["version"]
        version_flag = "PY{}".format(version.split(".")[0])
        interpreter = python["interpreter"]
        root = "python{}".format(version)

        all_versions_info[version] = {
            "wheel_suffix": _get_wheel_suffix(rctx, interpreter),
            "egg_suffix": "py{}-linux-x86_64.egg".format(version) # @todo Linux only.
        }

        rctx.symlink(
            python["include"],
            root + "/include")
        rctx.symlink(
            python["interpreter"],
            root + "/bin/python")

        substitutions = {
            "%{python_version}": version,
            "%{python_version_flag}": version_flag,
            "%{python_version_flag_lowercase}": version_flag.lower(),
            "%{python_interpreter_path}": interpreter,
        }

        rctx.template(
            root + "/BUILD",
            Label("//toolchain/python:BUILD.bzl.tpl"),
            substitutions,
            executable=False,
        )

    rctx.file("BUILD")
    rctx.template(
        "rules.bzl",
        Label("//toolchain/python:rules.bzl.tpl"),
        {"%{python_all_versions_info}": str(all_versions_info)},
        executable=False,
    )


find_local_python_repositories = repository_rule(
    implementation=_impl,
    local = True,
    environ = ["CARLA_PYTHON_VERSION"],
    attrs = {
        "exclude_versions": attr.string_list(mandatory=False),
    },
)
