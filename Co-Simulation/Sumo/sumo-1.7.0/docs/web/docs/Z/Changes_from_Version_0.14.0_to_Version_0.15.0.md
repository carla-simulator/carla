---
title: Z/Changes from Version 0.14.0 to Version 0.15.0
permalink: /Z/Changes_from_Version_0.14.0_to_Version_0.15.0/
---

## Version 0.15.0 (14.03.2012)

### Bugfixes

- Simulation
  - Real time factor in simulation summary is now correct (was off
    by factor 1000)
  - Option **--route-steps** now reads the given number of seconds ahead (input
    argument was interpreted as milliseconds, inadvertently)
  - specifying invalid departPos no longer causes an infinite loop
  - The minimum gap is now outside of the vehicle. This means, a
    vehicle starts at its position, the minimum breaking distance is
    in front of the position. This effects the retrieval of a
    vehicle's position via TraCI, and also the measures collected by
    detectors - now, the collected vehicle lengths do not contain
    the minimum distance anymore. see [blog entry](http://sourceforge.net/apps/wordpress/sumo/2012/02/14/request-for-comments-pulling-mingap-out-of-the-vehicle/)
  - consolidating right-of-way rules (again). **Networks must be
    rebuild**
  - fixed crash when using TraCI to set new TLS-programs with less
    phases (ticket \#652)
- netconvert
  - looped ways are now correctly imported from OSM (before, these
    were pruned from the network)
  - fixed bug related to **--proj.plain-geo** (would sometimes crash or produce invalid
    output)
  - fixed bug in geometry computation when dealing with
    3D-coordinates
  - fixed bug when joining junctions (did not join as much as
    requested)
  - OSM import no longer discards edges with multiple types as long
    as at least one type is known (this caused missing bridges etc.)
  - debugged import of VISUM-turn descriptions ("ABBIEGER")
  - fixed calculation of intersecting lines
- GUI
  - corrected the link numbering
  - no longer crashes when reload is pressed during running
    simulation
- duarouter
  - Option **--max-alternatives** is no longer ignored
  - clogit probabilities are calculated correctly

### Enhancements

- Simulation
  - Meandata output can now print default travel times / emissions
    on empty edges (excludeEmpty="defaults")
- duarouter
  - added Option **--routing-algorithm**. It supports the values *dijkstra* (default) and
    *astar* (new). The newly added *astar* algorithm uses the
    *euclidean distance heuristic*. It routes 30% faster on the road
    network of Cologne and 40% faster on the road network of Berlin.
  - In verbose mode, some performance measures of the routing
    algorithm are given
  - better defaults for emission based routing
- sumo-gui
  - object choser can now filter by selection
- netconvert
  - added Options **--speed.offset** and **--speed.factor**. These modify all edge speeds by first
    multiplying with factor and then adding offset.
  - added output Option **--junctions.join-output FILE**. This writes a protocol of joined junctions
    to FILE. Loading FILE as additional nod.xml reproduces these
    joins.
- All
  - Logging options are handled consistently
  - step logging enabled by default, can be disabled for all
    relevant applications (sumo, duarouter, jtrrouter, od2trips)

### Other

- Simulation
  - traffic light offset is now interpreted as delay. An offset of x
    delays all phases by x seconds. The old behaviour was to let all
    phases start x seconds earlier.