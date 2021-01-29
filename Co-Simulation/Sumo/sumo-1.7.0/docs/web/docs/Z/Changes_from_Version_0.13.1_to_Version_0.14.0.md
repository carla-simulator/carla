---
title: Z/Changes from Version 0.13.1 to Version 0.14.0
permalink: /Z/Changes_from_Version_0.13.1_to_Version_0.14.0/
---

## Version 0.14.0 (11.1.2012)

- **config file extensions renamed**, see [Other/File Extensions](../Other/File_Extensions.md)

### Bugfixes

- Simulation
  - removed invalid *collision* warnings
  - removed various gui glitches when drawing vehicles
  - fixed free speed calculation
- sumo-gui
  - tracking a vehicle no longer messes up start/stop controls
- netconvert
  - fixed minor bugs related to updating edge attributes with
    additional *edg.xml* files
  - builds without PROJ will no longer produce a bugged binary
    (failing with "no option with the name proj.inverse exists")
- duarouter
  - use identical units for parsed data and calculated defaults
- Tools
  - netdiff now correctly handles repeating identical traffic light
    phases

### Enhancements

- netconvert
  - changed the way junctions are joined when using **--join.junctions**; see
    [Networks/Building Networks from own XML-descriptions\#Joining Nodes](../Networks/PlainXML.md#joining_nodes)
  - all output is now written using UTF-8 encoding instead of
    Latin-1. This should allow the usage of international street
    names (note that street ids may use only ascii)
  - added option **--proj.plain-geo** which writes plain-xml files using geo-coordinates
  - location information is now embedded in *nod.xml* files. This
    makes conversion between *net.xml* and plain xml lossless.
  - Importing large OSM Networks is much faster due to algorithmic
    improvements
  - added options **--keep-edges.by-type** and **--remove-edges.by-type** for restricting a network
- sumo-gui
  - sumo-gui now parses command line options
- general options
  - boolean options may be disabled from the command line
    (--help=false)
  - a single parameter (not starting with "-") is treated as a
    configuration file
- Tools
  - added eco routing capabilities to duaIterate.py

Reduced memory consumption of all applications. Also increased speed for
some applications. For benchmark values see
[\#634](https://sourceforge.net/apps/trac/sumo/ticket/634)

### Other

- Simulation
  - default arrival position is now lane end instead of start
- netconvert
  - renamed XML-element **reset** to **delete** to better reflect
    its purpose
- sumo-gui
  - Changed close-simulation hotkey from Ctrl-C to Ctrl-W to better
    conform to interface standards
- Tools
  - python module sumolib.output now has a method *parse* which
    supports all output files
- first incarnation of a windows installer
- TraCI
  - emission related outputs are now in mg and ml