# Coding standard

*   [__General__](#general)  
*   [__Python__](#python)  
*   [__C++__](#c)

---
## General

  * Use spaces, not tabs.
  * Avoid adding trailing whitespace as it creates noise in the diffs.

---
## Python

  * Comments should not exceed 80 columns, code should not exceed 120 columns.
  * Code targets Python 3. The client wheel is built for the interpreter CARLA was configured with, and requires that version or newer; Python 2 is not supported.
  * [Pylint][pylintlink] should not give any error or warning (few exceptions
    apply with external classes like `numpy` and `pygame`, see `PythonAPI/.pylintrc`).
  * Python code follows [PEP8 style guide][pep8link] (use `autopep8` whenever
    possible).
  * Check before submitting: `pylint --rcfile=PythonAPI/.pylintrc <files>`.

[pylintlink]: https://www.pylint.org/
[pep8link]: https://www.python.org/dev/peps/pep-0008/

---
## C++

  * Comments should not exceed 80 columns, code may exceed this limit a bit in
    rare occasions if it results in clearer code.
  * The standard is C++20, built with the clang bundled in the Unreal Engine toolchain. Compilation should give no errors and no warnings (`-Wall -Wextra -Wno-missing-braces`).
  * The use of `throw` is forbidden, use `carla::throw_exception` instead.
  * Unreal C++ code (CarlaUnreal and Carla plugin) follow the
    [Unreal Engine's Coding Standard][uelink] with the exception of using
    spaces instead of tabs.
  * LibCarla uses a variation of [Google's style guide][googlelink].
  * Uses of `try-catch` blocks should be surrounded by
    `#ifndef LIBCARLA_NO_EXCEPTIONS` if the code is used in the server-side.

[uelink]: https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine
[googlelink]: https://google.github.io/styleguide/cppguide.html

---
## Before you submit

* The code compiles and the LibCarla tests pass.
* `pylint --rcfile=PythonAPI/.pylintrc` is clean on the Python you touched.
* `CHANGELOG.md` has an entry under `## LATEST Changes` for any user-visible change.

The full checklist is in the [contribution guidelines](cont_contribution_guidelines.md).
