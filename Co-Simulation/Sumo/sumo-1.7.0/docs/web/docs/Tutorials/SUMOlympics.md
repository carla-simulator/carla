---
title: Tutorials/SUMOlympics
permalink: /Tutorials/SUMOlympics/
---

This tutorial sets up a competition (a collective 100 meter sprint) for
different traffic modes. You will learn how to create special lanes and
(very simple) traffic lights in netedit, use different vehicle classes
to define vehicle types and you will create flows for the different
types. All files can also be found in the {{SUMO}}/docs/tutorial/sumolympics
directory.

This tutorial is a reconstruction of a [VISSIM Scenario devised
by the PTV Group](https://www.youtube.com/watch?v=IpaNLxrtHOs).

# Building the Net

![editEdgeAttr.png](../images/EditEdgeAttr.png "Editing the location of the edge's startpoint")Open
[netedit](../netedit.md) and create a new network and add a single
edge by pressing `e` for entering the edge creation mode and clicking on
two different locations in the editing area. Change to inspection mode
(press `i`) and click on the starting point of the edge (at the location
of your first click). Now enter `0,0` in the textfield labeled `pos` in
the inspector panel on the left (see figure). Do the same for the edge's
endpoint, setting its position to `1000,0`. Now save your network under
the name `sumolypics.net.xml` (press `Ctrl+Shift-S`).

Now we have a long road, which will be the stage of our competition. The
participants in the competition will be transportation modes, i.e.,
busses, trams, bicycles, passenger cars, and feet. They should travel on
different lanes side-by-side. Thus, we have to add lanes for each mode.
To do so, right-click on the edge and hover over "add restricted lane"
in the context menu. This will show you three choices for the creation
of special purpose lanes: Sidewalk, Bikelane, and Buslane. Add one lane
for each type. ![addSpecialLane.png](../images/AddSpecialLane.png
"Adding special lanes")

To create a tram, we add a new lane by clicking on "Duplicate lane" in
the same context menu. For that lane, we have to restrict the allowed
vehicle class to trams. To do this, first uncheck the "select edges"-box
just right of the edit mode dropdown menu in the toolbar (the mode
should still be set to "(i)Inspect"). Then click on the newly created
lane and on the button "allow" in the inspector panel. This opens a
dialog with check boxes for all possible vehicle classes. Uncheck all
but "rail_urban" and click on "accept". Now edit the allowances for the
remaining lane (it is currently allowed for all vehicle classes) and
reserve it to the class "passenger" (i.e. passenger
cars).![EditVClassRestrictions.png](../images/EditVClassRestrictions.png
"Editing vehicle class restrictions")

Now let us split the edge to create a starting point for the
competitors: Right-click somewhere on the edge and select "Split edge
here" from the context menu. Then click on the created node (in SUMO
terminology this is already a "junction"). Set its x-coordinate to 900
and its y-coordinate to 0 in the `pos`-field just as you did above when
creating the edge. Effectively, we have created a 100 meter running
track for the competitors with a 900 meter holding area for each of the
competing modes. Now check the check box "select edges" again and rename
the two edges to "beg" and "end" (in the inspector panel). Save your
network (`Ctrl-S`).

# Defining Competing Vehicle Types

As a next step, we define the competing vehicle types. Open a new file
called `sumolympics.rou.xml` and insert the following vehicle type
definitions:

```
<routes>
    <vType id="pkw" length="5" maxSpeed="50" accel="2.6" decel="4.5" sigma="0.2" speedDev="0.2" vClass="passenger"/>
    <vType id="bus" length="15" maxSpeed="30" accel="1.2" decel="2.5" sigma="0.1" speedDev="0.1" vClass="bus"/>
    <vType id="tram" length="40" maxSpeed="13" accel="0.8" decel="0.5" sigma="0.1" speedDev="0.1" vClass="rail_urban"/>
    <vType id="bike" length="1.8" width="0.8" maxSpeed="7.5" accel="0.8" decel="1.5" sigma="0.5" speedDev="0.5" vClass="bicycle"/>
</routes>
```

Take a look at the [vehicle type attributes
description](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types)
for details on these definitions.

For each vehicle type, we schedule and position vehicles transporting
100 people by adding the following `<flow .../>` elements just below the
vType definitions (within the `<routes>` element\!):

```
        ...
    <flow id="pkw" type="pkw" from="beg" to="end" begin="0" end="0" number="66" departPos="last"/>
    <flow id="bus" type="bus" from="beg" to="end" begin="0" end="0" number="5" departPos="last"/>
    <flow id="tram" type="tram" from="beg" to="end" begin="0" end="0" number="2" departPos="last"/>
    <flow id="bike" type="bike" from="beg" to="end" begin="0" end="0" number="100" departPos="last"/>
        ...
```

Note that this assumes that the average vehicle occupation is 1.5.[\[1\]](#references)
For details on the meaning of the attributes of the flows, see the
section [Flow
Definitions](../Demand/Shortest_or_Optimal_Path_Routing.md#flow_definitions).

To start the simulation, create a SUMO configuration file (name it
`sumolympics.sumocfg`):

```
<configuration>
   <input>
       <net-file value="sumolympics.net.xml"/>
       <route-files value="sumolympics.rou.xml"/>
   </input>
   <processing>
    <lateral-resolution value="1." />
   </processing>
</configuration>
```

![sumolympic_run1.png](../images/Sumolympic_run1.png "First test run without pedestrians and start signals")Here
we give the processing argument `lateral-resolution` with a value
corresponding to the sub-lane width in meters to achieve a more
realistic behavior of bicyclists utilizing the whole lane width to
overtake each other (see [Sublane
Model](../Simulation/SublaneModel.md) and [Bicycle
simulation](../Simulation/Bicycles.md)). Start the simulation by
double-clicking on the configuration file `sumolympics.sumocfg`
(Windows) or running `sumo-gui -c sumolympics.sumocfg` from a terminal.
Adjust the step delay to 100 ms and press the run button
(![Image:play.gif](../images/Play.gif "Image:play.gif")).

# Defining a Start Signal (Traffic Light) and Pedestrians

There are two things left to do for a fair and complete competition: 1)
All competitors should be allowed to position freely in front of the
scratch line (the bicyclists are inserted in a row, though they could
achieve a much better result by grouping more densely using the whole
lane width) 2) We wish to include pedestrians into the competition.

![sumolympics_TLSediting.png](../images/Sumolympics_TLSediting.png
"Editing traffic lights")First we create a traffic light on the
junction between the edges "beg" and "end" with netedit: Press `t` to
enter the traffic light editing mode. Click on the junction, then on
"Create TLS" in the left panel. Below, under the label phases, type
"rrrrr" for the first phase ("r" for red) and set its duration to 100
(secs.). This will give enough time for the bicyclists to group more
densely. For the second phase enter "GGGGG" (yes, "G" for green) and set
its duration to 1000 (i.e. until the end of the simulation run). Now run
the simulation again to see the bikes outrun the cars. See? We should
all use our bikes more often\!

If you have noticed a warning (like "Warning: Missing yellow phase in
tlLogic 'gneJ2', program '0' for tl-index 0 when switching to phase 0")
in the Message Window, don't worry. SUMO routinely checks tls-phases for
basic consistency and missing yellow phases may lead to crashes if you
have intersecting flows. However, this is a special situation and we
don't need to care about this, obviously. If you want to learn more
about traffic light control, see the TraCI-Tutorials
[TraCIPedCrossing](../Tutorials/TraCIPedCrossing.md) and
[TraCI4Traffic_Lights](../Tutorials/TraCI4Traffic_Lights.md) or
the main section on [traffic
lights](../Simulation/Traffic_Lights.md).

What do you think, will pedestrians be slower or faster? Let's see. You
can already guess that the approach is a little different for
pedestrians. This is because they are no vehicle class (not any more),
but constitute an own class called "person". For instance, there is no
such element as a person flow analogous to vehicle flows, yet (though it
is coming, see #1515). So, we are going to write a python script to generate
a routefile `sumolympic_walking.rou.xml`. (Note that there is a little
script in the {{SUMO}}/tools folder called `pedestrianFlow.py`, which can be
useful if you would like to do more sophisticated things.)

Here's the simple script (call the file something like
`makeSumolympicWalkers.py`):

```
#!/usr/bin/python
#parameters
outfile = "sumolympicWalks.rou.xml"
startEdge = "beg"
endEdge = "end"
departTime = 0. #time of departure
departPos = -30. #position of departure
arrivalPos = 100. #position of arrival
numberTrips = 100 #number of persons walking
#generate XML
xml_string = "<routes>\n"  
for i in range(numberTrips):
    xml_string += '    <person depart="%f" id="p%d" departPos="%f" >\n' % (departTime, i, departPos)
    xml_string += '        <walk edges="%s %s" arrivalPos="%f"/>\n' % (startEdge, endEdge, arrivalPos)
    xml_string += '    </person>\n'
xml_string += "</routes>\n"
with open(outfile, "w") as f:
    f.write(xml_string)
```

Execute the script by double-clicking (or from the command line with
`python makeSumolympicWalkers.py`). If you don't have python on your
computer, install it before doing anything else\!
([get it from here](https://www.python.org/downloads/)) We have to
include the generated route file `sumolympicWalks.rou.xml` in the config
file `sumolympic.sumocfg` to let the simulation know about them. Several
route files can be included by merely separating them by a comma.
Therefore, modify the `<route-files .../>`-entry of our config to look
like this (be sure to put no spaces between the filenames\!):

```
    ...
    <route-files value="sumolympics.rou.xml,sumolympicWalks.rou.xml"/>
    ...
```

Get the popcorn and start the simulation\!

# References

1.  <https://www.forschungsinformationssystem.de/servlet/is/79638/>

Back to [Tutorials](../Tutorials.md).