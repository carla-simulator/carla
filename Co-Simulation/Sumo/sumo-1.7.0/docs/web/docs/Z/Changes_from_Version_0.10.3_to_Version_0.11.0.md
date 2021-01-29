---
title: Z/Changes from Version 0.10.3 to Version 0.11.0
permalink: /Z/Changes_from_Version_0.10.3_to_Version_0.11.0/
---

- All
  - moved to xerces 3.0
  - moved to FOX1.6
  - changes in network format (use {{SUMO}}/tools/net/0103to0110.py to
    convert from old to new representation)
    - traffic light descriptions (18.05.2009)
    - descriptions of giving lanes free / prohibiting lanes for
      certain vehicle classes (10.06.2009)
    - moved character sections to attributes (22.-24.07.2009), see
      [SUMO Road Networks](../Networks/SUMO_Road_Networks.md)
  - configuration xml format changed from `<section><key>value</key></section>` to `<section key="value"/>` (use {{SUMO}}/tools/10to11.py to convert from old to new
    representation)
  - The data-folder was removed; instead the examples got a part of
    the tests and are generated using these.

- Simulation
  - Collisions
    - Collisions are now checked per default (removed option **--check-accidents**;
      added option **--ignore-accidents** with "reverse meaning" instead)
    - Removed option **--quit-on-accident**
    - Debugged buggy dealing with false vehicle leaving order
      (what should in fact not happen anyway)
  - Cleaning
    - Removed unused/undescribed lane-change output (**--lanechange-output**)
    - Removed unused/undescribed lane-change options **--lanechange.min-sight** and **--lanechange.min-sight-edges**
  - Outputs
    - patched problems with the intervals of detector outputs
      (including [defect 73: false end step information in detectors output](https://github.com/eclipse/sumo/issues/73))
  - removed TrafficOnline classes; new approaches will be
    described/discussed at
    TrafficOnline; was: [enhancement 57: remove TrafficOnline classes from simulation core](https://github.com/eclipse/sumo/issues/57)
    - [edgelane traffic](../Simulation/Output/Lane-_or_Edge-based_Traffic_Measures.md),
      [edgelane hbefa](../Simulation/Output/Lane-_or_Edge-based_Emissions_Measures.md),
      and [edgelane harmonoise]../(Simulation/Output/Lane-_or_Edge-based_Noise_Measures.md)
      are no longer writing empty intervals (13.05.2009)
    - corrected behavior of using intervals in [edgelane traffic](../Simulation/Output/Lane-_or_Edge-based_Traffic_Measures.md),
      [edgelane hbefa](../Simulation/Output/Lane-_or_Edge-based_Emissions_Measures.md),
      and [edgelane harmonoise](../Simulation/Output/Lane-_or_Edge-based_Noise_Measures.md)
      (solved [bug 2504162: multiple begins / ends should be defined in separate dumps](http://sourceforge.net/tracker/?func=detail&aid=2168007&group_id=45607&atid=443424)
      and [bug 2504144: dump intervals are mandatory](http://sourceforge.net/tracker/?func=detail&aid=2168007&group_id=45607&atid=443424))
    - [edgelane hbefa](../Simulation/Output/Lane-_or_Edge-based_Emissions_Measures.md)
      now additionally writes per-vehicle values
  - fixed re-emission check for edges with multiple waiting vehicles

- sumo-gui
  - Added the possibility to load view settings and
    [decals](../sumo-gui.md#using_decals_within_sumo-gui) from
    a configuration file (implemented [request 2168007: add possibility to load view settings file with start of gui](http://sourceforge.net/tracker/?func=detail&aid=2168007&group_id=45607&atid=443424)).
    Added the possibility to generate snapshots on startup, defined
    in a configuration file.
  - Removed the "additional weight" dialog - was not known to be
    used

- NETGEN
  - random networks may now contain bidirectional edges; the
    probability to have an edge being bidirectional is controlled by
    the **--rand-bidi-probability** {{DT_FLOAT}} with default=1 (all edges are bidirectional)
  - The default junction type (option **--default-junction-type**) may now be only one of
    "priority", "right_before_left", or "traffic_light". Building
    agentbased and actuated traffic lights from within NETGEN will
    no longer be supported.

- netconvert
  - Nodes which have same or almost same positions are no longer
    joined per default.
  - Type descriptions allow a certain edge type to be discarded from
    being imported using the `discard="x"` attribute.
  - netconvert can guess roundabouts; force using **--guess-roundabouts**
  - corrected usage of the `spread_type` attribute in edges; was: [defect 111 correct usage of the spread_type attribute](https://github.com/eclipse/sumo/issues/111)
  - patching problems with RoboCup Rescue League import

- od2trips
  - The default departure/arrival behavior can be controlled on the
    command line, now. The following options were introduced for
    this purpose: **--departlane**, **--departpos**, **--departspeed**, **--arrivallane**, **--arrivalpos**, **--arrivalspeed**. See also
    [Specification](../Specification.md)
  - solved [defect 67: false interpretation of the second time field (od2trips)](https://github.com/eclipse/sumo/issues/67)

- duarouter
  - The default departure/arrival behavior can be controlled on the
    command line, now. The following options were introduced for
    this purpose: **--departlane**, **--departpos**, **--departspeed**, **--arrivallane**, **--arrivalpos**, **--arrivalspeed**. See also
    [Specification](../Specification.md)

- jtrrouter
  - The default departure/arrival behavior can be controlled on the
    command line, now. The following options were introduced for
    this purpose: **--departlane**, **--departpos**, **--departspeed**, **--arrivallane**, **--arrivalpos**, **--arrivalspeed**. See also
    [Specification](../Specification.md)

- TraCI
  - changed name of "areal detectors" to "multi-entry/multi-exit
    detectors"
  - fixed crashes occurring when using the distance command
    ([defect 70: TraCI does not recognize edges with'\['...](https://github.com/eclipse/sumo/issues/70))
  - added [APIs for getting information about vehicles, vehicle types, lanes, routes, polygons, PoIs, and junctions](../TraCI.md).
  - added the possibility to change the phase and retrieve the
    current phase and complete definition of a tls using the new
    phase definition