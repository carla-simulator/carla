---
title: Tutorials/CityMobil
permalink: /Tutorials/CityMobil_old/
---

The *city_mobil* subdirectory in {{SUMO}}/docs/tutorial contains the setup for
a remote controlled simulation of a parking lot using sumo with its
traci interface controlled by a Python script.

The parameters for the application (including the paths to the
executables) are in constants.py. If you are running it from the
directory in the same source tree where the binaries reside, it should
work without changes. Before running the application you have to execute

```
./createNet.py
```

In order to change the layout of the parking lot and the behavior
(speed, capacity etc.) of the vehicles, constants.py should be edited
and then createNet.py re-executed.

In order to start the (remote controlled) simulation execute one of the
managers agentManager.py or simpleManager.py. If you want a visual
display you have to provide the -g option, e.g.

```
./simpleManager -g
```

Further options are described by starting the scripts with -h.