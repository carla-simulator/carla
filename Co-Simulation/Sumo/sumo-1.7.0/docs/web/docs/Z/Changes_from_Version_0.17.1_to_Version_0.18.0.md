---
title: Z/Changes from Version 0.17.1 to Version 0.18.0
permalink: /Z/Changes_from_Version_0.17.1_to_Version_0.18.0/
---

## Version 0.18.0 (28.08.2013)

### Bugfixes

- Simulation
  - fixed crash/hangup if a step-length below the minimum of 0.001
    is given
  - fixed bug that caused visual collisions (vehicles on different
    lanes occupying the same space while on a junction)
  - fixed bug where vehicles were inserted with unsafe speeds on
    minor roads causing collisions
  - removed arrival attribute from vehicle routes output for non
    arrived vehicles
  - now saving all routes if a vehicle is rerouted multiple times
  - fixed crash when using calibrators
- TraCI
  - the return values for link direction and link state are now
    working for lane variable: link (0x33)
  - routes are now preloaded before the first simulation step such
    that getMinExpectedNumber gives good results from the very start
  - Fixed documentation bug that stated geodetic positions have the
    format LAT,LON whereas they actually use LON,LAT. TraCI behavior
    remains unchanged.
  - Fixed rerouting for not departed vehicles (change target, change
    route and change route id)
- netconvert
  - street-names will no longer be missing from the output when
    importing a net.xml or edg.xml file which has them
  - using option **--ramps.no-split** {{DT_BOOL}} no longer creates networks with faulty
    connections.
  - fixed bug that caused invalid connections to be generated where
    roads split or merge
  - fixed invalid traffig light logics and occasional crashes when
    importing a .net.xml file along with split elements
  - when importing sumo networks, dangerous connections (multiple
    connections from the same edge to the same lane) are no longer
    silently discarded
  - fixed bug that caused invalid node geometries when using **--junctions.join** together with ramp building options **--ramps.\***.
- duarouter
  - option **--ignore-errors** {{DT_BOOL}} now works correctly when a vehicle is not allowed to
    drive on its depart edge
- sumo-gui
  - vehicles with guiShape="pedestrian" now have the same size in
    all levels of detail
  - traffic lights and lane-to-lane connections are now drawn for
    railways

### Enhancements

- Simulation
  - improved simulation speed
  - added option **--link-output** {{DT_FILE}} for recording the approach of vehicles at
    junctions. This option addresses the need of developers when
    investigating junction control.
  - added new abstract vehicle classes "custom1" and "custom2" for
    user defined purposes.
  - drivers on a minor road now grow impatient while waiting for a
    gap in the traffic. After a while they will drive even if it
    means that another car has to slow down to accommodate them (as
    before, they will only drive if it is safe to do so).
  - added vType attribute **impatience** which affects the
    willingness of drivers to cut into traffic even if it forces
    other drives to brake.
  - added support for stop-signs at minor roads
  - added support for the intersection type *allway_stop*. [This corresponds to an intersection type found in north america](https://en.wikipedia.org/wiki/All-way_stop)
  - added option **--fcd-output.signals** {{DT_BOOL}} for including a vehicles signal state in the
    output (brake lights etc.)
  - implemented "expected" attribute for stops, which expects IDs of
    people that have to enter the vehicle to allow the vehicle to
    leave the stop. Should be used with "triggered='true'" (would
    not work otherwise). Thanks to Shubham Jain for pointing out the
    wrong description of the possibilities in
    [\[1\]](http://sumo.sourceforge.net/cite.bib).
  - added options for saving and loading simulation states.
    Simulation states can be saved at predetermined times and then loaded to initialize a new
    simulation (this also requires loading the network and routes of the
    original simulation). When the filename for saving ends with **.sbx**
    the file will be saved in a binary format instead of xml.

-   - **--save-state.times** {{DT_TIME}} \[,{{DT_TIME}}...\]
  - **--save-state.files** {{DT_FILE}} \[,{{DT_FILE}}\]\*
  - **--save-state.prefix** {{DT_FILE}}
  - **--load-state** {{DT_FILE}}
  - **--load-state.offset** {{DT_TIME}}
   - option **--sloppy-insert** {{DT_BOOL}} is marked deprecated because the behavior is now the
    default, use **--eager-insert** {{DT_BOOL}} to get the old behavior
  - vehicle route output for persons now includes complete plans

- sumo-gui
  - it is now possible to toggle the simulation time display between
    'seconds' and 'hour-minute-seconds' by clicking on the
    'Time'-button next to the time display. The state of this
    setting persists across GUI sessions.
  - it is now possible to toggle between alternate delay values by
    clicking on the 'Delay'-button next to the delay dial. Clicking
    causes the current delay value to be saved as the alternative
    value. The alternative delay value persists across GUI sessions.
  - parking vehicles are now being drawn (at the side of the road)
  - the vehicle parameter table now contains information about the
    status of stopped vehicles
  - vehicles can now be colored by their selection state
  - added option for selecting a vehicles foes at the next junction
    to the vehicle popup menu (useful when coloring by selecting)
  - brake lights will no longer be triggered when the route ends
    unless the vehicle actually has to slow down.
  - now providing separate options for customizing the visualization
    of persons

- netconvert
  - when using the option **--no-internal-links** the edge lengths are now increased to
    compensate for the missing internal links. Thus, route lengths
    will correspond better to physical distances and roundabouts are
    less prone to congestions.
  - added option **--keep-edges.in-geo-boundary** which works similar to option **--keep-edges.in-boundary** but uses geodetic
    coordinates instead of cartesian.
  - added new node type *priority_stop*. This works similar to type
    *priority* but minor links now require a stop
  - added new node type *allway_stop*. [This corresponds to an intersection type found in north America](https://en.wikipedia.org/wiki/All-way_stop)
  - added option **--ignore-errors.edge-type** for ignoring unknown edge types during loading.
    This happens frequently when dealing with plain xml files based
    on OSM data.
  - now parsing the *OSM*-Attribute "tracks" to determine whether a
    railway element is unidirectional or bidirectional.
  - added option **--osm.railway.oneway-default** {{DT_BOOL}} for setting the default behavior when importing
    railways without "tracks" attribute. The appropriate value
    depends on the mapping style of the regional *OSM* comunity.
  - added option **--geometry.max-angle** {{DT_FLOAT}} which warns about sharp turns on an edge. This is
    enabled by default for turns above 99 Degrees.
  - added option **--geometry.min-radius** {{DT_FLOAT}} which warns about sharp turns at the start and end
    of an edge. This is enabled by default for radii below 9 meters
  - added option **--geometry.min-radius.fix** {{DT_BOOL}} which prunes geometry to avoid sharp turns at the
    start and end of an edge. This is disabled by default.
  - added option **--check-lane-foes.all** {{DT_BOOL}} (default=false) for lifting junction restrictions
    for vehicles which exit on different lanes of the same edge
    without crossing paths. This improves junction throughput in
    many cases but is a more aggressive interpretation of german
    traffic laws.
  - added option **--check-lane-foes.roundabout** {{DT_BOOL}} (default=true) for lifting some junction
    restrictions at roundabouts. This conforms to the typical layout
    of roundabouts where restrictions on lane-changing allow such
    movements.

- TraCI
  - added optional flags for cmd stop (0x12) to set the parking and
    triggered status
  - added command stop state (id 0xb5) for retrieving the
    stopped,parking and triggered status
  - added command resume (0x19) for resuming from the current stop
  - reduced verbosity on connection close (no more good byes)

### Other

- Simulation
  - The elements **<vehicle\>**, **<flow\>** and **<vType\>** now
    support child elements of the form **<param key="myKey"
    value="myValue/\>**. These can be used when implementing custom
    functionality (see
    [Developer/How_To/Device](../Developer/How_To/Device.md)).
  - code for messaging functionality removed (was disabled by
    default anyway)
- [Contributed/SmallMaps](../Contributed/SmallMaps.md) tools was
removed because it was outdated
- switched all references in code and documentation to sumo-sim.org
- TraCI version is now 6