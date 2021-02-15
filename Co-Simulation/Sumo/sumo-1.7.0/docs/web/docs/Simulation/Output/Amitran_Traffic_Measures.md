---
title: Simulation/Output/Amitran Traffic Measures
permalink: /Simulation/Output/Amitran_Traffic_Measures/
---

The edge based Amitran output writes aggregated speed and flow values
using the format defined in
<https://sumo.dlr.de/xsd/amitran/linkdata.xsd>.

## Instantiating within the Simulation

An edge based Amitran output is defined way within an additional file as
following:

```
<edgeData id="<DETECTOR_ID>" type="amitran" freq="<FREQUENCY>"
file="<OUTPUT_FILE>" \[vTypes="<LIST>"\] \[excludeEmpty="true"\]/\>
```

| Attribute Name | Value Type                     | Description                                                   |
| -------------- | ------------------------------ | ------------------------------------------------------------- |
| **id**         | id (string)                    | The id of the detector                                        |
| **file**       | filename                       | The path to the output file. The path may be relative.        |
| freq           | int (time)                     | The aggregation period the values the detector collects shall be summed up. If not given, the whole time range between begin and end is aggregated                                                                                              |
| begin          | int (time)                     | The time to start writing (intervals starting before this time are discarded). If not given, the simulation's begin is used.                                                                                                                    |
| end            | int (time)                     | The time to end writing (intervals starting at or after this time are discarded). If not given the simulation's end is used.                                                                                                                    |
| vTypes         | string                         | A space separated ist of vehicle types to collect data for. If not given, collect for all types.        |
| excludeEmpty   | string (true, false, defaults) | If set to true, edges/lanes which were not use by a vehicle during this period will not be written; *default: false*. If set to "defaults" default values for travel time and emissions depending on edge length and maximum speed get printed. |

## Generated Output

```
<linkData xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="https://sumo.dlr.de/xsd/amitran/linkdata.xsd">
   <timeSlice startTime="0" duration="60000"/>
   <timeSlice startTime="60000" duration="60000">
       <link id="32" amount="2" averageSpeed="888"/>
       <link id="35" amount="2" averageSpeed="879"/>
       <link id="38" amount="4" averageSpeed="859"/>
   </timeSlice>
   ...
</linkData>
```

### Value Descriptions

| Name      | Type                      | Description                                          |
| --------- | ------------------------- | ---------------------------------------------------- |
| startTime | (simulation) milliseconds | The first time step the values were collected in     |
| duration  | (simulation) milliseconds | Length of the data collection interval               |
| link\@id   | (edge) id                 | The integer id of the reported edge                  |
| amount    | int                       | number of vehicles entering the edge in the interval |
| speed     | 0.01 m/s                  | average speed on the edge in the interval            |

## Notes

Notes:

- Per default, all edges are written, even those on which no vehicle
  drove. It can be disabled setting the
  `excludeEmpty` attribute to true.
- The interval end is the interval begin + aggregation time, meaning
  that values were collected within these steps excluding the end time
  itself. If the simulation ends before the last interval is over, the
  interval will be prunned.
- The output file will be generated, does not have to exist earlier
  and will be overwritten if existing without any warning. The folder
  the output file shall be generated in must exist.
- If you need only information about the network states during certain
  time periods, you may constraint generation of the dumps by giving
  attributes "`begin="[,]+"`" and "`end="[,]+"`". When at
  least one combination is given, dumps will be written only if an
  according begin/end-pair exists for the current time. This means,
  only those intervals will be saved for which
  begin[x]<=INTERVAL_END and end[x]>=INTERVAL_BEGIN. All dumps
  will cover the complete simulation if no values for begin/end are
  given.

## See Also

- [edge/lane-based network performance measures
  output](../../Simulation/Output/Lane-_or_Edge-based_Traffic_Measures.md)
  which has similar content

<div style="border:1px solid #909090; min-height: 35px;" align="right">
<span style="float: right; margin-top: -5px;"><a href="http://cordis.europa.eu/fp7/home_en.html"><img src="../../images/FP7-small.gif" alt="Seventh Framework Programme"></a>
<a href="http://amitran.eu/"><img src="../../images/AMITRAN-small.png" alt="AMITRAN project"></a></span>
<span style="">This part of SUMO was developed, reworked, or extended within the project 
<a href="http://amitran.eu/">"AMITRAN"</a>, co-funded by the European Commission within the <a href="http://cordis.europa.eu/fp7/home_en.html">Seventh Framework Programme</a>.</span></div>