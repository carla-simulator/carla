---
title: Z/Changes from Version 0.27.0 to Version 0.27.1
permalink: /Z/Changes_from_Version_0.27.0_to_Version_0.27.1/
---

## Version 0.27.1 (27.07.2016)

### Bugfixes

- netconvert
  - Original IDs are no longer lost when splitting edges. #2497
  - Elevation data is now correctly imported from OpenDRIVE
    networks. #2239

- netedit
  - Fixed crash when increasing the number of lanes by setting
    numLanes. #2505
  - Fixed slow loading of large networks.
  - Custom colors for selected junctions and edges are now working. #2386

- duarouter
  - Fixed crash when using option **--ignore-error** with invalid `via`-edges. #2494

### Enhancements

- Simulation
  - Added new option **save-state.period** {{DT_TIME}} to save simulation state periodically. #2441
  - Added new option **save-state.suffix** {{DT_STR}} to determine the suffix of saved state files.
    The default is *.sbx* which saves in a binary format.
    Alternatively, *.xml* may be used which makes the state files
    human-readable.

- netconvert
  - Networks exported to OpenDRIVE now use parametric curves to
    represent smooth geometry in place of straight-line segments. #2041
  - Networks exported to OpenDRIVE now contain elevation data. #2240
  - Parametric curves as specified in OpenDRIVE version 1.4 can now
    be imported. #2478
  - Revised default OpenDRIVE typemap. Now imports additional lane
    types such as tram and rail. #2492
  - Added new option **osm.layer-elevation** {{DT_FLOAT}} to import implicit elevation data from
    [OSM-layering
    information](../Networks/Import/OpenStreetMap.md#layer_information).
  - Networks imported from OpenDRIVE now contain edge type
    information based on the OpenDRIVE lane types. #2509

- sumo-gui
  - Added Visualization options for drawing intersections with
    exaggerated size and disable edge drawing based on custom
    widths. #2140
  - Added Visualization option for indicating the driving direction
    of lanes. #2180
  - Added Visualization option for drawing [sublane
    boundaries](../Simulation/SublaneModel.md)
  - Lanes can now be colored according to the number of vehicles
    that are delayed from entering the network. #2489

- netedit
  - Added Visualization options for drawing intersections with
    exaggerated size and disable edge drawing based on custom
    widths. #2140
  - Added Visualization option for indicating the driving direction
    of lanes. #2180