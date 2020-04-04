# @todo this should check the commit instead of the branch.
def _ensure_branch(rctx, branch, path):
    result = rctx.execute(
        ["git", "rev-parse", "--abbrev-ref", "HEAD"],
        working_directory = path,
    )
    if result.return_code != 0:
        fail("{} - not a git repository!".format(path))
    b = result.stdout.strip()
    if b != branch:
        fail("{} - requires branch '{}', but found '{}'".format(path, branch, b))


def _configure_llvm_toolchain(rctx, ue4_root):
    llvm_root = "llvm-8.0.1"
    version = "v15_clang-8.0.1-centos7"
    version_number = "8.0.1"
    host = "Linux_x64"
    target = "x86_64-unknown-linux-gnu"

    libcxx_path = "{}/Engine/Source/ThirdParty/Linux/LibCxx".format(ue4_root)
    sdk_path = "{}/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/{}/{}/{}".format(
        ue4_root,
        host,
        version,
        target)

    # llvm
    #   ├── bin             -> sdk_path/bin
    #   ├── include         -> libcxx_path/include
    #   └── lib
    #       ├── clang       -> sdk_path/lib/clang
    #       ├── libc++.a    -> libcxx_path/lib/Linux/{target}/libc++.a
    #       └── libc++abi.a -> libcxx_path/lib/Linux/{target}/libc++abi.a

    rctx.symlink(
        sdk_path + "/bin",
        llvm_root + "/bin")
    rctx.symlink(
        libcxx_path + "/include",
        llvm_root + "/include")
    for obj in ["libc++.a", "libc++abi.a"]:
        rctx.symlink(
            libcxx_path + "/lib/Linux/{}/{}".format(target, obj),
            llvm_root + "/lib/" + obj)
    rctx.symlink(
        sdk_path + "/lib/clang",
        llvm_root + "/lib/clang")

    return {
        "%{package_name}": rctx.name,
        "%{toolchain_name}": "clang-7-linux",
        "%{host_system_name}": host,
        "%{target_system_name}": "x86_64-linux-gnu",
        "%{target_cpu}": "k8",
        "%{llvm_tools_path_prefix}": llvm_root,
        "%{llvm_path_prefix}": llvm_root,
        "%{llvm_version}": version,
        "%{llvm_version_number}": version_number,
    }


def _impl(rctx):
    ue4_root = rctx.os.environ["UE4_ROOT"]

    _ensure_branch(rctx, rctx.attr.branch, ue4_root)

    rctx.symlink(ue4_root, "ue4_root")

    llvm_substitutions = _configure_llvm_toolchain(rctx, "ue4_root")

    rctx.template(
        "BUILD",
        Label("//toolchain/llvm:BUILD.bzl.tpl"),
        llvm_substitutions,
        executable=False,
    )
    rctx.template(
        "cc_toolchain_config.bzl",
        Label("//toolchain/llvm:cc_toolchain_config.bzl.tpl"),
        llvm_substitutions,
        executable=False,
    )


unreal_engine_repository = repository_rule(
    implementation=_impl,
    local = True,
    environ = ["UE4_ROOT"],
    attrs = {
        "branch": attr.string(mandatory = True),
    },
)
