---
title: Libsumo
permalink: /Libsumo/
---

# Libsumo

The main way to interact with a running simulation is
[TraCI](TraCI.md) which gives the complete flexibility of doing
cross-platform, cross-language, and networked interaction with
[sumo](sumo.md) acting as a server. One major drawback is the
communication overhead due to the protocol and the socket communication.
To have a more efficient coupling without the need for socket
communication, the TraCI API is provided as a C++ library with the
following properties:

- C++ interface based on static functions and a few simple wrapper
  classes for results which can be linked directly to the client code
- Function signatures similar to [TraCI](TraCI.md)
- Pre-built language bindings for Java and Python (using
  [SWIG](http://www.swig.org/))
- Support for other programming languages via
  [SWIG](http://www.swig.org/)

# Limitations

The following things currently do not work (or work differently than with the TraCI Python client):

- running with [sumo-gui](sumo-gui.md)
- subscriptions that require additional arguments (except for *vehicle.getLeader*)
- stricter type checking
  - the TraCI client sometimes accepts any iterable object where Libsumo wants a list
  - TraCI client may accept any object where Libsumo needs a boolean value
- using traci.init or traci.connect is not possible (you always need to use libsumo.start)
- with traci every TraCIException will generate a message on stderr, Libsumo does not generate this message

# Building it

It currently requires cmake and swig being installed together with the
developer packages for Python (and Java if needed), for Windows see
[Installing/Windows_CMake](Installing/Windows_CMake.md). You
need to (re-)compile sumo yourself under Windows following the remarks
above, under Linux see [Installing/Linux_Build](Installing/Linux_Build.md)
(it is probably just a matter of calling cmake and
make again if you previously did a build without swig).
For the python bindings you will get a libsumo.py and a
_libsumo.so (or .pyd on Windows). If you place them somewhere on your
python path you should be able to use them as described below.

# Using libsumo

## Python

```
import libsumo
libsumo.start(["sumo", "-c", "test.sumocfg"])
libsumo.simulationStep()
```

Existing traci scripts can mostly be reused by calling

```
import libsumo as traci
```

In case you have a lot of scripts you can also set the environment
variable `LIBSUMO_AS_TRACI` to a non empty value which will trigger the
import as above.
