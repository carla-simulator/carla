---
title: Tutorials/TraCIPedCrossing
permalink: /Tutorials/TraCIPedCrossing/
---

This tutorial shows how to use the Traffic Control Interface (in short
TraCI) for building a pedestrian-actuated crossing. It assumes
familiarity with
[Tutorials/TraCI4Traffic_Lights](../Tutorials/TraCI4Traffic_Lights.md)
which builds a vehicle-actuated controller. Additional information in
regard to pedestrian modelling can be found at
[Simulation/Pedestrians](../Simulation/Pedestrians.md).

All files mentioned here can also be found in the
{{SUMO}}*/docs/tutorial/traci_pedestrian* directory of your installation. The
most recent version can be found in the repository at [{{SUMO}}/tests/complex/tutorial/traci_pedestrian_crossing/]({{Source}}tests/complex/tutorial/traci_pedestrian_crossing/).

# Example description

The scenario consists of a single road which is crossed by a footpath.
The crossing is controlled by a traffic light and should switch when
triggered by the pedestrians via push-button. To model the
button-pushing behavior, it is assumed that a pedestrian which arrives
at a red light activates the button if it is not yet activated. To
prevent undue interference with vehicular traffic, there is a minimum
green duration of 15 seconds for the vehicular green phase.

![Image:ScreenshotPedCross.png](../images/ScreenshotPedCross.png
"Image:ScreenshotPedCross.png")

To run the example you need to execute the script *runner.py* with
python

```
python runner.py
```

!!! caution
    You need to press start in the simulation gui to run the tutorial.

## Code

The control logic resides in the python script *runner.py*. It builds
the network (via [netconvert](../netconvert.md)) generates the
pedestrian routes (via
[\[1\]](http://sumo.dlr.de/wiki/Tools/Trip#randomTrips.py%7CrandomTrips.py),
and interacts with the simulation to controls the traffic light. It
makes use of the TraCI python API bundled with SUMO. A description of
the API can be found at
[TraCI/Interfacing_TraCI_from_Python](../TraCI/Interfacing_TraCI_from_Python.md).
For a detailed list of available functions see the [pydoc generated
documentation](http://sumo.dlr.de/daily/pydoc/traci.html).

## Traffic light control

The traffic light follows a fixed cycle of phases of as defined below.
The definition below is taken from
[pedcrossing.tll.xml]({{Source}}tests/complex/tutorial/traci_pedestrian_crossing/data/pedcrossing.tll.xml)

```
<tlLogic id="C" type="static" programID="custom" offset="0">
  <phase duration="100000" state="GGGGr"/>
  <phase duration="4" state="yyyyr"/>
  <phase duration="10" state="rrrrG"/>
  <phase duration="10" state="rrrrr"/>
</tlLogic>
```

The duration of phase 0 is set to a very large value. This is the green
phase for vehicles and should be prolonged indefinitely in the absence
of pedestrians. When waiting pedestrians are detected while in phase 0,
the traffic light is switched into the next phase and then continues its
cycle automatically. The only traffic light related TraCI functions used
are *traci.trafficlights.getPhase()* and traci.trafficlights.setPhase().

## Person related TraCI functions

To simulate the push button it must be known whether pedestrians are
waiting to cross the intersection. The following TraCI functions are
used:

- *traci.edge.getLastStepPersonIDs()* : to find pedestrians at the
  intersection
- *traci.person.getNextEdge()*: to determine whether pedestrians are
  about to pass the crossing or are already leaving the intersection
- *traci.person.getWaitingTime()*: to determine whether the pedestrian
  has arrived at the crossing and is forced to wait

The input to *traci.edge.getLastStepPersonIDs()* as well as the output
of *traci.person.getNextEdge()* references network elements which are
specific to pedestrian simulation. In the script *runner.py* these are
defined as

```
WALKINGAREAS = [':C_w0', ':C_w1']
CROSSINGS = [':C_c0']
```

For a description of pedestrian network elements see
[Simulation/Pedestrians](../Simulation/Pedestrians.md).