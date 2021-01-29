---
title: Z/Changes from version 0.9.3 to version 0.9.4
permalink: /Z/Changes_from_version_0.9.3_to_version_0.9.4/
---

This release is rather a snapshot. Many things have changed and some
will require some further revalidation. There are three major
changes/extensions:

**1.** The TLS-subsystem now allows having more than a single logic
for a traffic light. One can describe the switch times and switch
between them on the GUI. The user-description on this may be found under [Traffic Lights](http://sumo.sourceforge.net/docs/gen/user_chp06.shtml#user_chp06-trigger-traffic_lights). There is no developer documentation on this, yet.
If you are parsing the networks by your own, you will see that traffic
lights definitions have changed slightly. Instead of the sgmltag
"logicno" a tag named "subkey" is used by now. Also, "inclanes" has been
removed.

**2.** A full support to import networks with positions encoded in
geocoordinates. This makes the usage of two further libraries necessary,
PROJ.4 and GDAL. The new building process is described [here](http://sumo.sourceforge.net/docs/gen/sumo_howto_building.shtml).
A description about how to import geocoordinated files (despite of
ArcView-files) can be found [here](http://sumo.sourceforge.net/docs/gen/user_chp04.shtml#user_chp04-further_options-geocoordinates).

Together with this extension, the import of ArcView-files has been
rechecked and now allows to import networks stored in other schemes than
the one NavTeq uses. The new options are described [here](http://sumo.sourceforge.net/docs/gen/user_chp04.shtml#user_chp04-other_input-arcview).

Some further comments on importing ArcView-files are available [here](http://sumo.sourceforge.net/docs/gen/sumo_moreon_arcview.shtml). By now, they include only some comments on importing an open-source network of Osnabrück. You can take a look at this [here](http://sumo.sourceforge.net/screens/frida.shtml) and also download it from (dead).


**3.** The developer documentation has been split into several parts.
It seems to be unmanageable to write a complete developer documentation.
Due to this, we now offer some additional documentation on certain
topics. Topics that were previously a part of the developer
documentation may be now found within the [More Docs](http://sumo.sourceforge.net/docs/moredocs.shtml) section.

<br>

There are also some further changes which do not yet show their
potentials, but are quite promising:

**1.** The routing interface has been made abstract. This allows to
use routing algorithms in both the simulation as in the routing
applications and will hopefully yield implementations of some further
routing algorithms - by now Dijkstra and a turning ration algorithm are
available.

**2.** Some first steps towards making SUMO be aware of vehicle types
have been done. It is now possible to allow/disallow vehicles by class
to use certain lanes. This is now implemented in the netconvert
application and within duarouter, but not yet in the simulation itself
and due to this not yet documented. But, as you can see, we are working
towards making SUMO multi-modal...

The complete list of changes:

- Multimodality
  - made lanes storing vehicle classes (additional XML-definition
      for lanes, Router is aware of vehicle classes) (undocumented)
- Routing Interface
  - Router have been made abstract
- GUI
  - the Grid now handles networks with negative positions
  - debugging the visualization grid (unfinished)
  - glu is now used to draw polygons in order to allo concave
    polygons
  - pois may now be added and moved (using shift)
  - Emitter may show their routes
  - added option to change the exaggeratioln of POIs
- Simulation
  - detector position may now be "friendly" (see
    [E1-Detectors (Induct Loops)](http://sumo.sourceforge.net/docs/gen/user_chp06.shtml#user_chp06-output-detectors-e1))
  - documentation on variable speed signs added (see [Variable Speed Signs (VSS)](http://sumo.sourceforge.net/docs/gen/user_chp06.shtml#user_chp06-management-vss))
- netconvert
  - Improved ArcView-import
  - Additional option for importing NavTeq-networks (using
    ArcView/Elmar/Elmar2): new navteq lane number information may be
    now only used if not differing too much from the previous \!\!\!
  - ramp-guessing improved: no lanes are appended if there are
    enough
  - Vision import added (see [Importing VISUM-networks](http://sumo.sourceforge.net/docs/gen/user_chp04.shtml#user_chp04-other_input-visum))
  - Positions of guessed TLS may now be saved as POIs (undocumented, yet)
  - The edge function can be given (undocumented, yet)
- jtrrouter
  - default-percentages may now contain more than three items
  - Examples added
  - renamed "percentage" to probability
  - Documentation updated (see [Using the Junction Turning Ratio - Router](http://sumo.sourceforge.net/docs/gen/user_chp05.shtml#user_chp05-own_routes-jtr))
- All
- documentation on meta information added (see [Available Meta-Information](http://sumo.sourceforge.net/docs/gen/user_chp03.shtml#user_chp03-sumo-meta))