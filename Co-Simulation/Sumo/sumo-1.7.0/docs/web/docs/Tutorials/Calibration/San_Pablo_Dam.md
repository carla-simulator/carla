---
title: Tutorials/Calibration/San Pablo Dam
permalink: /Tutorials/Calibration/San_Pablo_Dam/
---

This tutorial describes the execution of the simulation in a loop using
a control script. The aim is to calibrate the simulation to a given
real-world dataset. This tutorial may be used for further research on
car-following models or on optimization algorithms.

It is assumed that road network building and routes definition is known
from other tutorials, as [Tutorials/Hello
SUMO](../../Tutorials/Hello_SUMO.md), [Tutorials/quick
start](../../Tutorials/quick_start.md) or [Tutorials/Quick Start old
style](../../Tutorials/Quick_Start_old_style.md).

All files mentioned here can also be found in the
{{SUMO}}/docs/tutorial/san_pablo_dam directory. The most recent version can be
found in the repository at [{{SUMO}}/tests/complex/tutorial/san_pablo_dam/data/]({{Source}}tests/complex/tutorial/san_pablo_dam/data/).

# Description

Models for vehicle movement have usually several parameters which
control the behaviour of the vehicle. Their default values are usually
set based on assumptions or measures or by adapting them to a given data
set. When moving to a different scenario, they often have to be
re-adapted for being valid. Due to this, calibration is a crucial step
in preparing a traffic simulation scenario.

This tutorial shows one possibility to calibrate a car-following model
to match a set of data gained from the real world. The data set we use
was collected on the San Pablo Dam Road "from 6:45 a.m. to 9:00 a.m. on
Tuesday, November 18 and again on Thursday, November 20, 1997"
(\[Smilowitz1999\]). Please look [here
(OpenStreetMap)](http://www.openstreetmap.org/?lat=37.9191&lon=-122.2439&zoom=13&layers=M)
or [here
(GoogleMaps)](http://maps.google.de/maps?q=San+Pablo+Reservoir,+Contra+Costa,+Kalifornien,+Vereinigte+Staaten&hl=de&ll=37.923482,-122.244015&spn=0.097632,0.118618&sll=51.151786,10.415039&sspn=19.919551,30.366211&vpsrc=6&geocode=FZu4QgId6rO2-A&t=m&z=13)
for the location. The data set's pages are
[here](http://www.ce.berkeley.edu/~daganzo/spdr.html). It was also used
for benchmarking car-following models, see \[Brockfeld2003a\]. The data
set consists of times at which vehicles pass count points and is assumed
to be quite clean.

Having the passing times of vehicles, we want to calibrate our
car-following model so that the difference between real and simulated
travel times across all vehicles is minimized.

In order to obtain the passing times from the simulation, we use
[induction
loops](../../Simulation/Output/Induction_Loops_Detectors_(E1).md)
with a frequency of 1s. They are defined in "input_det.add.xml".
Additionally, we use a [variable speed
sign](../../Simulation/Variable_Speed_Signs.md) for constraining the
outflow velocity so that the original (real world) network's outflow
condition is preserved.

## Prerequisites

In order to execute this tutorial, you need

- a runnable SUMO simulation
- Python 2.x (tested with Python 2.7)
  - SciPy

# Network Preparation

Optimization requires several - many - iterations, and
[sumo](../../sumo.md)'s execution speed highly depends on the number
of edges a network is made of. Due to this we model the San Pablo Dam
Road using two edges only. We build an edge file, and a node file as
already discussed in previous ([Tutorials/Hello
SUMO](../../Tutorials/Hello_SUMO.md), [Tutorials/Quick
Start](../../Tutorials/Quick_Start_old_style.md)) tutorials.

# Demand Preparation

We use the function genDemand in "runner.py" for building the demand.
Here, times at observation point 1 are used as our vehicle departure
times. The route consists of the two edges the network consists of. All
vehicles have the same type. The values for this type - the
car-following parameter to optimize - are documented in the function gof
in "runner.py". Please note that we keep "minGap" at 2.5m constantly -
this should be changed for other models than the used SUMO-Krauß-model.
If wished, they may be set to the default parameter values before
performing the calibration (see the end of runner.py).

# Simulation Settings

Our configuration looks like this:

```
<configuration>
    <input>
        <net-file value="spd-road.net.xml"/>
        <route-files value="spd-road.rou.xml"/>
        <additional-files value="input_vss.add.xml,input_det.add.xml,input_types.add.xml"/>
    </input>
    <time>
        <begin value="24420"/>
    </time>
        <processing>
        <time-to-teleport value="0"/>
    </processing>
    <report>
        <no-duration-log value="true"/>
        <no-step-log value="true"/>
    </report>
</configuration>
```

This means: we load the network from "spd-road.net.xml", routes from
"spd-road.rou.xml", and three additional files, "input_vss.add.xml"
including the variable speed sign, "input_det.add.xml" containing
definitions of induction loops to simulate, and "input_types.add.xml"
containing the definition of our current vehicle type. The begin time is
set to the departure time of the first vehicle. We ignore possible
waiting times by setting **time-to-teleport** to 0 and disable simulation outputs.

Please note that "spd-road.rou.xml" is created from the input
measurements on the start of runner.py, "input_types.add.xml" is
created on every simulation loop with the new parameters and all the
other files are completely static.

# Calibration

We use SciPy's "COBYLA" implementation. It requires callbacks for
determining the error which we have to supply. Our callback (function
`gof` in "runner.py") works as following:

1.  Write the current vehicle type with the parameters supported by the
    optimizer into a file named "input_types.add.xml"
2.  Start the validation script "validate.py" and return the error value
    computed by it

The validation step, implemented in "validate.py" is not much more
complicated:

1.  Execute the simulation
2.  Read the real-world observations and the vehicle crossing times from
    the simulation
3.  Convert both from observation times to travel times
4.  Compute the RMSE (root mean square error) between both for all
    vehicles and observation points and return it

# Execution

In order to perform the calibration, you need to call only:

```
python runner.py
```

This is what it is doing:

- first it calls netconvert -n=spd-road.nod.xml -e=spd-road.edg.xml
  -o=spd-road.net.xml

  Build the network using nodes from "spd-road.nod.xml" and edges from
  "spd-road.edg.xml"; write to "spd-road.net.xml"; generates
  "spd-road.net.xml"

- calls the function buildVSS

  Build the speed limits for the end boundary; generates
  "spd-road.vss.xml" which is referenced by "input_vss.add.xml"

- Starts the calibration

For each calibration step, the following output should appear:

```
# simulation with: vMax:22.000 aMax:2.000 bMax:2.000 lCar:5.000 sigA:0.500 tTau:1.500
Loading configuration... done.
#### yields rmse: 212.6411
```

Of course, the values differ between the steps. 80 iterations need about
ten minutes to be executed.

"runner.py" generates a file named "results.csv" which includes for each
iteration the parameter and the error, line by line.

# References

\[Smilowitz1999\] K. Smilowitz, C. Daganzo, M.J. Cassidy and R.L.
Bertini. 1999. Some observations of highway traffic in long queues.
Transportation Research Records, 1678, pp. 225-233; available at
[\[1\]](http://www.its.berkeley.edu/publications/UCB/98/RR/UCB-ITS-RR-98-6.pdf)

\[Brockfeld2003a\] E. Brockfeld, R. Kühne, A. Skabardonis, P. Wagner.
2003 Towards a benchmarking of Microscopic Traffic Flow Models.
Transportation Research Records, 1852 (TRB2003-001164), pp. 124-129;
available at [\[2\]](http://elib.dlr.de/6646/)