---
title: Z/Changes from version 0.9.8 to version 0.9.9
permalink: /Z/Changes_from_version_0.9.8_to_version_0.9.9/
---

- All
  - debugged [bug 1976405 Saving invalid dump
    configuration](http://sourceforge.net/tracker/index.php?func=detail&aid=1976405&group_id=45607&atid=443421);
    Was a false encoding in Option_IntVector
  - builds under 64bit Linux (solved bugs
    [bug 1927822](http://sourceforge.net/tracker/index.php?func=detail&aid=1927822&group_id=45607&atid=443421),
    [bug 1928370](http://sourceforge.net/tracker/index.php?func=detail&aid=1928370&group_id=45607&atid=443421),
    [bug 1930444](http://sourceforge.net/tracker/index.php?func=detail&aid=1930444&group_id=45607&atid=443421),
    [bug 1931468](http://sourceforge.net/tracker/index.php?func=detail&aid=1931468&group_id=45607&atid=443421),
    [bug 1932961](http://sourceforge.net/tracker/index.php?func=detail&aid=1932961&group_id=45607&atid=443421),
    [bug 1933569](http://sourceforge.net/tracker/index.php?func=detail&aid=1933569&group_id=45607&atid=443421),
    [bug 1934635](http://sourceforge.net/tracker/index.php?func=detail&aid=1934635&group_id=45607&atid=443421);
    thanks to Ronald Nippold for being persistent in supplying the
    build logs
  - encapsulated xerces-attributes; makes value retrieval so much
    more comfortable; retrieve a attribute using
    attrs.getTYPENAME(id) instead of getTYPENAME(attrs, id)

- Simulation
  - Hidden state loading/saving completely from the microsim
  - Vehicle may now end their route at a certain position of the
    route's last edge (see
    [Specification](../Specification.md))
  - The route may be now given as an attribute (*route*) within the
    vehicle definition (see
    [Specification](../Specification.md))
  - The vehicle now stores most of its definition parameters in
    *myParameter*, not as atomar values
  - Solved problems with multiple stop on same edge and with stops
    on lanes which can not be used for continuing the route; thanks
    to Friedemann Wesner and Gerrit Lammert for reporting them
  - Reworked mean data output. The vehicle movements are now
    weighted by the time the vehicle was on an edge/lane. Additional
    constraints for output
    (**--exclude-empty-edges**/**--exclude-empty-lanes**) allow to
    dump only information about edges/lanes which were occupied by
    vehicles.
  - debugged [bug 1964433 different simulation results for different starting times](http://sourceforge.net/tracker/index.php?func=detail&aid=1964433&group_id=45607&atid=443421);
    The issue was a false computation of WAUT switching and TLS
    offset times when choosing a different simulation begin time
    than 0
  - replaced MSVehicle::getInTransit and MSVehicle::running by
    MSVehicle::isOnRoad
  - removed MSEdge::isSource; was used only once and can be easily
    replaced by comparing the result of MSEdge::getPurpose()
  - debugged [bug 1932105 second stop on the same edge confuses vehicles](http://sourceforge.net/tracker/index.php?func=detail&aid=1932105&group_id=45607&atid=443421)
  - debugged [bug 1941846 repeatable crash with sumo r5378](http://sourceforge.net/tracker/index.php?func=detail&aid=1941846&group_id=45607&atid=443421)
  - debugged [bug 1912547 linux 64 bit compile error for sumo svn 5143](http://sourceforge.net/tracker/index.php?func=detail&aid=1912547&group_id=45607&atid=443421)
  - debugged [bug 1913655 command line sumo throws unknown exception](http://sourceforge.net/tracker/index.php?func=detail&aid=1913655&group_id=45607&atid=443421)
  - debugged [bug 1924989 sumo r5186 crashs by pure virtual function call](http://sourceforge.net/tracker/index.php?func=detail&aid=1924989&group_id=45607&atid=443421)
  - begun work on [Specification](../Specification.md)
    - all vehicles are emitted at position=0, with speed=0
  - removed usage of NLLoadFilter
  - added an option to quit the simulation if a number of vehicles
    is exceeded (**--too-many-vehicles <INT\>**)
  - fully integrated Tino Morenz's MSVTypeProbe; moved it from
    microsim/trigger to microsim/output; tests added; a vtypeprobe
    is now defined within additional files this way:
    `<vtypeprobe id="test1" type="" freq="10" file="vtypeprobe.xml"/>`
    ("test1" is the vehicle type to report, leave empty in order to
    get information about all vehicles)
  - replaced usage of "Command" by encapsulating methods in
    "WrappingCommand" wherever possible
  - "debugged" usage of actuated/agentbased tls; should be covered
    by tests, in fact
  - further work on devices-consolidation
  - implemented a one-shot / multi-shot routing device

- sumo-gui
  - Removed opening an additional command line window from the
    release version
  - Title is now set properly
  - debugged [bug 2021454 Discarding View settings resets them](http://sourceforge.net/tracker/index.php?func=detail&aid=2021454&group_id=45607&atid=443421);
    The editor now uses the settings that were used when it was
    opened as the one to apply when "cancel" is pressed
  - debugged [bug 2021464 view settings are not saved](http://sourceforge.net/tracker/index.php?func=detail&aid=2021464&group_id=45607&atid=443421);
    The button is now removed, all the interaction will be done via
    the top buttons
  - debugged [bug 2015792 Save List of selected Items](http://sourceforge.net/tracker/index.php?func=detail&aid=2015792&group_id=45607&atid=443421);
    The proper extension (mostly ".txt") is now appended if no
    extension is given. Affects saving breakpoints, additional
    weights, tracked parameter, and selections.
  - [bug 1919509 guisim crashed](http://sourceforge.net/tracker/index.php?func=detail&aid=1919509&group_id=45607&atid=443421)
      is invalid, the issue was a buggy opengl driver

- netconvert
  - debugged [bug 1993928 --keep-edges.input-file does not remove empty nodes](http://sourceforge.net/tracker/index.php?func=detail&aid=1993928&group_id=45607&atid=443421);
    as soon as one edge-removal(keeping) option occures, nodes are
    rechecked whether they contain any further edge. If not, the
    node is removed.
  - VISUM-import: debugged failures due to false district node
    position computation
  - Connection building: made storing outgoing edge connections
    sane; replaced three strange storages by one
  - Disabled the support for setting an "edge function" - because
    vehicles are emitted the same (parametrised) way on each edge,
    we do not need a distinction between source and normal edges;
    sink edges were never distinguished.
  - Refactored the way edge types are stored in netconvert
  - removed "name" attribute from NBEdge; was not written anyway
  - work on SUMO-networks reimport
    <font color="red">(unfinished)</font>
  - documentation: debugged [bug 1909685 option "omit-corrupt-edges" not recognized](http://sourceforge.net/tracker/index.php?func=detail&aid=1909685&group_id=45607&atid=443421)
    (use **--dismiss-loading-errors** instead of
    **--omit-corrupt-edges**)
  - removed usage of NLLoadFilter
  - thanks to Christoph Sommer, the OSM importer now handles speed
    and lanes information; default OS values are now used, too

- duarouter / jtrrouter
  - Removed option **--move-on-short**. It got obsolete as vehicles
    may start on edges which are shroter than they are, now.

- duarouter
  - debugged [bug 1981921 duarouter truncates routes](http://sourceforge.net/tracker/index.php?func=detail&aid=1981921&group_id=45607&atid=443421);
    Now, **--remove-loops** only prunes the route if the vehicle
    would have to take a turnaround at the begin or end.
  - reworked handling of broken routes; longer routes are not
    replaced; incomplete routes may be corrected using the new
    **--repair** option

- jtrrouter
  - Removed possibility to specify turning percentages using CSV
    files

- dfrouter
  - Definition of emitters, vss, etc. are now written into the same
    folder as the declaration

- TraCI
  - TraCI is now enabled as default

- od2trips
  - due to being used for macroscopic demand definitions, od2trips
    now emits vehicles using departlane="best" and departspeed="max"
  - removed option to parse list of tables from a vissim file; they
    may be given at the command line (**--vissim** is no longer
    supported)
  - added the possibility to omit writing the vehicle type
    (**--no-vtype**)

- [Tools/Main](../Tools.md)
  - added some tools for output visualization
    (mpl_dump_twoAgainst.py, mpl_tripinfos_twoAgainst.py,
    mpl_dump_timeline.py) (see
    [Tools/Visualization](../Tools/Visualization.md))