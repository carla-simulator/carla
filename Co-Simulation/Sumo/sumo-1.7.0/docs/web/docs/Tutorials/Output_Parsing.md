---
title: Tutorials/Output Parsing
permalink: /Tutorials/Output_Parsing/
---

In this tutorial you will learn how to

- set up an abstract network using edge types,
- create repeatedly identical vehicles using flows,
- reroute vehicles dynamically such that they drive endlessly,
- analyze output files using the [sumolib python
  library](../Tools/Sumolib.md),

and as a bonus how to

- use socket output for online evaluation and saving disk space.

Despite the keywords online, socket, and python API, this tutorial does
*not* cover any TraCI related content.

## Network setup

The goal is to build a simple network where the vehicles drive in
circles so we setup four nodes for the corners as follows
(circular.nod.xml):

```
<?xml version="1.0" encoding="UTF-8"?>

<nodes version="0.13" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.sf.net/xsd/nodes_file.xsd">
    <node id="bottom-left" x="0" y="0"/>
    <node id="bottom-right" x="1250" y="0"/>
    <node id="top-right" x="1250" y="1250"/>
    <node id="top-left" x="0" y="1250"/>
</nodes>
```

All edges connecting the nodes should have the same number of lanes and
the same maximum speed. In order to save on typing we define the edge
type in a separate file (circular.typ.xml):

```
<?xml version="1.0" encoding="UTF-8"?>

<types xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.sf.net/xsd/types_file.xsd">
    <type id="edgeType" numLanes="2" speed="36.1"/>
</types>
```

Finally we define the edges connecting the nodes (circular.edg.xml):

```
<?xml version="1.0" encoding="UTF-8"?>

<edges version="0.13" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.sf.net/xsd/edges_file.xsd">
    <edge from="bottom-left" id="bottom" to="bottom-right" type="edgeType"/>
    <edge from="bottom-right" id="right" to="top-right" type="edgeType"/>
    <edge from="top-right" id="top" to="top-left" type="edgeType"/>
    <edge from="top-left" id="left" to="bottom-left" type="edgeType"/>
</edges>
```

The netconvert call is very straightforward

```
netconvert -n circular.nod.xml -t circular.typ.xml -e circular.nod.xml -o circular.net.xml
```

In order to simplify the resulting net (and get the highest speed out of
the simulation) we omit turnarounds and simplify the movement over
junctions by removing the junction internal lanes. The complete
netconvert configuration file (circular.netccfg):

```
<?xml version="1.0" encoding="UTF-8"?>

<configuration xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
    xsi:noNamespaceSchemaLocation="http://sumo.sf.net/xsd/netconvertConfiguration.xsd">

   <input>
       <node-files value="circular.nod.xml"/>
       <edge-files value="circular.edg.xml"/>
       <type-files value="circular.typ.xml"/>
   </input>

<output>
       <output-file value="circular.net.xml"/>
</output>

   <processing>
       <no-internal-links value="true"/>
       <no-turnarounds value="true"/>
   </processing>

</configuration>
```

Try

```
sumo-gui -n circular.net.xml
```

for a look at the final network.

## Route and flow setup

```
<?xml version="1.0" encoding="UTF-8"?>

<routes xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
  xsi:noNamespaceSchemaLocation="http://sumo.sf.net/xsd/routes_file.xsd">
    <vType accel="1.5" decel="4.5" id="car" length="5" maxSpeed="36.1"/>
    <vType accel="0.4" decel="4.5" id="truck" length="12" maxSpeed="22.2"/>
    <route id="routeRight" edges="bottom right top left"/>
    <route id="routeLeft" edges="top left bottom right"/>
    <route id="routeTop" edges="left bottom right top"/>
    <route id="routeBottom" edges="bottom right top left"/>
    <flow begin="0" departPos="free" id="carRight" period="1" number="70" route="routeRight" type="car"/>
    <flow begin="0" departPos="free" id="carTop" period="1" number="70" route="routeTop" type="car"/>
    <flow begin="0" departPos="free" id="carLeft" period="1" number="70" route="routeLeft" type="car"/>
    <flow begin="0" departPos="free" id="carBottom" period="1" number="70" route="routeBottom" type="car"/>
    <flow begin="0" departPos="free" id="truckRight" period="1" number="30" route="routeRight" type="truck"/>
    <flow begin="0" departPos="free" id="truckTop" period="1" number="30" route="routeTop" type="truck"/>
    <flow begin="0" departPos="free" id="truckLeft" period="1" number="30" route="routeLeft" type="truck"/>
    <flow begin="0" departPos="free" id="truckBottom" period="1" number="30" route="routeBottom" type="truck"/>
</routes>
```

## Rerouters

```
<?xml version="1.0" encoding="UTF-8"?>

<additional xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.sf.net/xsd/additional_file.xsd">
    <route id="routeRight" edges="bottom right top left"/>
    <route id="routeLeft" edges="top left bottom right"/>
    <route id="routeTop" edges="left bottom right top"/>
    <route id="routeBottom" edges="bottom right top left"/>
    <rerouter id="rerouterBottom" edges="bottom">
        <interval begin="0" end="100000">
            <routeProbReroute id="routeRight" />
        </interval>
    </rerouter>
    <rerouter id="rerouterTop" edges="top">
        <interval begin="0" end="100000">
            <routeProbReroute id="routeLeft" />
        </interval>
    </rerouter>
</additional>
```

## Putting it all together

```
<?xml version="1.0" encoding="UTF-8"?>

<configuration xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.sf.net/xsd/sumoConfiguration.xsd">
    <input>
        <net-file value="circular.net.xml"/>
        <route-files value="circular.rou.xml"/>
        <additional-files value="circular.add.xml"/>
    </input>

<output>
        <netstate-dump value="dump.xml"/>
</output>

    <time>
        <begin value="0"/>
        <end value="10000"/>
    </time>
</configuration>
```

## Analyzing the output

## Bonus: Socket communication