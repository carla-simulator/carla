---
title: TraCI/C++TraCIAPI
permalink: /TraCI/C++TraCIAPI/
---

# C++ TraCIAPI client library

The C++ TraCIAPI is client library that is part of the SUMO-source tree.
The library can be downloaded [as part of the source
distribution](http://sumo.dlr.de/daily/sumo-src-git.zip). Example code
that uses the library is at
{{SUMO}}*/src/traci_testclient/TraCITestClient.cpp*. Doxygen-generated
documentation can be found
[here](http://sumo.dlr.de/daily/doxygen/d8/d1c/class_tra_c_i_a_p_i.html)

!!! note
    Due to not being the main client library for use by the core developers, this client library is sometimes tailing behind in API converage

## Building the Library

To build the library without building SUMO, the following dependencies
from the SUMO-source directory must be present:

```
src/utils/traci/TraCIAPI.h
src/utils/traci/TraCIAPI.cpp
src/foreign/tcpip/socket.h
src/foreign/tcpip/socket.cpp
src/foreign/tcpip/storage.h
src/foreign/tcpip/storage.cpp
src/traci-server/TraCIConstants.h
src/libsumo/TraCIDefs.h
```

Additionally, the file *config.h* must be present but these files can be
left empty. If SUMO was built, they can either be found in the *src* or
the *cmake-build/src* directory)

If you chose to copy all these files into the same directory, the
include paths in *TraCIAPI.h* and *TraCIAPI.cpp* need to be patched as
well.

### Building on Windows

The file **ws2_32.lib** must be added to the linker files in the
project properties (it contains the windows socket calls) to avoid
linker errors.

## [TraCI-Testclient]({{Source}}src/traci_testclient)

The TraCI-Testclient is an application for testing the server side of
the TRaCI-API. When using it as the basis for your own control script
one needs to be aware of its two operating modes:

### Feeding raw TraCI commands to SUMO

In this operating mode, the full TraCI-API is supported. Example input
files typically look like:

```
repeat 50 simstep2 0
setvalue 0xc4 0x31 veh0 <string> e_vo0
simstep2 200
```

This mode is meant for testing the server side and is unsuitable for
building a control program that mixes TraCI calls with custom code.

### Testing the TraCIAPI client library

In this mode the client library itself is used. Example code looks like:

```
SUMOTime t = simulation.getCurrentTime();
std::vector<std::string> = edge.getIDList();
```

## Example Code

```
#include <iostream>
#include <utils/traci/TraCIAPI.h>

class Client : public TraCIAPI {
public:
    Client() {};
    ~Client() {};
};

int main(int argc, char* argv[]) {
    Client client;
    client.connect("localhost", 1337);
    std::cout << "time in s: " << client.simulation.getTime() << "\n";
    std::cout << "run 5 steps ...\n";
    client.simulationStep(5);
    std::cout << "time in s: " << client.simulation.getTime() << "\n";
    client.close();
}
```

### compiling (make sure SUMO_HOME is set)

```
g++ -o test -I$SUMO_HOME/src TraCIAPITest.cpp $SUMO_HOME/src/utils/traci/TraCIAPI.cpp $SUMO_HOME/src/foreign/tcpip/socket.cpp $SUMO_HOME/src/foreign/tcpip/storage.cpp
```

### running

```
sumo -c test.sumocfg --remote-port 1337 &
./test
```