---
title: Tutorials/ScenarioGuide
permalink: /Tutorials/ScenarioGuide/
---

# Introduction

This Tutorial is meant to serve as a high-level guide for building a
[sumo](../sumo.md) scenario. It provides an outline of the typical
steps when building a scenario and lists the recommended wiki pages for
each step.

!!! note
    This tutorial assumes familiarity with [Tutorials/Hello SUMO](../Tutorials/Hello_SUMO.md), [Tutorials/quick start](../Tutorials/quick_start.md) or [Tutorials/Quick Start old style](../Tutorials/Quick_Start_old_style.md).

# Build the road network

Every simulation requires a road network. The application
[netconvert](../netconvert.md) is used to create a network which
can be used by the simulation [sumo](../sumo.md). It is also
possible to build/modify a simulation network by hand with use of
[netedit](../netedit.md), which provides a graphical user
interface.

## If you already have some network data

Check whether a direct import is possible:
[Networks/Import](../Networks/Import.md). Otherwise you will need
to convert the data to a simple XML-format which can be read by
[netconvert](../netconvert.md). Read this page:
[Networks/Import](../Networks/Import.md).

## If you do not yet have any network data

Use publicly available network data from
[OpenStreetMap](http://www.openstreetmap.org/) as described here:
[Networks/Import/OpenStreetMapDownload](../Networks/Import/OpenStreetMapDownload.md).
Then import the network as described here:
[Networks/Import/OpenStreetMap](../Networks/Import/OpenStreetMap.md).

# Generate the traffic

First you should understand the basics of vehicle modelling:
[Definition_of_Vehicles,_Vehicle_Types,_and_Routes](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md).
You have various ways to insert vehicles into the map. Your choice
basically depends on what kind of information about the traffic you
have:
[Demand/Introduction_to_demand_modelling_in_SUMO](../Demand/Introduction_to_demand_modelling_in_SUMO.md).

# Improve your Scenario

Most methods for network import leave some aspect of deficient network
quality. Very often this manifests as unexpected/unrealistic traffic
jams and teleporting vehicle errors.

## Modifying the Network

You will have to patch your network data to add missing roads, prohibit
some turns, correct the number of lanes and add/remove some traffic
lights. The recommended ways to perform the necessary changes include:

1.  Encode the changes in *plain-xml* files as described in
    [Networks/Building_Networks_from_own_XML-descriptions](../Networks/PlainXML.md).
    Most XML-attributes are optional so you only need to set the values
    you would like to change.
2.  patch your network using [netconvert](../netconvert.md) by
    loading the net.xml along with the plain-xml files. You can even use
    this during the initial import (i.e. load an OSM-file along with
    your plain-xml files)
3.  directly adjust your network with GUI: after opening the program
    [netedit](../netedit.md) you load the network file
    (\*.net.xml)that needs to be modified. Then you choose the
    respective functions in netedit to modify e.g. the number of lanes,
    road shapes, traffic signal plans, intersection geometries, and to
    combine intersections or edges.

### Example: Patching the type of a node

prepare a file like this:

patch.nod.xml :

```
<nodes>
    <node id="id_of_the_node_you_want_to_modify" type="right_before_left"/>
<nodes>
```

and patch the network like this:

```
netconvert --sumo-net-file your.net.xml --node-files patch.nod.xml -o yourpatched.net.xml
```

or perform the patch during the initial import:

```
netconvert --osm-file yourOSMfile.xml --node-files patch.nod.xml ...<other options>
```

### Modifying an imported network via plain.xml

Instead of patching individual elements you can also convert your
network to plain.xml and modify the plain file and then re-assemble the
network like this:

```
netconvert --sumo-net-file your.net.xml --plain-output-prefix yourplain
```

or during import:

```
netconvert --osm-files yourOSMinput.xml --plain-output-prefix yourplain ... <your other options>
```

This will give you the files:

```
yourplain.edg.xml
yourplain.nod.xml
yourplain.con.xml
yourplain.tll.xml
```

You can edit these files and then reassamble the network by loading
some or all of them:

```
netconvert --edge-files yourplain.edg.xml --node-files yourplain.nod.xml -o new.net.xml
```

or

```
netconvert --edge-files yourplain.edg.xml --node-files yourplain.nod.xml --connection-files yourplain.con.xml -o new.net.xml
```

!!! note
    When working with networks based on OSM input, the .edg.xml file will contain information on the OSM-type of an edge (i.e. *highway.residential*. When loading such an .edg.xml file with [netconvert](../netconvert.md) you need to supply the option **--ignore-errors.edge-type** or load a type-file to prevent errors of the form *Type ... not defined*.

### Modifying node positions with the help of *sumolib*

[Sumolib](../Tools/Sumolib.md) is a python library for working with
SUMO files. The file [{{SUMO}}/tests/tools/sumolib/patch_network/runner.py]({{Source}}tests/tools/sumolib/patch_network/runner.py) provides an example for randomly changing all node
positions with a few lines of python code.

## Traffic Light Programs

In almost all cases the traffic light programs will have been guessed by
[netconvert](../netconvert.md) and turn out different from the real
traffic lights. A simple way to improve traffic lights programs is
making them start their program cycle at a different time. Experiment
with the option **--tls.half-offset** and all the other **--tls.\*** options of
[netconvert](../netconvert.md).

A more practical and efficient way to improve your traffic lights is to
[give the program explicitly with a *tls.xml*
file](../Networks/PlainXML.md#traffic_light_program_definition).

At this time [netconvert](../netconvert.md) only supports the
creation of static traffic light programs. For dynamic traffic lights
see
[Tutorials/TraCI4Traffic_Lights](../Tutorials/TraCI4Traffic_Lights.md).
Furthermore, actuated traffic control can also be set, either based on
time gaps or time loss. More information about actuated traffic control
and settings can be found at
[Simulation/Traffic_Lights\#Actuated_Traffic_Lights](../Simulation/Traffic_Lights.md#actuated_traffic_lights)).

# Manage Scenarios

If you have two networks *A.net.xml* and *B.net.xml* you may wish to
find out how they differ. This can be accomplished using the tool
{{SUMO}}*/tools/net/netdiff.py*. Running this tool will give you a set of
plain-XML difference files. They only contain the differences grouped by
*deleted*, *created* and *changed* elements. It is even possible to load
these files along with *A.net.xml* to recreate *B.net.xml*.