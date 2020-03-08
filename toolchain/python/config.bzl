def _call_python_to_get_version(rctx, python):
    result = rctx.execute([python, "--version"])
    if result.return_code != 0:
        fail("{} not found!".format(python))
    return ".".join(result.stdout.strip().split(" ")[-1].split(".")[:2])


def _impl(rctx):
    python_version = rctx.os.environ.get("CARLA_PYTHON_VERSION")

    if not python_version:
        python_version = _call_python_to_get_version(rctx, "python3")

    python_version_flag = "PY{}".format(python_version.split(".")[0])
    python_version_flag_lowercase = "py{}".format(python_version.split(".")[0])
    python_interpreter = "/usr/bin/python{}".format(python_version)

    rctx.symlink(
        "/usr/include/python{}".format(python_version),
        "include")
    rctx.symlink(
        "/usr/bin/python{}".format(python_version),
        "bin/python")

    substitutions = {
        "%{package_name}": rctx.name,
        "%{python_version}": python_version,
        "%{python_version_flag}": python_version_flag,
        "%{python_version_flag_lowercase}": python_version_flag_lowercase,
        "%{python_interpreter_path}": python_interpreter,
    }

    rctx.template(
        "BUILD",
        Label("//toolchain/python:BUILD.bzl.tpl"),
        substitutions,
        executable=False,
    )
    rctx.template(
        "defs.bzl",
        Label("//toolchain/python:defs.bzl.tpl"),
        substitutions,
        executable=False,
    )
    rctx.template(
        "dist.bzl",
        Label("//toolchain/python:dist.bzl.tpl"),
        substitutions,
        executable=False,
    )


find_local_python_repository = repository_rule(
    implementation=_impl,
    local = True,
    environ = ["CARLA_PYTHON_VERSION"],
    attrs = {
    },
)
