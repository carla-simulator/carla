---
title: Topics/V2X
permalink: /Topics/V2X/
---

# V2X simulations in SUMO

SUMO is usually coupled to a communication network simulator using a
middleware.

## Coupling to network simulators

<center>

**Coupling Middlewares Overview**

</center>

| Name                           | Used Network Simulator                                                                                           | Under Development |
| ------------------------------ | ---------------------------------------------------------------------------------------------------------------- | ----------------- |
| [Veins](#veins)     | Focused on OMNeT++/MiXiM with an extendable, modular framework (ports exist for INET, INETMANET, and JiST/SWANS) | yes               |
| [iCS](#ics)         | ns3                                                                                                              | yes               |
| [VSimRTI](#vsimrti) | ns3, JiST/SWANS or OMNeT++ (extendable)                                                                          | yes               |


### Veins

See [the Veins Homepage](http://veins.car2x.org/) for a short overview,
download links, beginner's tutorial, full publication list, and
documentation.

### iCS

iCS is the "iTETRIS Control System", a middleware developed in the
[iTETRIS](http://www.ict-itetris.eu/) project to join SUMO, ns-3, and
V2X application simulators. It is currently used and extended within the
[TransAID](https://www.transaid.eu/) project.

### VSimRTI

The V2X Simulation Runtime Infrastructure (VSimRTI) is a framework which
enables the preparation and execution of V2X simulations. The easy
integration and exchange of simulators enables the substitution of the
most relevant simulators for a realistic presentation of vehicle
traffic, emissions, wireless communication, and the execution of V2X
applications. For immediate use, a set of simulators is already coupled
with VSimRTI. For example, the traffic simulators VISSIM and SUMO, the
communication simulators ns3, JiST/SWANS and OMNeT++, the application
simulator VSimRTI_App, and several visualization and analysis tools are
prepared for VSimRTI. For more information see [VSimRTI
Homepage](http://www.dcaiti.tu-berlin.de/research/simulation/).