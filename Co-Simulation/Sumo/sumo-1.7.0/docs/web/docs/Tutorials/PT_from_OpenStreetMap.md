---
title: Tutorials/PT from OpenStreetMap
permalink: /Tutorials/PT_from_OpenStreetMap/
---

# Introduction

This tutorial shows you how to build a public transit (PT) simulation
scenario from scratch using an [OpenStreetMap
file](../OpenStreetMap_file.md) as the only input. If you are new
to OpenStreetMap in conjunction with SUMO you will also find the
[OSM](../Tutorials/Import_from_OpenStreetMap.md) tutorial helpful.

The creation of PT simulation consists of two steps:

1.  Initial network and public transit information extraction
2.  Finding feasible stop-to-stop travel times and creating PT schedules

!!! note
    If you use the [osmWebWizard tool](../Tutorials/OSMWebWizard.md), these steps are performed automatically, when you select the checkbox *Import Public Transport*.

# Initial network and public transit information extraction

In the first step PT stops and line information will be extracted. The
step is performed by invoking netconvert as follows

```
netconvert --osm-files osm.xml -o net.net.xml --osm.stop-output.length 20 
    --ptstop-output additional.xml --ptline-output ptlines.xml
```

, where

- *--osm-files osm.xml* points to the [OpenStreetMap
  file](../OpenStreetMap_file.md)
- *-o net.net.xml* points to the to-be-generated SUMO network file
- *--osm.stop-output.length 20* defines the length of the PT stop
  platforms in meter
- *--ptstop-output additional.xml* points to the to-be-generated [PT
  stop file](../Simulation/Public_Transport.md)
- *--ptline-output ptlines.xml* points to an intermediate file
  defining the PT lines.

# Finding feasible stop-to-stop travel times and creating PT schedules

The second step is to run a python script that performs the following:

1.  To determine feasible stop-to-stop travel times, a simulation run is
    performed with one PT vehicle for each PT line.
2.  The PT schedule (regular interval timetable) for all lines is
    created from the stop-to-stop travel time information.

The stop-to-stop travel times are determined on an empty network. It
will be straightforward to extend the script so the stop-to-stop travel
times a determined for an existing transport model. This, however, is
beyond the scope of this tutorial. The steps are performed by invoking
the *ptlines2flows.py* script as follows

```
tools/ptlines2flows.py -n net.net.xml -s additional.xml -l ptlines.xml 
    -o flows.rou.xml -p 600 --use-osm-routes
```

, where

- *-n net.net.xml* points to the SUMO network previously created with
  netconvert
- *-s additional.xml* points to the PT stop file created with
  netconvert
- *-l ptlines.xml* points to the PT lines definition file created with
  netconvert
- "-o flows.rou.xml'' points to the to-be-generated
  [route](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md)
  files for the PT vehicles
- "-p 600" is the period for the regular interval time table in
  seconds
- "--use-osm-routes" means the script will adopt the pt routes from
  osm.xml rather then creating new shortest path routes between stops