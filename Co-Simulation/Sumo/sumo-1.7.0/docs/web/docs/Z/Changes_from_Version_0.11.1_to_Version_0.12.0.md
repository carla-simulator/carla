---
title: Z/Changes from Version 0.11.1 to Version 0.12.0
permalink: /Z/Changes_from_Version_0.11.1_to_Version_0.12.0/
---

**Release date: 27.05.2010**

- Simulation
  - solved [regarding red light late blocks vehicles](http://sourceforge.net/apps/trac/sumo/ticket/173)
  - corrected the documentation on using [an abstract vehicle class](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#abstract_vehicle_class);
    the attribute which defines it is named `vclass` not `class`.
  - introducing flows as replacement for vehicles with repno and
    period
  - introducing stops in routes
  - corrected unmotivated deceleration in front of intersections
    reported by Björn Hendriks (thanks\!)
  - added option **--sloppy-emit** for not trying to emit on full edges (speeds up
    the simulation a lot)

- TraCI
  - moved to representation of time in milliseconds
  - Induction Loop occupancy is now given in % as defined

- sumo-gui
  - Improved layering (correct order of element drawing)
  - solved [bug 2872900: same title/naming for different dialog boxes](http://sourceforge.net/tracker/?func=detail&aid=2872900&group_id=45607&atid=443424)
  - solved [bug 2872824: fox related sumo crash](http://sourceforge.net/tracker/?func=detail&aid=2872824&group_id=45607&atid=443424)
  - Viewport settings can now be saved/loaded
  - Added new visualisation options
    - lanes: by vclass
    - vehicles: HSV by depart position/arrival position/direction
      and distance

- ROUTER
  - removed random routes generation; introduced a [script for generating random trips](../Tools/Trip.md#randomtripspy)
    instead

- netconvert
  - removed the **--tls-poi-output**; instead, [poi_atTLS.py](../Tools/Shapes.md) can be used
  - OpenStreetMap import
    - removal of edges and nodes which have the attribute `action='delete'`
    - recognition and removal of duplicate edges (all values but
      ids are same) added
    - opposite direction edge is built if oneway==-1
  - edge and node attributes can be overridden

- Tools
  - [tools for easier open street map import](../Networks/Import/OpenStreetMap.md) added