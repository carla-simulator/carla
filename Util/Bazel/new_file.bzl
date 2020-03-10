def _impl(ctx):
    content = ctx.attr.content
    for key, value in ctx.attr.substitutions.items():
        content = content.replace(key, value)
    out = ctx.actions.declare_file(ctx.attr.name)
    ctx.actions.write(
        output=out,
        content=content,
        is_executable=ctx.attr.is_executable,
    )
    return [DefaultInfo(files = depset([out]))]


new_file = rule(
    doc = """\
Creates a new file with the content provided after applying substitutions.
""",
    attrs = {
        "content": attr.string(mandatory=True),
        "substitutions": attr.string_dict(mandatory=False, default={}),
        "is_executable": attr.bool(mandatory=False, default=False),
    },
    implementation = _impl,
)
