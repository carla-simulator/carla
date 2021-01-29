---
title: Tutorials/Trace File Generation
permalink: /Tutorials/Trace_File_Generation/
---

# Introduction

This tutorial describes how to generate vehicular trace files. Besides
SUMO, the [Tools/TraceExporter](../Tools/TraceExporter.md) will be
used. It requires Python to be executed and currently allows to generate
trace files for ns2/ns3, Shawn, OMNET, and PHEM.

# Simulation Scenario

For generating vehicular traces, You need a simulation scenario,
consisting at least of a road network and routes. You may visit
[Tutorials/Hello SUMO](../Tutorials/Hello_SUMO.md),
[Tutorials/quick start](../Tutorials/quick_start.md) or
[Tutorials/Quick Start old
style](../Tutorials/Quick_Start_old_style.md) to learn how to get a
scenario.

# Generating a SUMO-trace

We assume You have the scenario and a configuration file named
"myConfig.sumocfg". It may have a different name, of course. You should
be able to start SUMO using Your configuration like this:

```
sumo -c myConfig.sumocfg
```

Now, we use the simulation to get an [fcd
output](../Simulation/Output/FCDOutput.md), a trace file in a
SUMO-format. This file will be later converted into a trace file for one
of the applications supported by
[Tools/TraceExporter](../Tools/TraceExporter.md). We generate an
[fcd output](../Simulation/Output/FCDOutput.md) as following:

```
sumo -c myConfig.sumocfg --fcd-output sumoTrace.xml
```

And we will obtain the file "sumoTrace.xml".

# Converting the Trace

We can now convert the vehicular traces in SUMO format into another
format. For now, let's assume You would like to have a ns2 mobility
file. We can generate one from the obtained [fcd
output](../Simulation/Output/FCDOutput.md) using:

```
traceExporter.py --fcd-input sumoTrace.xml --ns2mobility-output ns2mobility.tcl
```

We obtain the file "ns2mobility.tcl", which we can give ns2 as input.

# Further Possibilities

## Output Options

[Tools/TraceExporter](../Tools/TraceExporter.md) generates files
for ns2/ns3, OMNET, Shawn, and PHEM. Please visit the
[Tools/TraceExporter](../Tools/TraceExporter.md) manual page for
further information about how to generate these files. Look for "output
options".

## Different Penetration Rates

[Tools/TraceExporter](../Tools/TraceExporter.md) allows to define
different equipment / penetration rates using the **--penetration** {{DT_FLOAT}}. {{DT_FLOAT}} will be the amount
of vehicles that are exported, where 1 is equal to 100 %. This means, if
You want to have 10 % of vehicles to show up in Your ns2-trace, use:

```
traceExporter.py --penetration 0.1 \
 --fcd-input sumoTrace.xml --ns2-mobilityoutput ns2mobility.tcl
```

The **--seed** {{DT_FLOAT}} allows You to give different random number initialisations for
choosing the vehicles to export.

## Limiting the Time

You may limit the time steps to export using the options **--begin** {{DT_TIME}} and **--end** {{DT_TIME}}. All time
steps before **--begin** and all after **--end** will be dismissed.

Additionally, You may force
[Tools/TraceExporter](../Tools/TraceExporter.md) to convert only
every n<sup>th</sup> time step using the option **--delta** {{DT_TIME}}. Please note that the
value should be a multiple of the time step length used within the
simulation.

# Further Information

- See the [Tools/TraceExporter](../Tools/TraceExporter.md) manual
  page
- See more information on SUMO's [fcd
  output](../Simulation/Output/FCDOutput.md)
- Visit tutorials describing how to set up a simulation:
  [Tutorials/Hello SUMO](../Tutorials/Hello_SUMO.md) or
  [Tutorials/Quick Start](../Tutorials/quick_start.md)