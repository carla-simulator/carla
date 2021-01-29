---
title: Tools
permalink: /Tools/
---

Tools can be found in the SUMO-distribution under {{SUMO}}/tools. Most of
them are tiny - they were written for a certain purpose and worked well
under certain input, but may be not verified for other cases. Please
note that we are currently restructuring the tools, see
<https://github.com/eclipse/sumo/issues/1425>. Most of the tools will be
moved directly into the {{SUMO}}/tools dir, while obsolete scripts will
move to {{SUMO}}/tools/purgatory.

The tools are divided into the following topics:

- *[assign](Tools/Assign.md)* - traffic assignment tools
- *[build](Tools/Build.md)* - tools used for code styling and
by the building subsystems
- *[detector](Tools/Detector.md)* - some tools for dealing
with real life induction loop data
- *[turn-data](Tools/Turns.md)* - some tools with turning counts and turning
  ratio data
with real life induction loop data
- *[district](Tools/District.md)* - tools for handling traffic
analysis zones (TAZ) aka districts
- *import* - allow to import data from formats which can not be
processed "natively"
  - *[osm](Tools/Import/OSM.md)* - some helpers for
    accessing/using OpenStreetMap data
  - *[visum](Tools/Import/VISUM.md)* - some helpers for
    using VISUM data
  - *[vissim](Tools/Import/VISSIM.md)* - some helpers for
    using VISSIM data
  - *[MATSim](Tools/Import/MATSim.md)* - some helpers for
    using MATSim data
- *[net](Tools/Net.md)* - tools for working with networks
(mainly SUMO-networks)
- *projects* - additional tools used by our projects
- *[route](Tools/Routes.md)* - tools for working with routes
- *[shapes](Tools/Shapes.md)* - tools for working with shapes
(PoIs and polygons)
- *[tls](Tools/tls.md)* - tools for traffic light systems
- *[traci](TraCI/Interfacing_TraCI_from_Python.md)* - Python
implementation of the TraCI interface
- *[traceExporter](Tools/TraceExporter.md)* - generate
mobility traces / trajectories / orbits / gps tracks in various
formats
- *[trip](Tools/Trip.md)* - Trip generation and modification
without [od2trips](od2trips.md)
- *[visualization](Tools/Visualization.md)* - graphical
evaluation of SUMO-outputs
- *[sumolib](Tools/Sumolib.md)* - python library for working
with sumo networks, simulation output and miscellaneous utility
functions
- *[output](Tools/Output.md)* - python tools for working with
sumo output files
- *[emissions](Tools/Emissions.md)* - tools for computing and
visualizing emissions
- *[xml](Tools/Xml.md)* - python tools for converting xml and
xml schema related utilities
- *[miscellaneous](Tools/Misc.md)* - tools that do not fit the
above categories

!!! caution
    Most tools were written for Python 2.7 and still require this version. With the exception of sumolib and traci, Python 3.0 compatibility is not assured. We are still working on it.