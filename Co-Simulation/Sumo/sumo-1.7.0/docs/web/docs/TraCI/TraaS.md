---
title: TraCI/TraaS
permalink: /TraCI/TraaS/
---

# TraaS

TraaS is a java library for working with TraCI. The name **TraaS**
stands for *TraCI as a Service*. It was developed by Mario Krumnow and
uses code from the now discontinued
[TraCI4J](https://github.com/egueli/TraCI4J) project by Enrico Gueli.
Since version 1.0, TraaS is maintained within the SUMO repository at [{{SUMO}}/tools/contributed/traas]({{Source}}tools/contributed/traas).

TraaS provides three things which are explained below.

- A java traci client that can be used to write TraCI scripts directly
- A web service that can be used to connect multiple webservice
  clients to a running TraCI instance in a language-agnostic way.
- A web service client

In contrast to the native TraCI [multi-client
support](../TraCI.md#multiple_clients), TraaS does not require the
number of clients to be known beforehand. It also cannot impose any
ordering on client command execution which leaves client synchronization
to be managed on the user side.

# Java Client

The client provides functions using with the same name and signature as
the [python client](../TraCI/Interfacing_TraCI_from_Python.md). A
major difference is, that the method return values must be casted in the
calling code:

```
double timeSeconds = (double)conn.do_job_get(Simulation.getTime());
```

## Examples

The examples must be compiled using

```
 javac -cp sumo/bin/TraaS.jar ExampleName.java
```

after compiling they can be run using

```
 java -cp .:sumo/bin/TraaS.jar ExampleName
```

### Insert Vehicles and Access Induction Loop Data

[{{SUMO}}/tests/complex/traas/simple/data/Main.java]({{Source}}tests/complex/traas/simple/data/Main.java)

### Connect Multiple clients

[{{SUMO}}/tests/complex/traas/multiclient/data/MultiClient1.java]({{Source}}tests/complex/traas/multiclient/data/MultiClient1.java)

[{{SUMO}}/tests/complex/traas/multiclient/data/MultiClient2.java]({{Source}}tests/complex/traas/multiclient/data/MultiClient2.java)

### Subscribe to Variables

[{{SUMO}}/tests/complex/traas/subscription/data/Subscription.java]({{Source}}tests/complex/traas/subscription/data/Subscription.java)

### Generate screenshots during simulation runtime

### Change the TLS-program

### Get CO2 Emissions

# Webservice

# Webservice Client

# Documentation

JavaDoc for TraaS can be found at <https://sumo.dlr.de/javadoc/traas/>

# Building TraaS

The file {{SUMO}}*/bin/TraaS.jar* is included in every SUMO release. To build
TraaS locally, Apache [Maven](https://maven.apache.org/) is required. To build,
simply call *mvn package* within the traas folder or call make traas when you have a configured cmake build. 