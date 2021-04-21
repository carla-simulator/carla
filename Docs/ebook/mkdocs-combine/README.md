# mkdocs-combine

**2018-06-05: Note that MkDocs now supports plugins that provide a better architecture for this task. I'll probably replace this project with a fork of [shauser's plugin](https://github.com/shauser/mkdocs-pdf-export-plugin)** — Adam

[**`mkdocs-combine`**](https://github.com/twardoch/mkdocs-combine/) is a Python module that combines a [MkDocs](http://www.mkdocs.org/)-style Markdown source site into a single Markdown document. This is useful for

* Generating PDF or EPUB from your MkDocs documentation
* Generating single-page HTML from your MkDocs documentation
* Converting your MkDocs documentation to other formats, such as asciidoc

The output Markdown document is compatible with [pandoc](http://www.pandoc.org/). 

This package is written in Python 2.7 and relies on `mkdocs` and the Python `Markdown` implementation. Aside from several filters, the module contains a `MkDocsCombiner` class tying them together into a coherent whole, and the command-line tool `mkdocscombine`. 

[`mkdocs-combine`](https://github.com/twardoch/mkdocs-combine/) is maintained by Adam Twardoch. It's a fork of [`mkdocs-pandoc`](https://github.com/jgrassler/mkdocs-pandoc) by Johannes Grassler. 

# Installation

_Note: The following instructions apply to both Unixoid systems and Windows._

If you'd like to use the development version, use

```
pip install git+https://github.com/twardoch/mkdocs-combine.git
```

Note that if you are behind a proxy, you might need to add the `--proxy` option like this

```
pip --proxy=http[s]://user@mydomain:port install ...
```

If you'd like to install a local development version from the current path, use

```
pip install -e .
```

## Pandoc compatibility

For generating PDF through `pandoc` you will need to install a few things `pip` won't handle, namely `pandoc` and the somewhat exotic LaTeX packages its default LaTeX template uses. On a Ubuntu 14.04 system this amounts to the following packages:

```
fonts-lmodern
lmodern
pandoc
texlive-base
texlive-latex-extra
texlive-fonts-recommended
texlive-latex-recommended
texlive-xetex
```
On a Windows system you can get them through [Chocolatey](https://chocolatey.org/). Once you have Chocolatey up and running the following commands should leave you with everything you need to create PDF output from `pandoc`:

```
choco install python
choco install pandocpdf
```

# Usage

When executed in the directory where your documentation's `mkdoc.yml` and the `docs/` directory containing the actual documentation resides, `mkdocscombine` should print one long Markdown document suitable for `pandoc` on standard output. The tool also allows to output a long HTML file in addition to, or in place of the Markdown file. 

```
usage: mkdocscombine [-h] [-V] [-o OUTFILE] [-f CONFIG_FILE] [-e ENCODING]
                     [-x EXCLUDE] [-H OUTHTML] [-y | -Y] [-c | -C] [-u | -k]
                     [-t | -g] [-G WIDTH] [-r | -R] [-a | -A] [-m | -l]
                     [-i IMAGE_EXT]

mkdocscombine.py - combines an MkDocs source site into a single Markdown
document

optional arguments:
  -h, --help            show this help message and exit
  -V, --version         show program's version number and exit
  -v, --verbose         print additional info during execution

files:
  -o OUTFILE, --outfile OUTFILE
                        write combined Markdown to path ('-' for stdout)
  -f CONFIG_FILE, --config-file CONFIG_FILE
                        MkDocs config file (default: mkdocs.yml)
  -e ENCODING, --encoding ENCODING
                        set encoding for input files (default: utf-8)
  -x EXCLUDE, --exclude EXCLUDE
                        exclude Markdown files from processing (default: none)
  -H OUTHTML, --outhtml OUTHTML
                        write simple HTML to path ('-' for stdout)

structure:
  -y, --meta            keep YAML metadata (default)
  -Y, --no-meta         strip YAML metadata
  -c, --titles          add titles from mkdocs.yml to Markdown files (default)
  -C, --no-titles       do not add titles to Markdown files
  -u, --up-levels       increase ATX header levels in Markdown files (default)
  -k, --keep-levels     do not increase ATX header levels in Markdown files
  -B, --no-page-break   do not add page break between pages (default)
  -b, --page-break      add page break between pages

tables:
  -t, --tables          keep original Markdown tables (default)
  -g, --grid-tables     combine Markdown tables to Pandoc-style grid tables
  -G WIDTH, --grid-width WIDTH
                        char width of converted grid tables (default: 100)

links:
  -r, --refs            keep MkDocs-style cross-references
  -R, --no-refs         replace MkDocs-style cross-references by just their
                        title (default)
  -a, --anchors         keep HTML anchor tags
  -A, --no-anchors      strip out HTML anchor tags (default)

extras:
  -m, --math            keep \( \) Markdown math notation as is (default)
  -l, --latex           combine the \( \) Markdown math into LaTeX $$ inlines
  -i IMAGE_EXT, --image-ext IMAGE_EXT
                        replace image extensions by (default: no replacement)
  -d, --admonitions-md  convert admonitions to HTML already in the Markdown
```

## Usage example

```
cd ~/mydocs
mkdocscombine -o mydocs.pd
pandoc --toc -f markdown+grid_tables+table_captions -o mydocs.pdf mydocs.pd   # Generate PDF
pandoc --toc -f markdown+grid_tables -t epub -o mydocs.epub mydocs.pd         # Generate EPUB
```

# Bugs

The following things are known to be broken:

* Line wrapping in table cells will wrap links, which causes whitespace to be inserted in their target URLs, at least in PDF output. While this is a bit of a Pandoc problem, it can and should be fixed in this module.
* [Internal Hyperlinks](http://www.mkdocs.org/user-guide/writing-your-docs/#internal-hyperlinks) between markdown documents will be reduced to their link titles, i.e. they will not be links in the resulting Pandoc document.

# Copyright

  * © 2015 Johannes Grassler <johannes@btw23.de>
  * © 2017 Adam Twardoch <adam+github@twardoch.com>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

[http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0)

You will also find a copy of the License in the file `LICENSE` in the top level
directory of this source code repository. In case the above URL is unreachable and/or differs from the copy in this file, the file takes precedence.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.


### Projects related to Markdown and MkDocs by Adam Twardoch: 

* [https://twardoch.github.io/markdown-rundown/](https://twardoch.github.io/markdown-rundown/) — summary of Markdown formatting styles [git](https://github.com/twardoch/markdown-rundown)
* [https://twardoch.github.io/markdown-steroids/](https://twardoch.github.io/markdown-steroids/) — Some extensions for Python Markdown [git](https://github.com/twardoch/markdown-steroids)
* [https://twardoch.github.io/markdown-utils/](https://twardoch.github.io/markdown-utils/) — various utilities for working with Markdown-based documents [git](https://github.com/twardoch/markdown-utils)
* [https://twardoch.github.io/mkdocs-combine/](https://twardoch.github.io/mkdocs-combine/) — convert an MkDocs Markdown source site to a single Markdown document [git](https://github.com/twardoch/mkdocs-combine)
* [https://github.com/twardoch/noto-mkdocs-theme/tree/rework](https://github.com/twardoch/noto-mkdocs-theme/tree/rework) — great Material Design-inspired theme for MkDocs [git](https://github.com/twardoch/noto-mkdocs-theme)
* [https://twardoch.github.io/clinker-mktheme/](https://twardoch.github.io/clinker-mktheme/) — great theme for MkDocs [git](https://github.com/twardoch/clinker-mktheme)

