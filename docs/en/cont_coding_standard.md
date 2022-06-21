# Coding standard

*   [__General__](#general)  
*   [__Python__](#python)  
*   [__C++__](#c++)  

---
## General

  * Use spaces, not tabs.
  * Avoid adding trailing whitespace as it creates noise in the diffs.

---
## Python

  * Comments should not exceed 80 columns, code should not exceed 120 columns.
  * All code must be compatible with Python 2.7 and 3.7.
  * [Pylint][pylintlink] should not give any error or warning (few exceptions
    apply with external classes like `numpy` and `pygame`, see our `.pylintrc`).
  * Python code follows [PEP8 style guide][pep8link] (use `autopep8` whenever
    possible).

[pylintlink]: https://www.pylint.org/
[pep8link]: https://www.python.org/dev/peps/pep-0008/

---
## C++

  * Comments should not exceed 80 columns, code may exceed this limit a bit in
    rare occasions if it results in clearer code.
  * Compilation should not give any error or warning
    (`clang++-8 -Wall -Wextra -std=C++14 -Wno-missing-braces`).
  * The use of `throw` is forbidden, use `carla::throw_exception` instead.
  * Unreal C++ code (CarlaUE4 and Carla plugin) follow the
    [Unreal Engine's Coding Standard][ue4link] with the exception of using
    spaces instead of tabs.
  * LibCarla uses a variation of [Google's style guide][googlelink].
  * Uses of `try-catch` blocks should be surrounded by
    `#ifndef LIBCARLA_NO_EXCEPTIONS` if the code is used in the server-side.

[ue4link]: https://docs.unrealengine.com/latest/INT/Programming/Development/CodingStandard/
[googlelink]: https://google.github.io/styleguide/cppguide.html
