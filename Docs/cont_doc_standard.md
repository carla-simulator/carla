# Documentation standard

House rules for the pages under `Docs/`. They are short on purpose: the deliverable is the page a reader lands on, not the process that produced it.

*   [__What a page is for__](#what-a-page-is-for)
*   [__Shape__](#shape)
*   [__Markdown__](#markdown)
*   [__Correctness__](#correctness)
*   [__Placement__](#placement)
*   [__Generated pages__](#generated-pages)
*   [__Building the docs__](#building-the-docs)

---
## What a page is for

Write for someone trying to do something, not for someone auditing the codebase.

*   __Lead with the result, not the mechanism.__ "Multi-camera rigs are about 30% faster" is the point; "readbacks are pooled into one GPU sync per tick" is the reason, and goes after it, in one clause.
*   __Say what the reader has to do.__ Renames, deprecations and behaviour changes get the action: "the old name still works and raises a `DeprecationWarning`", "set it again after loading a map".
*   __Cut anything that argues for the change.__ Justification is PR material.
*   __Do not document what has no user-visible effect.__ Internal plumbing with no API, no CLI flag and no behaviour change does not belong on a page.

---
## Shape

*   __Precise and concise.__ No filler, no restating the obvious, no sentence that does not earn its place.
*   __The sections a reader needs, in the order they need them.__ A feature usually wants: what it is, the code to use it, the table of values or parameters, then the caveat.
*   __Use the element that carries the meaning.__ A table for values, attributes or parameter lists. A diagram for a hierarchy or a pipeline. A code block for anything the reader will copy. Prose for the rest, and only the rest.
*   __One admonition at most per section.__ `!!! Note` for a caveat, `!!! Warning` for something that will break their code.

---
## Markdown

*   __`# Title` with a space__ at the top of the page, then `##` and `###` for the sections. The navigation bar is built from them.
*   __A bullet list of the page's sections__ under the intro, and a `---` separator between sections.
*   __Never hard-wrap prose.__ A paragraph or a bullet is one line, however long. The wrap is invisible when rendered and makes the source and the diff harder to read.
*   __Fence every code block with its language__: ```` ```py ````, ```` ```sh ````, ```` ```cpp ````.
*   __Markdown, not HTML.__ No `<br>`, `<font>` or `<h1>`; the only HTML left is the image captions and button blocks the theme needs.
*   __Match the page you are editing__ for heading depth, table style and admonition casing.
*   Images go in `Docs/img/` and are referenced as `img/<name>`. Styling lives in [`extra.css`](https://github.com/carla-simulator/carla/blob/ue58-dev/Docs/extra.css).

---
## Correctness

*   __Verify every name against the bindings__, not against the reference. `PythonAPI/carla/src/*.cpp` is the truth; the YAML can be out of date, and the reference is generated from the YAML.
*   __Run the snippets you write__, or at least check the attributes exist on a live client. Examples that fail when pasted are worse than no examples.
*   __Take the values from the data.__ Blueprint attribute values, map lists and catalogue groups come from the JSON in the content repo, not from memory.
*   __Restore the world in a snippet that changes it.__ A tutorial that leaves the server in synchronous mode breaks the next script the reader runs.

---
## Placement

*   __Put it where a reader would look for it__, which is almost always an existing page. A new page is for a feature that has no home at all.
*   __Check it is reachable.__ A page that is not in `mkdocs.yml` and not linked from a page that is cannot be found by anyone.
*   __Link to the API reference by anchor__: `[carla.Lane](python_api.md#carla.Lane)`. Build the docs and check the anchor exists.
*   __Fix the surrounding text when the feature changed it.__ If a method was renamed, update the places that use the old name instead of adding a note beside them.

---
## Generated pages

Two pages are generated and must never be edited directly.

| Page | Source | Regenerate with |
| --- | --- | --- |
| `Docs/python_api.md` | `PythonAPI/docs/*.yml` | `python3 PythonAPI/docs/doc_gen.py` |
| `Docs/bp_library.md` | the blueprints of a running server | `python3 PythonAPI/docs/bp_doc_gen.py` |

`bp_doc_gen.py` needs a CARLA server running; pass `--host` and `--port` if it is not on `127.0.0.1:2000`.

---
## Building the docs

```sh
pip3 install mkdocs
python3 -m mkdocs serve
```

The docs are then at `http://127.0.0.1:8000`. `python3 -m mkdocs build --strict` fails on a broken internal link, which is the check to run before opening a pull request.
