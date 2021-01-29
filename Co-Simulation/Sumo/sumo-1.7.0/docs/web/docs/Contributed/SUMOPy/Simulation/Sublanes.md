---
title: Contributed/SUMOPy/Simulation/Sublanes
permalink: /Contributed/SUMOPy/Simulation/Sublanes/
---

Sublanes is a new feature in SUMO since 2015. The simulation with
Sublanes render traffic flows on roads more realistic. With sublanes,
several vehicles can share side-by-side the same lane given there is
enough room. For example a car can pass a bicycle on the same lane if
the total lane width is larger than the bike width plus car width.

For sublane simulation with SUMOPy, simply browse to

```
scenario.demand.vtypes
```

and set the lanechange model model to SL2015. Note that SUMOPy applies
the same lanechange model to all vehicle types. Then run the simulation
with

```
Simulation>Sumo>export routes and simulate...
```

Note that on the SUMO dialog, the sublane width is now positive (1m by
default). This value (which can be changed) determines how many sublanes
can stay within one lane. For example a 3m wide lane can have 3 sublane
of 1m but only 2 sublane of width 1.5m.

The lanechange behavior with sublanes can be tweaked for each vehicle
type with the sublane parameters in `scenario.demand.vtypes`.