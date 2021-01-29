---
title: Simulation/Output/RouteProbe
permalink: /Simulation/Output/RouteProbe/
---

RouteProbe detectors are meant to determine the route distribution for
all vehicles that passed an edge in a given interval. Their real-world
equivalent would be a police stop with questionnaire or a look into the
database of navigation providers such as
[TomTom](http://www.tomtom.com/).

## Instantiating within the Simulation

RouteProbe Detectors have to be defined within an {{AdditionalFile}} which has to be
loaded by the simulation.

```
<additional>
  <routeProbe id="<ROUTEPROBE_ID>" edge="<EDGE_ID>" freq="<FREQUENCY>" file="<OUTPUT_XMLFILE>"/>
</additional>
```

The following attributes are used within the `routeProbe` element:

| Attribute Name | Value Type  | Description                                       |
| -------------- | ----------- | ------------------------------------------------- |
| **id**         | id (string) | The id of the routeProbe element (user choice)    |
| **edge**       | id (string) | The id of an edge in the simulation network       |
| **freq**       | time (s)    | The frequency in which to report the distribution |
| **file**       | string      | The file for generated output                     |
| **begin**      | time (s)    | The time at which to start generating output      |

## Generated Output

The output takes the form of a [route alternative
file](../../duarouter.md#outputs) which can be loaded directly in
[sumo](../../sumo.md)

```
<route-probes>
   <routeDistribution id="r3/1to2/1_80.00">
      <route id="r1_80.00" edges="4/1to3/1 3/1to2/1 2/1to1/1 1/1to0/1" probability="1.00"/>
   </routeDistribution>
</route-probes>
```

## Further Notes

RouteProbe elements can be used in conjunction with
[Calibrators](../../Simulation/Calibrator.md) where route
distributions are required when inserting new vehicles.