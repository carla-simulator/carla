def _impl(ctx):
    template = ctx.actions.declare_file(ctx.outputs.out.basename + ".tpl")
    ctx.actions.write(output = template, content = ctx.attr.content)
    ctx.actions.expand_template(
        template=template,
        output=ctx.outputs.out,
        substitutions=ctx.attr.substitutions,
        is_executable=ctx.attr.is_executable,
    )


new_file = rule(
    doc = """\
Creates a new file with the content provided after applying substitutions.
""",
    attrs = {
        "out": attr.string(mandatory=True),
        "content": attr.string(mandatory=True),
        "substitutions": attr.string_dict(mandatory=False, default={}),
        "is_executable": attr.bool(mandatory=False, default=False),
    },
    outputs = {"out": "%{out}"},
    implementation = _impl,
)
