---
title: Z/Changes from version 0.9.7 to version 0.9.8
permalink: /Z/Changes_from_version_0.9.7_to_version_0.9.8/
---

- Building
  - MSVC8: extracted properties for build configurations into
    separate property files.
  - many coarse changes in directory/project due to resolving
    request 1641440
  - gui/dialogs, gui/drawerimpl, /utils/gui/events,
    /utils/gui/windows, /utils/gui/globjects, /utils/gui/tracker
    have now own projects
  - microsim/traffic_lights - project was named properly
  - netbuild/nodes, microsim/lanechanging, and all netimport classes
    but vissim import were moved one folder up
  - SUMOFrame was moved to microsim (renamed to MSFrame\!)
  - utils/helpers and utils/router were moved to utils/common

- all
  - added inline-documentation for utils/importio

- simulation
  - debugged tls-coupled e2-output
  - debugged [bug 1886198 Segmentation
    Fault](http://sourceforge.net/tracker/index.php?func=detail&aid=1871037&group_id=45607&atid=443421);
    see comment; thanks to Helcio Bezerra de Mello for the bug
    report
  - further work on detectors
    - values revalidated and described <font color="orange">output
      has changed</font>
    - marked "measures" and "style" as deprecated
  - while looking for next links to use, the vehicle is now aware of
    his best lanes
  - simplified computation of best lanes
  - debugged [bug 1871859 vehicles do not wait on lane
    end](http://sourceforge.net/tracker/index.php?func=detail&aid=1871859&group_id=45607&atid=443421);
    thanks to Jensen Chou for the bug report
  - debugged [bug 1871964 left-moving vehicles do not fill
    junction](http://sourceforge.net/tracker/index.php?func=detail&aid=1871964&group_id=45607&atid=443421)
  - patched error handling in emitters; tests added
  - debugged [bug 1873155 vehicles with depart lower than begin are
    emitted](http://sourceforge.net/tracker/index.php?func=detail&aid=1873155&group_id=45607&atid=443421);
    tests added
  - tests for rerouter added; partially debugged
  - added inline-documentation for microsim/output
  - debugging inner-junction simulation
  - refactoring detectors
    - MSDetector2File into MSDetectorControl
    - complete e1-refactoring
    - complete e2-refactoring
    - complete e3-refactoring
    - MSMeanData_Net_Cont is now simply a
      std::vector\<MSMeanData_Net\*\>
    - improved documentation
    - spell checking
- removed MSUnit - the usage was bogus, often same functions were
  defined twice. Rather confusing. Added
  utils/common/PhysicalTypeDefs.h instead, as a new begin.
- Removed MSNet's deltaT - was falsely used and duplicate with
  utils/common/SUMOTime

- guisim
  - debugged [bug 1830296 guiemitter destroys event
    que](http://sourceforge.net/tracker/index.php?func=detail&aid=1830296&group_id=45607&atid=443421)
    by introducing a thread-aware event control version for guisim
    (GUIEventControl)
  - implementing [feature request 1645216 Indicator / blinker
    depending on junction
    distance](http://sourceforge.net/tracker/index.php?func=detail&aid=1645216&group_id=45607&atid=443424)
  - debugged [bug 1862073 view is not refreshed when (de)selecting
    item](http://sourceforge.net/tracker/index.php?func=detail&aid=1862073&group_id=45607&atid=443421)
  - added doxygen-documentation for utils/gui/div
- work on problems with guisim (undeterministic termination)

- netconvert
  - debugged [bug 1886198 User reports unknown error while importing
    shapefile](http://sourceforge.net/tracker/index.php?func=detail&aid=1886198&group_id=45607&atid=443421);
    see comment; thanks to Maxim Raya for the bug report

- dfrouter
  - trying to make it more usable for inner-city scenarios with many
    connections
- spell checking
  - renamed 'succeding' to 'succeeding'
  - guisim now shows internal lanes less wide
  - false left closure of leftmost lane shown in "real world" scheme
    debugged
- removed utils/importio/CSVHelpers (was used once and contained a
  two-lines-method (sic\!))
- working on doxygen documentation (and adding throw-declarations)
  - utils/options
  - utils/iodevices
  - utils/importio
- removing unneeded debug-helpers in microsim
- debugged problems with repetition of vehicles that have a color
  within guisim (application terminated)
- patched XMLSubSys documentation (was
  [bug 1828803](http://sourceforge.net/tracker/index.php?func=detail&aid=1828803&group_id=45607&atid=443421))
- fixed vehicle decelerations when being on a higher priorised road
  (bug
  [1830266](http://sourceforge.net/tracker/index.php?func=detail&aid=1830266&group_id=45607&atid=443421))
- fixed netconvert bugs occuring when connected nodes are too close
  together (bugs
  [1639460](http://sourceforge.net/tracker/index.php?func=detail&aid=1639460&group_id=45607&atid=443421),
  [1639436](http://sourceforge.net/tracker/index.php?func=detail&aid=1639436&group_id=45607&atid=443421))
- moved color out of MSVehicle (affects loading classes)
- router import classes now process input colors as strings
- consolidated XML-definitions
  - vehicle type is now called "type" in all cases
  - removed unneeded elements and attributes
- (almost complete) rework of the microsimulation loop<br>
What was done, basically is to change the way vehicles move over
junctions and how other vehicles get informed about vehicles
approaching on incoming lanes. Quite heavy changes, in fact, I hope
of benefit. You may read some further information on this in the
following tracker items: [bug 1840935: exchange of vehicles over
lanes disallow fails with two
vehs](http://sourceforge.net/tracker/index.php?func=detail&aid=1840935&group_id=45607&atid=443421),
[bug 1840938: lane exchange mechanism ignores undefined lane
order](http://sourceforge.net/tracker/index.php?func=detail&aid=1840938&group_id=45607&atid=443421),
[bug 1840952: drop vehicle exchange in lane
changer](http://sourceforge.net/tracker/index.php?func=detail&aid=1840952&group_id=45607&atid=443421),
[bug 1840944: replace approaching information in
lanes](http://sourceforge.net/tracker/index.php?func=detail&aid=1840944&group_id=45607&atid=443421)

- Further work on inner-junction traffic (still not 100% complete)
  - bug [1840950: take into account inner-junction
    state](http://sourceforge.net/tracker/index.php?func=detail&aid=1840950&group_id=45607&atid=443421)
- speeding up simulation by not touching empty lanes
- C2C was made optional (using defines) in order to reduce memory
  footprint; was: [feature 1843267: make extensions
  optional](http://sourceforge.net/tracker/index.php?func=detail&aid=1843267&group_id=45607&atid=443421)