---
title: Developer/CodeStyle
permalink: /Developer/CodeStyle/
---

# C++ Code

We try to have the code compatible to a wide range of C++11 compatible
compilers and test every commit / pull request against [gcc and clang on
macOS and Linux](https://travis-ci.org/eclipse/sumo), and [Visual
Studio 2013 - 2017 on
Windows](https://ci.appveyor.com/project/eclipsewebmaster/sumo). For the
C++ code we use [AStyle](http://astyle.sourceforge.net/) to keep
indentation and other whitespace usage consistent throughout the
project. In order to make your code appear same as the original SUMO
code use the following call to AStyle (or execute
tools/build/checkStyle.py --fix <FILE_NAME\> before committing):

```sh
astyle --style=java --unpad-paren --pad-header --pad-oper --add-brackets --indent-switches --align-pointer=type -n <FILE_NAME>
```

Still, there are several other things you should keep in mind (The
[Google C++ Style
Guide](https://google.github.io/styleguide/cppguide.html) is a good read
as well):

## Templates

Use the following templates for your files:

- [Developer/CppFileTemplate](../Developer/CppFileTemplate.md)
- [Developer/HFileTemplate](../Developer/HFileTemplate.md)

## Whitespace

- Never use *tab stop*. Always use 4 *spaces* for indentation. In
  Visual Studio *tab stop* can be disabled in Tools/Options/Text
  Editor/All languages/Tabs.

## Class Names

- All class names must start with capital letter
- The first capital letters of a class should be used to indicate
  their module (**NB**Edge -\> **N**et **B**uild module,
  **MS**ChargingStation -\> **M**icro **S**imulation module
- If a class name is composed of multiple words, every word must start
  with capital letter (Micro simulation charging station -\>
  **MSC**harging**S**tation)
- please do not arbitrarily introduce new namespaces

## Variable Names

- Variables used within methods should be named using "camel-caps",
  f.e. "currentCarVelocity"
- Member variables should additionally have the prefix "my", f.e.
  "myMaximumVehicleVelocity"
- try not to abbreviate variable names (myMaximumVehicleVelocity
  instead of myMVV)
- give names to variables which describe its purpose within the
  context
- do not reuse variables for different purposes

## Enums

- All enum names must start with capital letter, and If their name is
  composed of multiple words, every word must start with capital
  letter
- Every definition must be written using capital letters (SumoXMLTag {
  **SUMO_TAG_EDGE**, **SUMO_TAG_LANE**, ... } )

## Encoding

Only use the plain [ASCII](https://en.wikipedia.org/wiki/ASCII)
characters in *cpp* and *h* files. (0x20 - 0x7E). If you need to convey
extended character information use TeX or HTML notation instead.

## \#include-statements

- **do not use backslashes (\\)\!** use slashes (/) only
- **do not use absolute paths\!**; something like \#include
  "/home/daniel/mylibs/include_me.h" will never work somewhere else\!
  Use relative paths and tell the compiler where the other libraries
  may be found. If you have problems on this, please tell us, we'll
  write some further documentation\!
- in general we mention the standard library includes first (like
  \#include <vector\>), then the includes of libraries used by SUMO
  (FOX / Xerces) and later our own headers
- brackets ("<" and "\>") are preferred for files not residing in the
  same directory, while quotation marks are usually used for files in
  the same directory.

## "using namespace ..."-statements

- Avoid them whenever possible and especially **do not use "using
  namespace ..."-statements within the .h-files\!** It's possible that
  an included library tries to use a different implementation - who
  knows? Using a "using namespace ..."-statement within your .h-file
  may yield in an unexpected behaviour.

## Throw-declarations

- We are not using throw-declarations, see:
  <http://www.gotw.ca/publications/mill22.htm> (thanks to Björn
  Hendriks); we'll remove the existing ones subsequently.

## Object Handling

- If the structure you are implementing has an id, then derive the
  object from `Named` ({{SUMO}}/src/utils/common/Named.h). This class supports
  the `std::string getID()`-method.
- If you want to store something in a map pointing from string ids to
  object instances, then use `NamedObjectCont`
  ({{SUMO}}/src/utils/common/NamedObjectCont.h).

## Integer Types

- Use simple int whenever possible. There are probably lots of places
  in the sumo code which rely on the fact that this has at least 32
  bits, so you may too.
- Try to avoid unsigned ints whenever possible. It is sometimes not
  possible when calling Xerces-C and GL functions or using
  std::string::npos but try to limit it to those cases.
- If you need at least 64 bit, use long long int.

## Conditionals

- Do not use redundant condition checks. Example: ***if***
  *(myCondition)* instead of ***if*** (*myCondition* == ***true***)

## Loops

- Use the C++ 11 range loop syntax whenever possible. Example:
  ***for*** *(element& it : myContainer)* instead of ***for***
  *(container::iterator it = myContainer.begin(); it \!=
  myContainer.end(); it++)*. Still try to name the element type
  explicitly instead of using "auto" at least when iterating over
  lists / vectors.

# Python Code

We try to adhere to the [PEP 8 Style
Guide](http://www.python.org/dev/peps/pep-0008/) and use
[autopep8](https://github.com/hhatto/autopep8) and
[flake8](https://gitlab.com/pycqa/flake8) to check this. If you have
both modules installed, you can run [tools/build/checkStyle.py
<FILENAME\>](../Tools/Build.md#checkstylepy) to check this and also
fix some of the errors. We run both tools without any special options
except for the line length which we limit to 120.

Please remember to make your file executable if (and only if) the file
should be executable (i.e. is not a module to be imported by others).
Windows users please follow this
[recipe](https://stackoverflow.com/questions/21691202/how-to-create-file-execute-mode-permissions-in-git-on-windows).
Executable files should also contain a so called shebang in the first
line:

```
!#/usr/bin/env python
```

If your script needs Python 3 then state python3 there.

## Python3 compatibility

Our main development still focuses on Python 2.7 but we strive for
Python 3 compatibility. When writing or editing scripts, keep the
following in mind:

- use *print* as a function (`from __future__ import print_function`)
- use *items* instead of iteritems, which will return a list under
  Python 2.7 and an iterator in Python 3(`for item in dict.items()`)
- use *values* instead of *itervalues*, which will return a list under
  Python 2.7 and an iterator in Python 3(`for val in dict.values()`)
- use *//* instead of */* for integer division because */* will do a
  floating point division under Python 3 (`4/3 = 1.3333333`)
- the floating point division with */* can be included in Python 2 by (`from __future__ import division`)
- while iterating over unsorted lists or iterators Python 2 and Python
  3 do not work in the same order. Therefore add a explicit *sorted*
  before iteration when you write e.g. a xml file (`for item in sorted(list)`)
- use argparse library instead for optparse due to optparse wont be
  updated for Python 3 anymore. The use of most functions stayed the
  same, only some names differ. For further information please check
  <https://docs.python.org/3/library/argparse.html>
- if you want to use different code for Python 2 and Python 3, which
  can be necessary since not all libraries exist for both versions,
  you can use a simple if else statement with the PY3 boolean defined
  by(`PY3 = sys.version_info > (3,)`)
- use byte type in Python 3 wherever you use binary data and string
  type wherever you used unicode strings before. This can be made by
  if else statements using the PY3 boolean. See a list for the
  different methods possible on bytes and strings at
  <https://docs.python.org/3/howto/pyporting.html#text-versus-binary-data>

For further information about possible error sources and difficulties
resulting in Python 3 compatibility see also
<http://python3porting.com/> and
<https://docs.python.org/3/howto/pyporting.html>

## Template

Use the following template for your files:

- [Developer/PythonFileTemplate](../Developer/PythonFileTemplate.md)

# Line endings and keyword replacement

We enforce special line endings for the following file types (overriding
core.autocrlf settings of git) using [{{SUMO}}/.gitattributes](https://github.com/eclipse/sumo/blob/master/.gitattributes):

- LF for
  - source files (".h", ".cpp", ".py", ...),
  - Test files (".xml", ".prog", ".complex", ".dfrouter",
    ".duarouter", ".jtrrouter", ".netconvert", ".sumo", ".meso",
    ...)
- CRLF for Windows only files (".bat", ".props", ".vcxproj",
  ".filters", ...)

Git has no equivalent to the svn:keywords property. We use a custom
script to handle the $Id$ keyword like before but it has to be enabled
manually by adding the following lines to your global .gitconfig
(usually in your home directory)

```
  [filter "keywords"]
      clean  = .git_filters/keywords.py clean
      smudge = .git_filters/keywords.py %f
```

The filter is enabled for Python and C++ source files.

For the current settings you can always look at [{{SUMO}}/.gitattributes]({{Source}}.gitattributes).