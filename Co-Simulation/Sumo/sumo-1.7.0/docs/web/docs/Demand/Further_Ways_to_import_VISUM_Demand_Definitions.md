---
title: Demand/Further Ways to import VISUM Demand Definitions
permalink: /Demand/Further_Ways_to_import_VISUM_Demand_Definitions/
---

VISUM stores its demand in [OD-matrices which can be
imported](../Demand/Importing_O/D_Matrices.md) using
[od2trips](../od2trips.md). Though, it may be interesting for using
the computed assignments without performing a [dynamic user
assignment](../Demand/Dynamic_User_Assignment.md). The SUMO package
contains some scripts which allow to process other VISUM data and are
discussed in the following.

# Importing Turn Percentages

VISUM can save the defined/computed turning percentages at junctions.
The format differs from [turning probabilities
format](../Demand/Routing_by_Turn_Probabilities.md) used by
[jtrrouter](../jtrrouter.md). The script
***visum_convertTurnPercentages.py*** converts VISUM turning
percentages into [jtrrouter](../jtrrouter.md) [turning
definitions](../Demand/Routing_by_Turn_Probabilities.md). The tool
requires the SUMO-network converted from VISUM, the turning
probabilities from VISUM, and the name of the file into which the
converted turning probabilities shall be written:

```
visum_convertTurnPercentages.py <SUMO_NET> <VISUM_TURNINGS> <OUTPUT>
```

The script is located in {{SUMO}}/tools/import/visum. It is written in Python.

## Usability

It seems as using turning ratios for large areas would not make any
sense. The resulting routes are very unrealistic as they contain many
loops.

## See Also

- [jtrrouter](../jtrrouter.md) - page on the turning ratios
  router
- [Demand/Routing by Turn
  Probabilities](../Demand/Routing_by_Turn_Probabilities.md) -
  further documentation on this tool's usage

# Importing Routes

VISUM can save the routes it computes during the assignment. The format
differs from [Definition of Vehicles, Vehicle Types, and
Routes](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md)
used by [sumo](../sumo.md) and [sumo-gui](../sumo-gui.md). The
script ***visum_convertRoutes.py*** converts VISUM routes into
[sumo](../sumo.md) routes. The tool requires the SUMO-network
converted from VISUM, the routes exported from VISUM, and the name of
the file into which the converted turning probabilities shall be
written. Additional options are shown in the following table:

| Option                             | Description                           |
|------------------------------------|-----------------------------------------------------------------|
| **--net-file** {{DT_FILE}}<br>**-n** {{DT_FILE}}     | Read the SUMO-network to map the routes onto from <FILE\>; mandatory      |
| **--visum-routes** {{DT_FILE}}<br>**-r** {{DT_FILE}} | Read VISUM-routes to map from <FILE\>; mandatory                 |
| **--output** {{DT_FILE}}<br>**-o** {{DT_FILE}}       | Write generated routes to <FILE\>; mandatory              |
| **--begin** {{DT_INT}}<br>**-b** {{DT_INT}}          | Define the begin of the interval the vehicles are emitted within [s\]; default: 0s      |
| **--end** {{DT_INT}}<br>**-e** {{DT_INT}}            | Define the end of the interval the vehicles are emitted within [s\]; default: 3600s       |
| **--prefix** {{DT_STR}}<br>**-p** {{DT_STR}}   | Add <STRING\> as prefix to the IDs of generated vehicles; optional, default: no prefix      |
| **--type** {{DT_STR}}<br>**-t** {{DT_STR}}     | Set the vehicle type to <STRING\>; optional, default: no type              |
| **--uniform**<br>**-u**                    | Vehicle departure times will be spread across the interval uniformly; optional, default: false        |
| **--timeline** {{DT_STR}}<br>**-l** {{DT_STR}} | Applies a daily time line. The time line must be given as a list of 24 floats, each describing the percentage of emissions from the original number for each hour of a day; optional, default: no timeline |

Example call:

```
visum_convertRoutes.py -n <SUMO_NET> -r <VISUM_ROUTES> -o <OUTPUT> --uniform
```

The script is located in {{SUMO}}/tools/import/visum. It is written
in Python.

## Usability

The routes can be directly used within [sumo](../sumo.md) /
[sumo-gui](../sumo-gui.md).

## See Also

- [Examples of daily time
  lines](../Demand/Importing_O/D_Matrices.md#daily_time_lines)