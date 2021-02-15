---
title: Tutorials/Manhattan
permalink: /Tutorials/Manhattan/
---

# Introduction

This Tutorial explains how to build a [Manhattan Mobility
Model](https://en.wikipedia.org/wiki/Manhattan_mobility_model) in SUMO.
In this model a fixed number of vehicles drive randomly on a manhattan
grid network. All files can also be found in the
{{SUMO}}/docs/tutorial/manhattan directory.

# Creating the network

Creating manhattan grid networks is supported by the
[netgenerate](../netgenerate.md) application. The option **--grid** creates
grid networks. The number of grid cells can be set using the option **--grid.number**.
There are various options to configure the size and number of the cells
and to change the number of lanes and types of junctions. The options
for this tutorial are written in a configuration file. The network is
created by calling

```
netgenerate -c manhattan/data/manhattan.netgcfg
```

# Generating vehicles

The vehicles in a Manhattan mobility model drive randomly according to
specified turning ratios. This type of mobility is supported by the
[jtrrouter](../jtrrouter.md) application. This application requires
`<flow>`-definitions as input to define the starting point and starting times of
vehicles.

## Generating random flows for jtrrouter

The [randomTrips.py](../Tools/Trip.md#randomtripspy) tool can be
used to generated suitable randomFlows with the following options.

```
 <SUMO_HOME>/tools/randomTrips.py -n net.net.xml -o flows.xml --begin 0 --end 1 \
       --flows 100 --jtrrouter \
       --trip-attributes 'departPos="random" departSpeed="max"'
```

The option **--flows 100** defines the number of vehicles that shall drive in the
network. Usually a `<flow>` is used to defined multiple vehicles but in this
case each flow generates just a single vehicle at the start of the
simulation. (hence option **--end 1**). The option **--jtrrouter** must be set to generated flows
without destination. Otherwise the generated vehicles might end their
trip too early. The arguments supplied to option **--trip-attributes** are set to ensure that
multiple vehicles may enter the source edge in the first step.

The options are also encoded in the script runner.py.

!!! caution
    The randomTrips option **--jtrrouter** is only available since SUMO version 1.2.0. In earlier versions, the 'to'-attribute must be removed manually from the generated flows before processing them with [jtrrouter](../jtrrouter.md).

## Calling jtrrouter

The [jtrrouter](../jtrrouter.md) application is called with the
generated random flows. To ensure routes of sufficient length the option
**--allow-loops** must be set. Since no sink edge are defined, the option **--accept-all-destinations** is set. The
default turn ratios for the Manhattan Mobility Model (25% right, 50%
straight, 25% left) are set via option **--turn-defaults 25,50,25**.

All options for this tutorial are written in a configuration file. The
vehicles are created by calling

```
jtrrouter -c manhattan/data/manhattan.jtrrcfg
```

## Remarks on Vehicle number

The number of vehicles in the first few simulation seconds is limited by
available road space for vehicle insertions. If the number of vehicles
is large with respect to the network size, it may take a few simulation
steps before all vehicles have entered the network.

## Making vehicles run forever

Using JTRRouter, routes of arbitrary length can be generated. However, vehicles will eventually reach the end of their route and exit the simulation. To avoid this, the tool [generateContinuousRerouters.py](../Tools/Misc.md#generatecontinuousrerouterspy) can be used.