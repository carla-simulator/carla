---
title: Z/Changes from Version 0.13.0 to Version 0.13.1
permalink: /Z/Changes_from_Version_0.13.0_to_Version_0.13.1/
---

**Release Date: 01.11.2011**

### Bugfixes

- switched-off traffic lights are now put into the correct state
- vehicles with gui shape can now be used as router input
- updated tutorial networks
- fixed bug in duarouter which prevented use of multiple inline routes
- object-chooser no longer centers on unwanted objects

### Enhancements

- TraCI
  - Added the possibility to retrieve an induction loop's position
    (offset/lane) as suggested by Xiao-Feng Xie; thanks\!
  - Added the possibility to [remove a vehicle via TraCI](../TraCI/Change_Vehicle_State.md)
  - Added the description about how to [add a vehicle via TraCI](../TraCI/Change_Vehicle_State.md)
  - Retrieve the number of still expected vehicles
- netconvert
  - added more control over joining junctions. You can declare nodes
    to be joined as well as exclude nodes from joining (see
    [Networks/PlainXML\#Joining_Nodes](../Networks/PlainXML.md#joining_nodes))
  - added option **--osm.discard-tls** (do not use tls definitions when importing OSM)
  - added option **--junctions.join-exclude** ** *<STRING\>\[,<STRING\>\]\** ** (junctions in this list are not joined)
  - added option **--no-left-connections** (do not build connections to left)
  - added optional attributes 'toLane' and 'fromLane' to
    connection-file element 'reset' (see
    [Networks/PlainXML\#Connection_Descriptions](../Networks/PlainXML.md#connection_descriptions))
  - added importer for openDRIVE files, see
    [Networks/Import/OpenDRIVE](../Networks/Import/OpenDRIVE.md);
    many thanks go to Marius Dupuis from
    [VIRES](http://www.vires.com/) for allowing to use the example
    files for tests
  - added importer for matSIM files, see
    [Networks/Import/MATsim](../Networks/Import/MATsim.md)
    (actually already in 0.13.0)
  - added new plain xml input/output format **plain.tll.xml** which
    holds information about traffic lights.
  - added option **--geometry.max-segment-length** {{DT_FLOAT}} to restrict segment length by inserting additional
    geometry points
  - debugged brakedown on VISUM import
  - ignore connections for explicitly removed edges (instead of
    throwing an error)
- Simulation
  - Added [Simulation/Output/Instantaneous Induction Loops Detectors](../Simulation/Output/Instantaneous_Induction_Loops_Detectors.md)
  - [Simulation/Output/Induction Loops Detectors (E1)](../Simulation/Output/Induction_Loops_Detectors_(E1).md)
    now also allow to generate values on per-vehicle type base
- jtrrouter
  - added Karol Stosiek's patch for reading more than one turn-definitions file; changes **--turn-ratio-file** {{DT_FILE}} to **--turn-ratio-files** ** *<FILE\>\[,<FILE\>\]\** **
- sumo-gui
  - can now switch traffic lights back on
- Tutorials
  - moved tutorials from {{SUMO}}/docs to {{SUMO}}/tests/complex for assuring their
    compliance with the current version; they should though appear
    in the release under {{SUMO}}/docs
  - added a [tutorial on car-following parameter calibration](../Tutorials/Calibration/San_Pablo_Dam.md)
- Tools
  - added script tools/net/netdiff.py `<net1.net.xml> <net2.net.xml> <diff_prefix>` which creates the plain-xml
    differences of two sumo networks. The set of difference files
    can be loaded together with `<net1.net.xml>` to recreate `<net2.net.xml>`. This allows for advanced scenario
    management.
  - added tools for dealing with turn-ratios
- Documentation
  - restructured wiki-pages
  - new static HTML docs generated from the wiki

### Other

- updated gl2ps to version 1.3.6
- removed MSMsgInductLoop; the functionality was almost the same as
for a plain MSInductLoop just that an additional string wasgiven;
the functionality can be easily obtained by using proper ids and
mapping them to "messages"