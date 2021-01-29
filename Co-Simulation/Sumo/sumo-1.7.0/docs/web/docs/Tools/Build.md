---
title: Tools/Build
permalink: /Tools/Build/
---

# Build Tools

## dailyBuildMSVC.py

The build script for the nightly windows build. Calls cmake and triggers
release and debug build as well as the tests and installer / zip
creation. Depends on a successful Linux build to pick up the docs.

## dailyUpdateMakeGCC.sh

The build script for the nightly Linux and macOS build. Calls cmake and
triggers release and debug build as well as the tests and the zips.

# Code Cleaning Tools

## checkStyle.py

Checks .java, .cpp, .h and .py files for the correct license header and
applies [autopep8](https://github.com/hhatto/autopep8) and
[flake8](https://gitlab.com/pycqa/flake8) for PEP 8 compatibility. Also
applies [astyle](http://astyle.sourceforge.net/) to all .java, .cpp, and
.h files. The position of the source tree(s) to check is given as
optional arguments or determined relative to the position of the script.
This call is part of our nightly tests. The script is able to fix some
minor whitespace / naming issues when called with the "--fix" flag. The
PEP 8 checks are quite slow, so if only a header check is wished, run
with the option "--skip-pep".

Call:

```
checkStyle.py
checkStyle.py --fix <SUMO_HOME>/tools
```

# Documentation Tools

## buildHTMLDocs.py

Converts wiki-documentation into static HTML pages. It removes the wiki
navigation and adds user doc navigation while keeping the contents
intact. This script is called by the main Makefile if "make docs" is
called. If the script is called with an argument, it tries to fetch the
wiki site with the given name, if not it retrieves all sites listed in
the [table of contents](../index.md). Call
"buildHTMLDocs.py --help" for a complete list of options.

Call:

```
buildHTMLDocs.py SUMO
```

## checkAuthors.py

Checks the log for all source files for authors and "thanks" and checks
whether they appear in the file header. It can optionally try to fix the
file header and also update the global AUTHORS file if needed.

Call:

```
checkAuthors.py
```