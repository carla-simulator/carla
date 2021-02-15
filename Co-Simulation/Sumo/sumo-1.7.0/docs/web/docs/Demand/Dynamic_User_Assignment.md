---
title: Demand/Dynamic User Assignment
permalink: /Demand/Dynamic_User_Assignment/
---

# Introduction

For a given set of vehicles with of origin-destination relations
(trips), the simulation must determine routes through the network (list
of edges) that are used to reach the destination from the origin edge.
The simplest method to find these routes is by computing shortest or
fastest routes through the network using a routing algorithm such as
Djikstra or A\*. These algorithms require assumptions regarding the
travel time for each network edge which is commonly not known before
running the simulation due to the fact that travel times depend on the
number of vehicles in the network.

!!! caution
    A frequent problem with naive user assignment is that all vehicles take the fastest path under the assumption that they are alone in the network and are then jammed at bottlenecks due to the sheer amount of traffic.

The problem of determining suitable routes that take into account travel
times in a traffic-loaded network is called *user assignment*. SUMO
provides different tools to solve this problem and they are described
below.

# Iterative Assignment (**D**ynamic **U**ser **E**quilibrium)

The tool {{SUMO}}*/tools/assign/duaIterate.py* can be used to compute the
(approximate) dynamic user equilibrium.

!!! caution
    This script will require copious amounts of disk space

```
python duaIterate.py -n <network-file> -t <trip-file> -l <nr-of-iterations>
```

*duaIterate.py* supports many of the same options as
[sumo](../sumo.md). Any options not listed when calling
*duaIterate.py* ***--help*** can be passed to [sumo](../sumo.md) by adding **sumo--long-option-name arg**
after the regular options (i.e. **sumo--step-length 0.5**).

This script tries to calculate a user equilibrium, that is, it tries to
find a route for each vehicle (each trip from the trip-file above) such
that each vehicle cannot reduce its travel cost (usually the travel
time) by using a different route. It does so iteratively (hence the
name) by

1.  calling [duarouter](../duarouter.md) to route the vehicles in a
    network with the last known edge costs (starting with empty-network
    travel times)
2.  calling [sumo](../sumo.md) to simulate "real" travel times
    result from the calculated routes. The result edge costs are used in
    the net routing step.

The number of iterations may be set to a fixed number of determined
dynamically depending on the used options. In order to ensure
convergence there are different methods employed to calculate the route
choice probability from the route cost (so the vehicle does not always
choose the "cheapest" route). In general, new routes will be added by
the router to the route set of each vehicle in each iteration (at least
if none of the present routes is the "cheapest") and may be chosen
according to the route choice mechanisms described below.

Between successive calls of duarouter, the *.rou.alt.xml* format is used
to record not only the current *best* route but also previously computed
alternative routes. These routes are collected within a route
distribution and used when deciding the actual route to drive in the
next simulation step. This isn't always the one with the currently
lowest cost but is rather sampled from the distribution of alternative
routes by a configurable algorithm described below.

## Route-Choice algorithm

The two methods which are implemented are called
[Gawron](../Publications.md#traffic_assignment) and
[Logit](https://en.wikipedia.org/wiki/Discrete_choice) in the following.
The input for each of the methods is a weight or cost function \(w\) on
the edges of the net, coming from the simulation or default costs (in
the first step or for edges which have not been traveled yet), and a set
of routes <img src="http://latex.codecogs.com/gif.latex?R" border="0" style="margin:0;"/> where each route <img src="http://latex.codecogs.com/gif.latex?r" border="0" style="margin:0;"/> has an old cost <img src="http://latex.codecogs.com/gif.latex?c_r" border="0" style="margin:0;"/> and an
old probability <img src="http://latex.codecogs.com/gif.latex?p_r" border="0" style="margin:0;"/> (from the last iteration) and needs a new cost
<img src="http://latex.codecogs.com/gif.latex?c_r'" border="0" style="margin:0;"/> and a new probability <img src="http://latex.codecogs.com/gif.latex?p_r'" border="0" style="margin:0;"/>.

### Gawron (default)

The Gawron algorithm computes probabilities for choosing from a set of
alternative routes for each driver. The following values are considered
to compute these probabilities:

- the travel time along the used route in the previous simulation step
- the sum of edge travel times for a set of alternative routes
- the previous probability of choosing a route

### Logit

The Logit mechanism applies a fixed formula to each route to calculate
the new probability. It ignores old costs and old probabilities and
takes the route cost directly as the sum of the edge costs from the last
simulation.

<img src="http://latex.codecogs.com/gif.latex?c_r' = \sum_{e\in r}w(e)" border="0" style="margin:0;"/>

The probabilities are calculated from an exponential function with
parameter <img src="http://latex.codecogs.com/gif.latex?\theta" border="0" style="margin:0;"/> scaled by the sum over all route values:

<img src="http://latex.codecogs.com/gif.latex?p_r' = \frac{\exp(\theta c_r')}{\sum_{s\in R}\exp(\theta c_s')}" border="0" style="margin:0;"/>

## Termination

The option **--max-convergence-deviation** may be used to detect convergence and abort iterations
automatically. Otherwise, a fixed number of iterations is used. Once the
script finishes any of the resulting *.rou.xml* files may be used for
simulation but the last one(s) should be the best.

## Usage Examples

### Loading vehicle types from an additional file

By default, vehicle types are taken from the input trip file and are
then propagated through [duarouter](../duarouter.md) iterations
(always as part of the written route file).

In order to use vehicle type definitions from an {{AdditionalFile}}, further options must
be set

```
duaIterate.py -n ... -t ... -l ... 
  --additional-file <FILE_WITH_VTYPES> 
  duarouter--aditional-file <FILE_WITH_VTYPES> 
  duarouter--vtype-output dummy.xml
```

Options preceded by the string *duarouter--* are passed directly to
duarouter and the option *vtype-output dummy.xml* must be used to
prevent duplicate definition of vehicle types in the generated output
files.

# oneShot-assignment

An alternative to the iterative user assignment above is incremental
assignment. This happens automatically when using `<trip>` input directly in
[sumo](../sumo.md) instead of `<vehicle>`s with pre-defined routes. In this
case each vehicle will compute a fastest-path computation at the time of
departure which prevents all vehicles from driving blindly into the same
jam and works pretty well empirically (for larger scenarios).

The routes for this incremental assignment are computed using the
[Automatic Routing / Routing
Device](../Demand/Automatic_Routing.md) mechanism. Since this
device allows for various configuration options, the script
[Tools/Assign\#one-shot.py](../Tools/Assign.md#one-shotpy) may be
used to automatically try different parameter settings.

# [marouter](../marouter.md)

The [marouter](../marouter.md) application computes a *classic*
macroscopic assignment. It employs mathematical functions (resistive
functions) that approximate travel time increases when increasing flow.
This allows to compute an iterative assignment without the need for
time-consuming microscopic simulation.