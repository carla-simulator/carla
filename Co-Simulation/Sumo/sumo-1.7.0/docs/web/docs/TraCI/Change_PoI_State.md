---
title: TraCI/Change PoI State
permalink: /TraCI/Change_PoI_State/
---

# Command 0xc7: Change PoI State

|  ubyte   | string |       ubyte       | <value_type\> |
| :------: | :----: | :---------------: | :----------: |
| Variable | PoI ID | Type of the value |  New Value   |

Changes the state of a point-of-interest. Because it is possible to
change different values of a PoI, the number of parameter to supply and
their types differ between commands. The following values can be
changed, the parameter which must be given are also shown in the table.

<center>**Overview Changeable PoI Variables**</center>

| Variable          | ValueType                          | Description                      | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)                  |
| ----------------- | ---------------------------------- | -------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| type (0x4f)       | string                             | Sets the PoI's type to the given value                                                                   | [setType](https://sumo.dlr.de/pydoc/traci._poi.html#PoiDomain-setType)           |
| color (0x45)      | color (ubyte,ubyte,ubyte,ubyte)    | Sets the PoI's color to the given value (r,g,b,a) - please note that a(lpha) = 0 means fully transparent | [setColor](https://sumo.dlr.de/pydoc/traci._poi.html#PoiDomain-setColor)         |
| position (0x42)   | Position2D (double, double)        | Sets the PoI's position to the given value                                                               | [setPosition](https://sumo.dlr.de/pydoc/traci._poi.html#PoiDomain-setPosition)   |
| image file (0x93) | string                             | sets the path to the image file of the poi                                                               | [setImageFile](https://sumo.dlr.de/pydoc/traci._poi.html#PoiDomain-setImageFile) |
| width (0x4d)      | float                              | Sets the width for the rendered image file                                                               | [setWidth](https://sumo.dlr.de/pydoc/traci._poi.html#PoiDomain-setWidth)         |
| height (0xbc)     | float                              | Sets the height for the rendered image file                                                              | [setHeight](https://sumo.dlr.de/pydoc/traci._poi.html#PoiDomain-setHeight)       |
| angle (0x43)      | float                              | sets the angle for the rendered image file                                                               | [setAngle](https://sumo.dlr.de/pydoc/traci._poi.html#PoiDomain-setAngle)         |
| ADD (0x80)        | PoI-definition, see below          | Adds the defined PoI                                                                                     | [add](https://sumo.dlr.de/pydoc/traci._poi.html#PoiDomain-add)                   |
| REMOVE (0x81)     | int (layer), see below             | Removes the defined PoI                                                                                  | [remove](https://sumo.dlr.de/pydoc/traci._poi.html#PoiDomain-remove)             |
| highlight (0x6c)  | highlight specification, see below | Adds a highlight to the PoI                                                                              | [remove](https://sumo.dlr.de/pydoc/traci._poi.html#PoiDomain-remove)             |
|                   |                                    |                                                                                                          |                                                                                  |



The message contents are as following:

### type (0x4f)

|        ubyte        |    string     |
| :-----------------: | :-----------: |
| value type *string* | New Type Name |

### color (0x45)

|       ubyte        | ubyte | ubyte | ubyte | ubyte |
| :----------------: | :---: | :---: | :---: | :---: |
| value type *color* |  red  | green | blue  | alpha |

### position (0x42)

|          ubyte          |    double    |    double    |
| :---------------------: | :----------: | :----------: |
| value type *position2D* | x-coordinate | y-ccordinate |

### ADD (0x80)

|         ubyte         |       int       |        ubyte        |  string   |       ubyte        | color |      ubyte       |  int  |          ubyte          | position |
| :-------------------: | :-------------: | :-----------------: | :-------: | :----------------: | :---: | :--------------: | :---: | :---------------------: | :------: |
| value type *compound* | item number = 4 | value type *string* | type name | value type *color* | color | value type *int* | layer | value type *position2D* | position |

If the PoI could not been added because another one with the same ID
already exists within the layer, an error message is generated.

### REMOVE (0x81)

|      ubyte       |  int  |
| :--------------: | :---: |
| value type *int* | layer |

If the named PoI can not be found in the given layer, all PoIs with the
given ID are removed (from all layers). If no PoI with the given ID
could be found, an error message is generated.

### highlight (0x6c)

|   ubyte  |      int    |  ubyte | color | ubyte  | double   |  ubyte  |  ubyte   |   ubyte   |  double    |     ubyte        |            ubyte                      |
| :-----------: | :-----------: | :-------------: | :---: | :-----------------: | :-------: | :--------: | :-------: | :-------: | :------: | :----------------: | :----------: |
| value type *compound* | number of items in {0,1,2,3,4,5} | value type *color* | color | value type *double* | size (radius in m.) | value type *ubyte* | maximal alpha value | value type *double* | duration (in secs.) | value type *ubyte* | type ID (allows different highlight categories) |

The highlight method adds a circle of the specified size and color
centered at the POI. The circles alpha value is animated over the
specified duration (fade in and out) if a positive value is given,
maxAlpha <= 0 implies persistent highlighting without animation.