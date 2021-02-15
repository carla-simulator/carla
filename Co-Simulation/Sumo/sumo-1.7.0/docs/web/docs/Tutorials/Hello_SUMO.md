---
title: Tutorials/Hello SUMO
permalink: /Tutorials/Hello_SUMO/
---

!!! note
    This tutorial assumes very basic computer skill. If you run into any questions please read the page [Basics/Basic Computer Skills](../Basics/Basic_Computer_Skills.md).

## Hello Sumo - Introduction

This tutorial aims at first time Sumo users. We are building the
simplest net possible and let a single car drive on it. All files
mentioned here can also be found in the {{SUMO}}/docs/tutorial/hello directory.
The most recent version can be found in the repository at [{{SUMO}}/tests/complex/tutorial/hello/data/]({{Source}}tests/complex/tutorial/hello/data/).

In [sumo](../sumo.md) a street network consists of nodes
(junctions) and edges (streets connecting the junctions). Thus, if we
want to create a network with two streets, subsequent to each other, we
need three nodes and two edges. We will see in the section on
[\#Routes](#routes), why the simplest network cannot contain
only one edge.

## Nodes

All nodes have a location (x- and y-coordinate, describing distance to
the origin in meters) and an id for future reference. Thus our simple
node file looks as follows

```
<nodes>
    <node id="1" x="-250.0" y="0.0" />
    <node id="2" x="+250.0" y="0.0" />
    <node id="3" x="+251.0" y="0.0" />
</nodes>
```

You can edit a file with a text editor of your choice and save this for
instance as `hello.nod.xml` where `.nod.xml` is the default suffix for
Sumo node files.

## Edges

Now we are connecting the nodes with edges. This is as easy as it
sounds. We have a source node id, a target node id, and an edge id for
future reference. Edges are directed, thus every vehicle travelling this
edge will start at the node given in `from`
and end at the node given in `to`.

```
<edges>
    <edge from="1" id="1to2" to="2" />
    <edge from="2" id="out" to="3" />
</edges>
```

Save this data into a file called `hello.edg.xml`. Now that we have
nodes and edges we can call the first SUMO tool to create a network.
Make sure [netconvert](../netconvert.md) is somewhere in your
`PATH` and call

```
netconvert --node-files=hello.nod.xml --edge-files=hello.edg.xml --output-file=hello.net.xml
```

This will generate our network called `hello.net.xml`.

## Routes

Now that we have a net, we still need a car. In [sumo](../sumo.md)
the vehicles have types defining their basic properties such as length,
acceleration and deceleration, and maximum speed. Furthermore it needs a
so called sigma parameter which introduces some random behavior and is
due to the car following model used. Setting it to 0 gives a
deterministic car.

Now we define a route for our car which simply consists of the two edges
we defined.

Last but not least we define our single car mainly referring to the
entries before and giving it a departure time as in the following
`hello.rou.xml` file. There are many more attributes for customizing a
vehicle and its type. See "[Definition of Vehicles, Vehicle Types, and
Routes](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md)"
for further information.

```
<routes>
    <vType accel="1.0" decel="5.0" id="Car" length="2.0" maxSpeed="100.0" sigma="0.0" />
    <route id="route0" edges="1to2 out"/>
    <vehicle depart="1" id="veh0" route="route0" type="Car" />
</routes>
```

## Configuration

Now we glue everything together into a configuration file

```
<configuration>
    <input>
        <net-file value="hello.net.xml"/>
        <route-files value="hello.rou.xml"/>
    </input>
    <time>
        <begin value="0"/>
        <end value="10000"/>
    </time>
</configuration>
```

Saving this to `hello.sumocfg` we can start the simulation by either

```
sumo -c hello.sumocfg
```

or with GUI by

```
sumo-gui -c hello.sumocfg
```

When simulating with GUI it's useful to add a gui-settings file, so you
don't have to change the settings after starting the program. To do so,
alter `hello.sumocfg` to look like this

```
<configuration>
    <input>
        <net-file value="hello.net.xml"/>
        <route-files value="hello.rou.xml"/>
        <gui-settings-file value="hello.settings.xml"/>
    </input>
    <time>
        <begin value="0"/>
        <end value="10000"/>
    </time>
</configuration>
```

After that create a file with the viewsettings

```
<viewsettings>
    <viewport y="0" x="250" zoom="100"/>
    <delay value="100"/>
</viewsettings>
```

Save this as the name you included in the config file, in this example
this would be `hello.settings.xml`.

Here we used viewport to set the camera position and we used delay to
set the delay between each step of the simulation in ms.

## Further Reading

More information on defining networks using XML can be found here:
[Networks/Building Networks from own
XML-descriptions](../Networks/PlainXML.md).
[netconvert](../netconvert.md) also allows to import networks from
other applications, further information is available here:
[Networks/Import](../Networks/Import.md).

More information on defining vehicles can be found here: [Definition of
Vehicles, Vehicle Types, and
Routes](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md).

More [Tutorials](../Tutorials.md).