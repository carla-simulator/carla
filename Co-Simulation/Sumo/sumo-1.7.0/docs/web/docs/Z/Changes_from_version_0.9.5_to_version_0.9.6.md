---
title: Z/Changes from version 0.9.5 to version 0.9.6
permalink: /Z/Changes_from_version_0.9.5_to_version_0.9.6/
---

- polyconvert: possibility to prune the polygons to import on a
  bounding box added
- DUA: Simulation may now load only a part of defined vehicles
- netconvert:
  - debugged connections for on-ramps which already have the right
    number of lanes
  - debugged problems when using networks built without internal
    lanes in conjunction with (\!\!\! use-internal in simulation)
  - lanes split definition added to lane definitions (\!\!\!
    undescribed)
  - added error messages when false lanes are defined in connections
- SIMULATION:
  - c2c wth rerouting
- sumo-gui:
  - debugged memory leak in polygon visualization
  - removed simple/full geometry options; everything is now drawn
    using full geometry
  - the names of nodes, additional structures, vehicles, edges, pois
    may now be shown
- All: help/config subsystem changed
- Netz hat sich geändert
- added the possibility to load a network only
- lane expansions
- iodevices reworked partially
- "Action" removed\* was the same as Command
- changed the Event-interface
  - execute now gets the current simulation time
  - event handlers are non-static
- .icc-files removed
- class-documentation added/patched within dfrouter
- made access to positions in Position2DVector c++ compliant
- xml-types are now read prior to other formats (other Xml-description
  afterwards)
- double projection in XML-import debugged
- vehicle actors added
- shape layers added
- extracted drawing of lane geometries
- added projection information to the network
- edge types may now store the edge function
- removed the unneeded slow lane changer
- VISUM: lane connections for unsplitted lanes implemented, further
  refactoring
- retrieval of microsim ids is now also done using getID() instead of
  id()
- complete od2trips rework
- all structures now return their id via getID()
- rerouter: patched the distribution usage; added possibility o load
  predefined routes
- unifying threads
- beautifying: output consolidation
- removal of loops added (undescribed\!\!\!)
- got rid of the static RONet instance
- got rid of the cell-to-meter conversions
- began with the extraction of the car-following-model from MSVehicle
- began the work on a network editor
- added consecutive process messages
- debugged splitting of matrices
- added the possibility to stretch/change the current phase and
  consecutive phases
- made static comboboxes really static
- VISUM: patching problems on loading split lanes and tls
- added blinker visualisation and vehicle tracking (unfinished)
- rertrieval-methods have the "get" prependix; EmitControl has no
  dictionary; MSVehicle is completely scheduled by MSVehicleControl;
  new lanechanging algorithm
- tls switches added
- made the assertion that two consecutive edge geometry points must not
  be same less aggressive
- made MSVehicleControl completely responsible for vehicle handling\*
  MSVehicle has no longer a static dictionary
- dump-begin/dump-end renamed to dump-begins/dump-ends
- E3 detectors refactored partially
- added the possibility to make a polygon being filled or not
- cartesian and geocoordinates are shown; changed the APIs for this
- error checks for dump-begins/dump-ends added
- simulation-wide cartesian to geocoordinates conversion added
- ROUTER: faster access to weight time lines added;
- removed build number information
- API for drawing not filled circles
- first try to make junctions tls-aware
- changed the way geocoordinates are processed
- changed vehicle class naming
- patching junction-internal state simulation
- added vehicle class support to microsim
- added vehicle class support to microsim
- removed deprecated c2c functions, added new made by Danilot Boyom
- c2c visualisation added (unfinished)
- added the possibility to copy an artefact's (gl-object's) name to
  clipboard (windows)
- removed unneeded id member in MSMoveReminder
- added an error handling for adding a tls definition for an unknown
  tls
- added the option to not emit vehicles from flows using a fix
  frequency
- tls tracker now support switches between logics
- first steps towards car2car-based rerouting
- variable vehicle type usage
