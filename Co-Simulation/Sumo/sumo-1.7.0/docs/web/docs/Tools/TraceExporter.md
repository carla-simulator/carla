---
title: Tools/TraceExporter
permalink: /Tools/TraceExporter/
---

# From 30.000 feet

*traceExporter.py* converts and filters SUMO [fcd
output](../Simulation/Output/FCDOutput.md) to different "trace
file" formats: OMNET, Shawn, ns2/ns3, PHEM.

- **Purpose:** trace file conversion/generation
- **System:** portable (Linux/Windows is tested); runs on command line
- **Input (mandatory):** SUMO fcd-output
- **Output:** One or more "trace file(s)" and other complementary files
- **Programming Language:** Python

# Introduction

Different applications read "vehicular traces", files containing mainly
positions of vehicles over time.
**traceExporter.py** generates such files
by converting SUMO's [fcd
output](../Simulation/Output/FCDOutput.md) into several supported
formats. Following files can be generated, sorted by application:

- OMNET: mobility-traces (.xml)
- Shawn: snapshot-files (.xml)
- ns2/ns3: trace-files, activity-files, and mobility-files (all
  \*.tcl)
- PHEM: .dri-files, .str-files, .fzp-files, .flt-files
- GPSDAT csv traces (id,date,x,y,status,speedKMH)
- [GPX](http://en.wikipedia.org/wiki/GPS_eXchange_Format)
- [KML](https://en.wikipedia.org/wiki/Keyhole_Markup_Language)
- [Kepler-JSON](https://kepler.gl/)
- [POIs](../Simulation/Shapes.md#poi_point_of_interest_definitions)
  (can visualize GPS distortions and sampling in
  [sumo-gui](../sumo-gui.md))
- [fcd output](../Simulation/Output/FCDOutput.md) to restrict the
  file to certain regions or time intervals

# Installation

**traceExporter.py** comes with SUMO and
is located in {{SUMO}}/tools/. Python must be installed.

# Usage Description

You convert a given [fcd output](../Simulation/Output/FCDOutput.md)
into a format using a command like this:

```
traceExporter.py --fcd-input myFCDoutput.xml --omnet-output myOMNETfile.xml
```

In the case given above, a file for [OMNET](http://www.omnetpp.org/)
will be built.

In the following, the command line options are described.

## Input Options

**traceExporter.py** often needs only the
[fcd output](../Simulation/Output/FCDOutput.md) as input. In some
cases, the network used while obtaining the [fcd
output](../Simulation/Output/FCDOutput.md) must be given.
**traceExporter.py** gives an error
message if the wanted output needs additional files, such as the
network, but no such file is specified. The options for defining the
input files are:

| Option                          | Description                                                                          |
|---------------------------------|--------------------------------------------------------------------------------------|
| **--fcd-input** {{DT_FILE}}<br>**-i** {{DT_FILE}} | The fcd-output - file to convert (numeric value is interpreted as port to listen on) |
| **--net-input** {{DT_FILE}}<br>**-n** {{DT_FILE}} | The network file used; Mainly used for exporting the network                         |

All outputs which need to read the input only once support reading from
a socket. To do so just give a number as file name and the script will
open a port to listen on. Afterwards you can start sumo with the
corresponding port as output, e.g. traceExporter.py --fcd-input 43214
... needs to be followed by sumo --fcd-output localhost:43214. This
should work at least with OMNET, Shawn, GPSDAT, GPX, POIs and fcd
filtering.

## Output Options

In the following, the output options of
**traceExporter.py** are given, divided by
the application,

### PHEM

| Option              | Description                                                                                                                               |
|---------------------|-------------------------------------------------------------------------------------------------------------------------------------------|
| **--dri-output** {{DT_FILE}} | Generates a PHEM .dri-file named {{DT_FILE}}                                                                                                   |
| **--str-output** {{DT_FILE}} | Generates a PHEM .str-file named {{DT_FILE}}; note that the SUMO network used during the simulation must be given using the **--net-input** option |
| **--fzp-output** {{DT_FILE}} | Generates a PHEM .fzp-file named {{DT_FILE}}                                                                                                   |
| **--flt-output** {{DT_FILE}} | Generates a PHEM .flt-file named {{DT_FILE}}                                                                                                   |

### OMNET

| Option                | Description                                   |
|-----------------------|-----------------------------------------------|
| **--omnet-output** {{DT_FILE}} | Generates a OMNET mobility trace named {{DT_FILE}} |

### Shawn

| Option                | Description                                 |
|-----------------------|---------------------------------------------|
| **--shawn-output** {{DT_FILE}} | Generates a Shawn snaphot file named {{DT_FILE}} |

### ns2/ns3

| Option                      | Description                                     |
|-----------------------------|-------------------------------------------------|
| **--ns2config-output** {{DT_FILE}}   | Generates a ns2 configuration file named {{DT_FILE}} |
| **--ns2activity-output** {{DT_FILE}} | Generates a ns2 activity file named {{DT_FILE}}      |
| **--ns2mobility-output** {{DT_FILE}} | Generates a ns2 mobility file named {{DT_FILE}}      |

### GPSDAT

| Option                 | Description                          |
|------------------------|--------------------------------------|
| **--gpsdat-output** {{DT_FILE}} | Generates a GPSDAT file named {{DT_FILE}} |

### GPX

| Option              | Description                       |
|---------------------|-----------------------------------|
| **--gpx-output** {{DT_FILE}} | Generates a GPX file named {{DT_FILE}} |

### PoI

| Option              | Description                            |
|---------------------|----------------------------------------|
| **--poi-output** {{DT_FILE}} | Generates a SUMO PoI file named {{DT_FILE}} |

### KML

| Option              | Description                       |
|---------------------|-----------------------------------|
| **--kml-output** {{DT_FILE}} | Generates a KML file named {{DT_FILE}} |

### fcd

| Option                        | Description                                                 |
|-------------------------------|-------------------------------------------------------------|
| **--fcd-filter** {{DT_FILE}}           | Reads filter settings from {{DT_FILE}}                           |
| **--fcd-filter-comment** {{DT_STR}} | Extra comments to include in fcd file                       |
| **--fcd-filter-type** {{DT_STR}}    | Modify the vehicle type of all written vehicles to {{DT_STR}} |

The filter file is a simple csv format giving
begin;end;bounding_box;outfile where the bounding box is x1,y1,x2,y2
defining the lower left and the upper right corner of the area to
extract. The coordinate format needs to fit the input format (so if the
input has geo coordinates the bounding box needs to have geo coordinates
as well). It is currently not possible to give the same output file for
different filter lines.

Example filter file:

```
0;50;500,400,600,500;out.xml
50;100;550,450,600,500;out2.xml
```

## Processing Options

Several options allow to fine-tune the processing.

| Option                                     | Description                                                                                             |
|--------------------------------------------|---------------------------------------------------------------------------------------------------------|
| **--persons** {{DT_BOOL}}                           | Whether persons should be included in the trace; *default: False*                                        |
| **--penetration** {{DT_FLOAT}}                      | A float between 0 and 1, defining the probability of a vehicle to be included in the output; *default: 1* |
| **--gps-blur** {{DT_FLOAT}}                         | A float defining a random offset applied on the vehicle positions before exporting them; *default: 0*   |
| **--boundary** {{DT_FLOAT}},{{DT_FLOAT}},{{DT_FLOAT}},{{DT_FLOAT}} | Defines the bounding box as 'xmin,ymin,xmax,ymax'                                                       |
| **--delta-t** {{DT_TIME}}<br>**-d** {{DT_TIME}}              | Defines the time sampling, if given; only every {{DT_TIME}} time step will be exported.                      |
| **--begin** {{DT_TIME}}<br>**-b** {{DT_TIME}}                | Time steps before {{DT_TIME}} are not exported.                                                              |
| **--end** {{DT_TIME}}<br>**-e** {{DT_TIME}}                  | Time steps after {{DT_TIME}} are not exported. Please note that some issues are known, here.                 |
| **--seed** {{DT_INT}}<br>**-s** {{DT_INT}}                   | The randomizer seed to use.                                                                             |
| **--base-date** {{DT_INT}}<br>**-s** {{DT_INT}}              | The base date in seconds since the epoch (1970-01-01 00:00:00), the default is the current local time.  |

## Additional Options

| Option       | Description          |
|--------------|----------------------|
| **--help**<br>**-h** | Shows a help screen. |

# See Also

- This [tutorial on using
  TraceExporter](../Tutorials/Trace_File_Generation.md)
- [working around bugs in version
  0.19.0](../FAQ.md#traceexporterpy_fails_to_work_properly_in_verson_0190)

<div style="border:1px solid #909090; min-height: 35px;" align="right">
<span style="float: right; margin-top: -5px;"><a href="http://cordis.europa.eu/fp7/home_en.html"><img src="../images/FP7-small.gif" alt="Seventh Framework Programme"></a>
<a href="http://colombo-fp7.eu/"><img src="../images/COLOMBO-small.png" alt="COLOMBO project"></a></span>
<span style="">This part of SUMO was developed, reworked, or extended within the project 
<a href="http://colombo-fp7.eu/">"COLOMBO"</a>, co-funded by the European Commission within the <a href="http://cordis.europa.eu/fp7/home_en.html">Seventh Framework Programme</a>.</span></div>