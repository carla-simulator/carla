---
title: Demand/Further Ways to import Vissim Demand Definitions
permalink: /Demand/Further_Ways_to_import_Vissim_Demand_Definitions/
---

Besides using [OD-matrices which can be
imported](../Demand/Importing_O/D_Matrices.md) using
[od2trips](../od2trips.md) as VISUM does, Vissim also allows to
define vehicle flows as a combination of in-flows and route decisions.
The following tool allows to import these. The file needs to be in
German. For tips on conversion, see [Networks/Import/Vissim\#Converting
VISSIM networks to
German](../Networks/Import/Vissim.md#converting_vissim_networks_to_german)

# vissim_parseRoutes.py

Parses routes stored in the given Vissim file (first parameter) as
(in-)flows and route decisions. The read information is saved twice; the
read flows are saved as *<OUTPUT_PREFIX\>.flows.xml*, and the read routes
are saved as *<OUTPUT_PREFIX\>.rou.xml*. Edges of the route may be
renamed by setting them within "edgemap" variable (within the script).
To filter out (short) VISSIM connectors, which may not have been
converted to edges in the network import but are included in the VISSIM
route definitions, you may also (ab)use this facility to delete the edge
from all relevant routes, e.g. edgemap\["10000"\] = ""

```
vissim_parseRoutes.py <VISSIM_NETWORK> <OUTPUT_PREFIX>
```

The script is located in {{SUMO}}/tools/import/vissim. It is written in Python.

# vissim_parseBusStops.py

Parses bus stops and bus routes stored in the given Vissim file (first
parameter). The read bus lines are saved as
*<OUTPUT_PREFIX\>_busses.rou.xml*. The read routes are saved as
*<OUTPUT_PREFIX\>_stops.add.xml*. (Starting?) edges of the route may be
renamed by setting them within "edgemap" variable (see below).

```
vissim_parseBusStops.py <VISSIM_NETWORK> <OUTPUT_PREFIX>
```

The script is located in {{SUMO}}/tools/import/vissim. It is written in Python.