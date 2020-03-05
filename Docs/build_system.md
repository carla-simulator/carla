# Build system

> _This document is a work in progress, only the Linux build system is taken into account here._

The most challenging part of the setup is to compile all the dependencies and
modules to be compatible with a) Unreal Engine in the server-side, and b) Python
in the client-side.

The goal is to be able to call Unreal Engine's functions from a separate Python
process.

![modules](img/modules.png)

In Linux, we compile CARLA and all the dependencies with clang-7.0 and C++14
standard. We however link against different runtime C++ libraries depending on
where the code going to be used, since all the code that is going to be linked
with Unreal Engine needs to be compiled using `libc++`.

#### Setup

Command

```sh
make setup
```

Get and compile dependencies

  * llvm-7.1 (libc++ and libc++abi)
  * rpclib-2.2.1 (twice, with libstdc++ and libc++)
  * boost-1.72.0 (headers and boost_python for libstdc++)
  * googletest-1.8.1 (with libc++)

#### LibCarla

Compiled with CMake (minimum version required CMake 3.9).

Command

```sh
make LibCarla
```

Two configurations:

|                 | Server       | Client    |
|-----------------|--------------|-----------|
| **Unit tests**  | yes          | no        |
| **Requires**    | rpclib, gtest, boost | rpclib, boost
| **std runtime** | LLVM's `libc++` | Default `libstdc++` |
| **Output**      | headers and test exes | `libcarla_client.a` |
| **Required by** | Carla plugin | PythonAPI |  

<br>

#### CarlaUE4 and Carla plugin

Both compiled at the same step with Unreal Engine build tool. They require the
`UE4_ROOT` environment variable set.

Command

```sh
make CarlaUE4Editor
```

To launch Unreal Engine's Editor run

```sh
make launch
```

#### PythonAPI

Compiled using Python's `setuptools` ("setup.py"). Currently requires the
following to be installed in the machine: Python, libpython-dev, and
libboost-python-dev; both for Python 2.7 and 3.5.

Command

```sh
make PythonAPI
```

It creates two "egg" packages

  * `PythonAPI/dist/carla-X.X.X-py2.7-linux-x86_64.egg`
  * `PythonAPI/dist/carla-X.X.X-py3.5-linux-x86_64.egg`

This package can be directly imported into a Python script by adding it to the
system path

```python
#!/usr/bin/env python

import sys

sys.path.append(
    'PythonAPI/dist/carla-X.X.X-py%d.%d-linux-x86_64.egg' % (sys.version_info.major,
                                                             sys.version_info.minor))

import carla

# ...
```

or installed with `easy_install`

```sh
easy_install2 --user --no-deps PythonAPI/dist/carla-X.X.X-py2.7-linux-x86_64.egg
easy_install3 --user --no-deps PythonAPI/dist/carla-X.X.X-py3.5-linux-x86_64.egg
```
