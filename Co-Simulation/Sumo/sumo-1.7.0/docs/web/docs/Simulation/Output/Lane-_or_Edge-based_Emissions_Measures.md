---
title: Simulation/Output/Lane- or Edge-based Emissions Measures
permalink: /Simulation/Output/Lane-_or_Edge-based_Emissions_Measures/
---

The edge/lane based vehicular emission output writes absolute and normed
values of vehicular pollutant emissions collected on edges or lanes. The
absolute values hold the sum of each of the pollutants emitted on each
edge/lane, the normed values give the values normed by the interval
duration and the edge's/lane's length.

Please note, that each vehicle type in SUMO may belong to a certain
emission class (see [Definition of Vehicles, Vehicle Types, and
Routes](../../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md)
and [Models/Emissions](../../Models/Emissions.md)). When using the
emissions output, you should assure your vehicles reflect a realistic
population in the means of vehicle emission class distribution.

## Instantiating within the Simulation

### Edge-Based Emission Output

An edge-based emissions output is defined way within an {{AdditionalFile}} as following:

```
<edgeData id="<DETECTOR_ID>" type="emissions" freq="<FREQUENCY>" file="<OUTPUT_FILE>" [excludeEmpty="true"]/>
```

### Lane-Based Emission Output

A lane-based emissions output is defined way within an {{AdditionalFile}} as following:

```
<laneData id="<DETECTOR_ID>" type="emissions" freq="<FREQUENCY>" file="<OUTPUT_FILE>" [excludeEmpty="true"]/>
```

### Attributes, for both Edge- and Lane-Based Emission Output

| Attribute Name | Value Type                     | Description        |
| -------------- | ------------------------------ | --------------------------------------- |
| **id**         | id (string)                    | The id of the detector             |
| **file**       | filename                       | The path to the output file. The path may be relative.     |
| freq           | int (time)                     | The aggregation period the values the detector collects shall be summed up. If not given, the whole time range between begin and end is aggregated   |
| begin          | int (time)                     | The time to start writing (intervals starting before this time are discarded). If not given, the simulation's begin is used.    |
| end            | int (time)                     | The time to end writing (intervals starting at or after this time are discarded). If not given the simulation's end is used.     |
| withInternal   | bool                     | If set, junction internal edges/lanes will be written as well; *default: false*.  |
| maxTraveltime  | float (time)                   | The maximum traveltime in seconds to write if only very small movements occur; *default 100000*.   |
| minSamples     | float (time)                   | The minimum total number of seconds vehicles have to be on the edge / lane to consider it non-empty; *default: \>0*.    |
| excludeEmpty   | string (true, false, defaults) | If set to true, edges/lanes which were not use by a vehicle during this period will not be written; *default: false*. If set to "defaults" default values for travel time and emissions depending on edge length and maximum speed get printed. |
| vTypes         | string                   | space separated list of vehicle type ids to consider, "" means all; *default ""*. |

## Generated Output

### Edge-Based Network States

### Lane-Based Network States

### Value Descriptions

| Name                | Type                 | Description                     |
| ------------------- | -------------------- | --------------------------------------------------- |
| begin               | (simulation) seconds | The first time step the values were collected in   |
| end                 | (simulation) seconds | The last time step + DELTA_T in which the reported values were collected |
| edge\@id             | (edge) id            | The name of the reported edge         |
| lane\@id             | (lane) id            | The name of the reported lane             |
| sampledSeconds      | s                    | Number seconds vehicles were measured on the edge/lane (may be subseconds if a vehicle enters/leaves the edge/lane). This value is the sum of the measured times of all vehicles. |
| traveltime          | s                    | Time needed to pass the edge/lane, note that this is just an estimation based on the mean speed, not the exact time the vehicles needed.                                          |
| CO_abs             | mg                   | The complete amount of CO emitted by the vehicles on this lane/edge during the aggregation period                                                                                 |
| CO2_abs            | mg                   | The complete amount of CO<sub>2</sub> emitted by the vehicles on this lane/edge during the aggregation period                                                                     |
| HC_abs             | mg                   | The complete amount of HC emitted by the vehicles on this lane/edge during the aggregation period                                                                                 |
| PMx_abs            | mg                   | The complete amount of PM<sub>x</sub> emitted by the vehicles on this lane/edge during the aggregation period                                                                     |
| NOx_abs            | mg                   | The complete amount of NO<sub>x</sub> emitted by the vehicles on this lane/edge during the aggregation period                                                                     |
| fuel_abs           | ml                   | The complete amount of fuel the vehicles on this lane/edge during the aggregation period have consumed                                                                            |
| electricity_abs    | Wh                   | The complete amount of electricity the vehicles on this lane/edge during the aggregation period have consumed                                                                     |
| CO_normed          | g/km/h               | CO emissions during this interval normed by time and edge/lane length                                                                                                             |
| CO2_normed         | g/km/h               | CO<sub>2</sub> emissions during this interval normed by time and edge/lane length                                                                                                 |
| HC_normed          | g/km/h               | HC emissions during this interval normed by time and edge/lane length                                                                                                             |
| PMx_normed         | g/km/h               | PM<sub>x</sub> emissions during this interval normed by time and edge/lane length                                                                                                 |
| NOx_normed         | g/km/h               | NO<sub>x</sub> emissions during this interval normed by time and edge/lane length                                                                                                 |
| fuel_normed        | l/km/h               | fuel consumption during this interval normed by time and edge/lane length                                                                                                         |
| electricity_normed | W/km                 | electricity consumption during this interval normed by time and edge/lane length                                                                                                  |
| CO_perVeh          | mg                   | Assumed CO emissions a vehicle would produce when passing the edge                                                                                                                |
| CO2_perVeh         | mg                   | Assumed CO<sub>2</sub> emissions a vehicle would produce when passing the edge                                                                                                    |
| HC_perVeh          | mg                   | Assumed HC emissions a vehicle would produce when passing the edge                                                                                                                |
| PMx_perVeh         | mg                   | Assumed PM<sub>x</sub> emissions a vehicle would produce when passing the edge                                                                                                    |
| NOx_perVeh         | mg                   | Assumed NO<sub>x</sub> emissions a vehicle would produce when passing the edge                                                                                                    |
| fuel_perVeh        | ml                   | Assumed fuel consumption a vehicle would need when passing the edge                                                                                                               |
| electricity_perVeh | Wh                   | Assumed electricity consumption a vehicle would need when passing the edge     |

## Notes

Notes:

- Per default, all edges are written, even those on which no vehicle
  drove. It can be disabled setting the
  `excludeEmpty` attribute to true.
- The interval end is the interval begin + aggregation time, meaning
  that values were collected within these steps excluding the end time
  itself. If the simulation ends before the last interval is over, the
  interval will be pruned.
- The output file will be generated, does not have to exist earlier
  and will be overwritten if existing without any warning. The folder
  the output file shall be generated in must exist.
- If you need only information about the network states during certain
  time periods, you may constraint generation of the dumps by giving
  attributes "`begin="<TIME>\[,<TIME>\]+"`"
  and "`end="<TIME>\[,<TIME>\]+"`". When at
  least one combination is given, dumps will be written only if an
  according begin/end-pair exists for the current time. This means,
  only those intervals will be saved for which
  begin\[x\]<=INTERVAL_END and end\[x\]\>=INTERVAL_BEGIN. All dumps
  will cover the complete simulation if no values for begin/end are
  given.

## See Also

- [edge/lane-based network performace measures
  output](../../Simulation/Output/Lane-_or_Edge-based_Traffic_Measures.md)
  and [edge/lane-based vehicular noise emission
  output](../../Simulation/Output/Lane-_or_Edge-based_Noise_Measures.md)
  which have similar formats
- The
  [mpl_dump_onNet.py](../../Tools/Visualization.md#mpl_dump_onnetpy)
  script can display values of this output as a colored net (and
  further [visualization tools](../../Tools/Visualization.md)
  exist).

## Discussion

### Omitting empty Edges/Lanes

**17.07.2009**: I have excluded computation of mean per-vehicle emission
to pass a lane/edge though almost the same functionality is already
covered by the "excludeEmpty"-attribute. I am not sure whether this
makes sense.

Also, we maybe should then also decide whether or not this also should
be done for travel time computation.

### Value Range

**17.07.2009**: The values generated for absolute (..._abs) emission
during the period and for the mean per-vehicle emission to pass a
lane/edge are now given in mg for making lower values visible though
only digits after the dot are used. I am not really happy with the
results. If a vehicle passes a lane of 390m length at 10m/s, it is
assumed it emits less CO than 0.005mg so it does not appear in the
outputs; On the other hand, if one looks at a jammed situation, the
emissions of CO2 may climb up to CO_perVeh="21246834688.00" (sure, they
would get infinite if the vehicle would be really standing). I suppose we
have to evaluate how the system behaves.