---
title: Simulation/Shapes
permalink: /Simulation/Shapes/
---

# Using additional Polygons and POIs within the Simulation

*Example of using polygons and POIs; from Traffic Online, area\#1*
![tol1_with_polys.gif](../images/Tol1_with_polys.gif "Example of using polygons and POIs; from Traffic Online, area#1")

Definitions of colored polygons and POIs (points of interest) can be
loaded within an [*additional-file*](../sumo.md#format_of_additional_files). These shapes are currently meant to improve a
simulation's appearance and to allow an easier debugging. No special
interaction with them is implemented, yet.

Both polygons and points of interest may be located within a "layer".
Shapes with lowest layer values are below those with a higher layer
number. The network itself is drawn as layer 0. An additional file may
contain definitions for both points of interest and polygons.

Geometrical objects may either be defined "by hand" or imported using
[polyconvert](../polyconvert.md). A valid geometry-file can be
given to [sumo](../sumo.md) as one of the additional files (option:
**--additional-files <FILE\>**). For usage within
[sumo-gui](../sumo-gui.md), you have to add it to the list of
additional files to load within the used [configuration
file](../Basics/Using_the_Command_Line_Applications.md#configuration_files)
or load the additional file interactively through the
[GUI](../sumo-gui.md#loading_shapes_and_pois).

# Definitions

The geometrical objects (POIs, polygons) are stored one by one into an
"additional file". Currently, the root element may arbitary.

## Polygon Definitions

A polygon is defined as following: `<poly id="<POLYGON_ID>" type="<TYPENAME>" color="<COLOR>" fill="<FILL_OPTION>" layer="<LAYER_NO>" shape="<2D-POSITION>[ <2D-POSITION>]*"/>`

These attributes have the following meanings:

| Attribute Name | Value Type       | Description                                                                         |
| -------------- | ---------------- | ----------------------------------------------------------------------------------- |
| **id**         | id (string)      | The id (a unique name) of the polygon                                               |
| **shape**      | 2D position list | The shape of the polygon                                                            |
| color          | color            | The RGBA color with which the polygon shall be displayed; see {{DT_Color}} for details          |
| geo            | bool             | Whether the shape shall be interpreted as geo-coordinates and converted             |
| fill           | bool             | An information whether the polygon shall be filled; optional bool, *default: false* |
| lineWidth      | double           | Drawing width of unfilled polygons in m, *default 1*                                |
| layer          | float            | The layer in which the polygon lies; optional                                       |
| type           | string           | A typename for the polygon.                                                         |
| imgFile        | string           | A bitmap to use for rendering this polygon                                          |
| angle          | float            | angle of rendered image in degree                                                   |

## POI (Point of interest) Definitions

A point-of-interest is defined as following: `<poi id="<POLYGON_ID>" type="<TYPENAME>" color="<RED>,<GREEN>,<BLUE>" layer="<LAYER_NO>" [(x="<X_POS>" y="<Y_POS>") | (lane="<LANE_ID>" pos="<LANE_POS>")]/>`

It means that the position a point-of-interest is located at may be
given either using explicite x/y-coordinates or a lane name and a
position on this lane. So, the attributes have the following meanings:

| Attribute Name        | Value Type  | Description                                                                                                                                                                                            |
| --------------------- | ----------- | ---------------------------------------------------------------------------------- |
| **id**                | id (string) | The id (a unique name) of the polygon                                                                                                                                                                  |
| **color**             | color       | The color with which the poi shall be displayed; *<RED\>*, *<GREEN\>*, and *<BLUE\>* must be floating point numbers between 0 and 1. They are devided using a ',' (no space); optional, *default "1,0,0"* |
| x<sup>(\*)</sup>      | float       | The position of the poi along the x-axis in meters                                                                                                                                                     |
| y<sup>(\*)</sup>      | float       | The position of the poi along the y-axis in meters                                                                                                                                                     |
| lane<sup>(\*)</sup>   | id (string) | The name of the lane the poi is located at; the lane must be a part of the loaded network                                                                                                              |
| pos<sup>(\*)</sup>    | float       | The position on the named lane at which the poi is located at                                                                                                                                          |
| posLat<sup>(\*)</sup> | float       | The lateral offset on the named lane at which the poi is located at (negative values lie on the right side of the lane center line in driving direction)                                               |
| lon<sup>(\*)</sup>    | float       | The geo-position of the poi along the east-west axis in degrees                                                                                                                                        |
| lat<sup>(\*)</sup>    | float       | The geo-position of the poi along the north-south axis in degrees                                                                                                                                      |
| type                  | string      | A typename for the poi.                                                                                                                                                                                |
| layer                 | float       | the layer of the poi for drawing and selecting.                                                                                                                                                        |
| imgFile               | string      | A bitmap to use for rendering this poi. If none is given, circle is drawn instead. The bitmap is tinted by the given color unless for white (*"1,1,1"*).                                               |
| width                 | float       | width of rendered image in meters                                                                                                                                                                      |
| height                | float       | height of rendered image in meters                                                                                                                                                                     |
| angle                 | float       | angle of rendered image in degree                                                                                                                                                                      |

<sup>(\*)</sup> Either `x/y` or `lane/pos` or `lon/lat` must be given

!!! note
    When specifying the attributes *lane*, *pos* and *posLat*, they are automatically added as [generic parameters](../Simulation/GenericParameters.md) accessible via TraCI.

# See Also

- See the description of [polyconvert](../polyconvert.md) in
  order to know how polygons/POIs can be imported from other sources
- [Developer/Implementation Notes/Drawing in
  sumo-gui](../Developer/Implementation_Notes/Drawing_in_sumo-gui.md)
  describes how [sumo-gui](../sumo-gui.md) renders loaded
  structures (for developers)
- You can [read variables of loaded
  PoIs](../TraCI/POI_Value_Retrieval.md) and [read variables of
  loaded polygons](../TraCI/Polygon_Value_Retrieval.md) via
  [TraCI](../TraCI.md)
- You can [add PoIs and manipulate their
  properties](../TraCI/Change_PoI_State.md) and [the same for
  polygons](../TraCI/Change_Polygon_State.md) via
  [TraCI](../TraCI.md)