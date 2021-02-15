---
title: Developer/Documentation Build
permalink: /Developer/Documentation_Build/
---

You can read the SUMO Documentation online at: <https://sumo.dlr.de/docs/>

The Documentation is continuously updated and always refers to the latest development version.

In summary, the documentation works as follows:

- The documentation is being "developed" or "written" as Markdown files, located in the [SUMO GitHub repository](https://github.com/eclipse/sumo/tree/master/docs/web)
- The static HTML sites are being generated using MkDocs and certain MkDocs-plugins

# Requirements
Building the documentation requires the python3-version of mkdocs along with several plugins. These can be installed with pip in the following manner:
```
 pip3 install mkdocs mkdocs-macros-plugin mkdocs-git-revision-date-plugin pymdown-extensions mdx_truly_sane_lists mdx_gh_links plantuml_markdown
```
You also need to install the plantuml package
```
 sudo apt install plantuml
```

# Documentation Parts

## Source Files

### Markdown files

Each "Page" or "Article" corresponds to a Markdown file. This files are located in the [repository](https://github.com/eclipse/sumo/tree/master/docs/web/docs) under `/docs/web/docs/` where its location corresponds to the hierarchy in the documentation (there are directories that contain content-related pages). The Path observed in the URI corresponds to the Path in the Git Documentation directory.

### Images

Images and other Media files that are included in the pages are located in the [repository](https://github.com/eclipse/sumo/tree/master/docs/web/docs/images) under `/docs/web/docs/images/`.

### MkDocs related files

All MkDocs related files, necessary to build the static HTML pages, are located in the [repository](https://github.com/eclipse/sumo/tree/master/docs/web) under `/docs/web/`.
The necessary files are as follows:

- **The configuration file:** called "*mkdocs.yml*", it contains some building instructions for MkDocs including the Macros and some site options.
- **The Theme:** all HTML, CSS and JS files located in the *theme* directory.

## Static HTML pages

The HTML pages are being build using the Markdown files. The final pages are available to read at the [online Documentation](https://sumo.dlr.de/docs/). Although the Documentation [Source files](#source_files) are included in the repository, the final Static HTML pages are not. In order to get a local copy of this pages, follow the [static HTML site generation](#static_html_site_generation) instructions.

# Static HTML site generation

The static HTML site generation is explained here: <https://github.com/eclipse/sumo/blob/master/docs/web/README.md>

It can be triggered (from within the DLR network) via <https://sumo.dlr.de/generate-documentation.php> (or for a complete rebuild, via <https://sumo.dlr.de/generate-documentation.php?type=complete>).