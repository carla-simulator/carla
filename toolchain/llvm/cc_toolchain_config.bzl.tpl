load(
    "@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl",
    "feature",
    "feature_set",
    "flag_group",
    "flag_set",
    "tool_path",
    "with_feature_set",
)

load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")

def _impl(ctx):
    toolchain_identifier = "%{toolchain_name}"
    host_system_name = "%{host_system_name}"
    target_system_name = "%{target_system_name}"
    target_cpu = "%{target_cpu}"
    target_libc = "glibc_unknown"
    compiler = "clang"
    abi_version = "clang"
    abi_libc_version = "glibc_unknown"
    cc_target_os = None

    if (ctx.attr.cpu != target_cpu):
        fail("Target CPU '{}' not supported ('{}' only)".format(
            ctx.attr.cpu,
            target_cpu))

    all_compile_actions = [
        ACTION_NAMES.c_compile,
        ACTION_NAMES.cpp_compile,
        ACTION_NAMES.linkstamp_compile,
        ACTION_NAMES.assemble,
        ACTION_NAMES.preprocess_assemble,
        ACTION_NAMES.cpp_header_parsing,
        ACTION_NAMES.cpp_module_compile,
        ACTION_NAMES.cpp_module_codegen,
        ACTION_NAMES.clif_match,
        ACTION_NAMES.lto_backend,
    ]

    all_cpp_compile_actions = [
        ACTION_NAMES.cpp_compile,
        ACTION_NAMES.linkstamp_compile,
        ACTION_NAMES.cpp_header_parsing,
        ACTION_NAMES.cpp_module_compile,
        ACTION_NAMES.cpp_module_codegen,
        ACTION_NAMES.clif_match,
    ]

    preprocessor_compile_actions = [
        ACTION_NAMES.c_compile,
        ACTION_NAMES.cpp_compile,
        ACTION_NAMES.linkstamp_compile,
        ACTION_NAMES.preprocess_assemble,
        ACTION_NAMES.cpp_header_parsing,
        ACTION_NAMES.cpp_module_compile,
        ACTION_NAMES.clif_match,
    ]

    codegen_compile_actions = [
        ACTION_NAMES.c_compile,
        ACTION_NAMES.cpp_compile,
        ACTION_NAMES.linkstamp_compile,
        ACTION_NAMES.assemble,
        ACTION_NAMES.preprocess_assemble,
        ACTION_NAMES.cpp_module_codegen,
        ACTION_NAMES.lto_backend,
    ]

    all_link_actions = [
        ACTION_NAMES.cpp_link_executable,
        ACTION_NAMES.cpp_link_dynamic_library,
        ACTION_NAMES.cpp_link_nodeps_dynamic_library,
    ]

    action_configs = []

    cxx_includes = [
        "%{llvm_path_prefix}/include",
        "%{llvm_path_prefix}/include/c++/v1",
        "/usr/include",
    ]

    cxx_flags_cpp = [
        "-std=c++17",
        "-stdlib=libc++",
    ]

    cxx_flags_default = [
        "-nostdinc++",
        "-fPIC",

        "-Werror",
        "-Wall",
        "-Wpedantic",
        "-Wthread-safety",
        "-Wno-unused-command-line-argument",
        "-Wno-deprecated-declarations",

        "-U_FORTIFY_SOURCE",
        "-fstack-protector",
        "-fno-omit-frame-pointer",

        "-fcolor-diagnostics",
    ]

    cxx_flags_extra_warnings = [
        "-Wconversion",
        "-Wdeprecated",
        "-Wdeprecated-declarations",
        "-Wduplicate-enum",
        "-Wfloat-overflow-conversion",
        "-Wheader-hygiene",
        "-Wnon-virtual-dtor",
        "-Wnull-dereference",
        "-Wold-style-cast",
        "-Wpessimizing-move",
        "-Wself-assign",
        "-Wshadow",
        "-Wuninitialized",
        "-Wunreachable-code",
    ]

    cxx_flags_no_exceptions = ["-fno-exceptions"]

    cxx_flags_no_rtti = ["-fno-rtti"]

    cxx_flags_dbg = [
        "-D_DEBUG",
        "-O0",
        "-ggdb",
        "-fstandalone-debug",
    ]

    cxx_flags_opt = [
        "-DNDEBUG",
        "-O3",
        "-D_FORTIFY_SOURCE=1",
        "-ffunction-sections",
        "-fdata-sections",
    ]

    cxx_flags_unfiltered = [
        "-no-canonical-prefixes",
        "-Wno-builtin-macro-redefined",
        "-D__DATE__=\"redacted\"",
        "-D__TIMESTAMP__=\"redacted\"",
        "-D__TIME__=\"redacted\"",
    ]

    linker_flags = [
        "-fPIC",
        "-fuse-ld=lld",

        "-no-canonical-prefixes",

        "-stdlib=libc++",
        "-Lexternal/%{package_name}/%{llvm_path_prefix}/lib",
        "-l:libc++.a",
        "-l:libc++abi.a",

        "-lpthread",
        "-ldl",
        "-lm",
    ]

    linker_flags_opt = ["-Wl,--gc-sections"]

    for include in cxx_includes:
        if not include.startswith("/"):
            include = "external/%{package_name}/" + include
        cxx_flags_default.append("-isystem")
        cxx_flags_default.append(include)

    opt_feature = feature(name = "opt")
    fastbuild_feature = feature(name = "fastbuild")
    dbg_feature = feature(name = "dbg")

    extra_warnings_feature = feature(name = "extra-warnings")
    no_exceptions_feature = feature(name = "no-exceptions")
    no_rtti_feature = feature(name = "no-rtti")

    supports_pic_feature = feature(name = "supports_pic", enabled = True)
    supports_dynamic_linker_feature = feature(name = "supports_dynamic_linker", enabled = True)

    default_compile_flags_feature = feature(
        name = "default_compile_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = all_cpp_compile_actions,
                flag_groups = [flag_group(flags = cxx_flags_cpp)],
            ),
            flag_set(
                actions = all_compile_actions,
                flag_groups = [flag_group(flags = cxx_flags_default)],
            ),
            flag_set(
                actions = all_cpp_compile_actions,
                with_features = [with_feature_set(features = ["extra-warnings"])],
                flag_groups = [flag_group(flags = cxx_flags_extra_warnings)],
            ),
            flag_set(
                actions = all_compile_actions,
                with_features = [with_feature_set(features = ["dbg"])],
                flag_groups = [flag_group(flags = cxx_flags_dbg)],
            ),
            flag_set(
                actions = all_compile_actions,
                with_features = [with_feature_set(features = ["opt"])],
                flag_groups = [flag_group(flags = cxx_flags_opt)],
            ),
            flag_set(
                actions = all_cpp_compile_actions,
                with_features = [with_feature_set(features = ["no-exceptions"])],
                flag_groups = [flag_group(flags = cxx_flags_no_exceptions)],
            ),
            flag_set(
                actions = all_cpp_compile_actions,
                with_features = [with_feature_set(features = ["no-rtti"])],
                flag_groups = [flag_group(flags = cxx_flags_no_rtti)],
            ),
        ],
    )

    unfiltered_compile_flags_feature = feature(
        name = "unfiltered_compile_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = all_compile_actions,
                flag_groups = [flag_group(flags = cxx_flags_unfiltered)],
            ),
        ],
    )

    default_link_flags_feature = feature(
        name = "default_link_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = all_link_actions,
                flag_groups = [flag_group(flags = linker_flags)],
            ),
        ] + [
            flag_set(
                actions = all_link_actions,
                flag_groups = [flag_group(flags = linker_flags_opt)],
                with_features = [with_feature_set(features = ["opt"])],
            ),
        ]
    )

    objcopy_embed_flags_feature = feature(
        name = "objcopy_embed_flags",
        enabled = True,
        flag_sets = [
            flag_set(
                actions = ["objcopy_embed_data"],
                flag_groups = [flag_group(flags = ["-I", "binary"])],
            ),
        ],
    )

    coverage_feature = feature(
        name = "coverage",
        flag_sets = [
            flag_set(
                actions = all_compile_actions,
                flag_groups = [
                    flag_group(
                        flags = ["-fprofile-instr-generate", "-fcoverage-mapping"],
                    ),
                ],
            ),
            flag_set(
                actions = all_link_actions,
                flag_groups = [flag_group(flags = ["-fprofile-instr-generate"])],
            ),
        ],
        provides = ["profile"],
    )

    features = [
        opt_feature,
        fastbuild_feature,
        dbg_feature,
        extra_warnings_feature,
        no_exceptions_feature,
        no_rtti_feature,
        supports_pic_feature,
        supports_dynamic_linker_feature,
        unfiltered_compile_flags_feature,
        default_link_flags_feature,
        default_compile_flags_feature,
        objcopy_embed_flags_feature,
        coverage_feature,
    ]

    tool_paths = [
        tool_path(
            name = "ld",
            path = "%{llvm_tools_path_prefix}/bin/ld.lld",
        ),
        tool_path(
            name = "cpp",
            path = "%{llvm_tools_path_prefix}/bin/clang++",
        ),
        tool_path(
            name = "dwp",
            path = "%{llvm_tools_path_prefix}/bin/llvm-dwp",
        ),
        tool_path(
            name = "gcov",
            path = "%{llvm_tools_path_prefix}/bin/llvm-profdata",
        ),
        tool_path(
            name = "nm",
            path = "%{llvm_tools_path_prefix}/bin/llvm-nm",
        ),
        tool_path(
            name = "objcopy",
            path = "%{llvm_tools_path_prefix}/bin/llvm-objcopy",
        ),
        tool_path(
            name = "objdump",
            path = "%{llvm_tools_path_prefix}/bin/llvm-objdump",
        ),
        tool_path(
            name = "strip",
            path = "%{llvm_tools_path_prefix}/bin/strip",
        ),
        tool_path(
            name = "gcc",
            path = "%{llvm_tools_path_prefix}/bin/clang",
        ),
        tool_path(
            name = "g++",
            path = "%{llvm_tools_path_prefix}/bin/clang++",
        ),
        tool_path(
            name = "ar",
            path = "%{llvm_tools_path_prefix}/bin/llvm-ar",
        ),
    ]

    out = ctx.actions.declare_file(ctx.label.name)
    ctx.actions.write(out, "Fake executable")
    return [
        cc_common.create_cc_toolchain_config_info(
            ctx = ctx,
            features = features,
            action_configs = action_configs,
            artifact_name_patterns = [],
            cxx_builtin_include_directories = cxx_includes,
            toolchain_identifier = toolchain_identifier,
            host_system_name = host_system_name,
            target_system_name = target_system_name,
            target_cpu = target_cpu,
            target_libc = target_libc,
            compiler = compiler,
            abi_version = abi_version,
            abi_libc_version = abi_libc_version,
            tool_paths = tool_paths,
            make_variables = [],
            cc_target_os = cc_target_os,
        ),
        DefaultInfo(executable = out),
    ]

cc_toolchain_config = rule(
    attrs = {
        "cpu": attr.string(
            mandatory = True,
            values = ["%{target_cpu}"], # Only one supported.
        ),
    },
    executable = True,
    provides = [CcToolchainConfigInfo],
    implementation = _impl,
)
