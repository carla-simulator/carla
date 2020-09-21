# Documentation Standard

This document will serve as a guide and example of some rules that need to be followed in order to contribute to the documentation.

*   [__Docs structure__](#docs-structure)  
*   [__Rules__](#rules)  
*   [__Exceptions__](#exceptions)  

---
## Docs structure

We use a mix of markdown and HTML tags to customize the documentation along with an [`extra.css`](https://github.com/carla-simulator/carla/tree/master/Docs/extra.css) file.
To update Python API docs, instead of directly modifying the Markdown you need to edit the corresponding YAML files inside [`carla/PythonAPI/docs/`][fileslink] and run [`doc_gen.py`][scriptlink] or `make PythonAPI.docs`.  

This will re-generate the respective Markdown files inside `carla/Docs/`, which can then be fed into `mkdocs`.

[fileslink]: https://github.com/carla-simulator/carla/tree/master/PythonAPI/docs
[scriptlink]: https://github.com/carla-simulator/carla/blob/master/PythonAPI/docs/doc_gen.py

---
## Rules

*   Leave always an empty line between sections and at the end of the document.
*   Writting should not exceed `100` columns, except for HTML related content, markdown tables, code snipets and referenced links.
*   If an inline link exceeds the limit, use referenced `[name][reference_link]` markdown notation `[reference_link]: https://` rather than `[name](https://)`.
*   Use `<br>` to make inline jumps rather than leaving two spaces at the end of a line.
*   Use `<h1>Title</h1>` at the beggining of a new page in order to make a Title or `<hx>Heading<hx>` to make a heading that **won't show** on the navigation bar.
*   Use `------` underlining a Heading or `#` hierarchy to make headings and show them in the navigation bar.

---
## Exceptions

  * Documentation generated via Python scripts like PythonAPI reference

Handy markdown [cheatsheet][cheatlink].

[cheatlink]: https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet
