---
title: TraCI/Polygon Value Retrieval
permalink: /TraCI/Polygon_Value_Retrieval/
---

## Command 0xa8: Get Polygon Variable

|  ubyte   |   string   |
| :------: | :--------: |
| Variable | Polygon ID |

Asks for the value of a certain variable of the named polygon.

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>
**Overview Retrievable Polygon Variables**
</center>

| Variable         | ValueType               | Description             | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)     |
| ---------------- | ----------------------- | ----------------------- | -------------------------------------------------------------- |
| id list (0x00)   | stringList              | Returns a list of ids of all polygons (the given polygon ID is ignored) | [getIDList](https://sumo.dlr.de/pydoc/traci._polygon.html#PolygonDomain-getIDList)       |
| count (0x01)     | int                     | Returns the number of polygons (the given polygon ID is ignored)        | [getIDCount](https://sumo.dlr.de/pydoc/traci._polygon.html#PolygonDomain-getIDCount)     |
| type (0x4f)      | string                  | Returns the (abstract) type of the polygon                              | [getType](https://sumo.dlr.de/pydoc/traci._polygon.html#PolygonDomain-getType)           |
| color (0x45)     | ubyte,ubyte,ubyte,ubyte | Returns the color of this polygon (rgba)                                | [getColor](https://sumo.dlr.de/pydoc/traci._polygon.html#PolygonDomain-getColor)         |
| shape (0x4e)     | 2D-polygon              | Returns the shape (list of 2D-positions) of this polygon                | [getShape](https://sumo.dlr.de/pydoc/traci._polygon.html#PolygonDomain-getShape)         |
| filled (0x55)    | int                     | Returns whether this polygon is filled (1) or not (0)                   | [getFilled](https://sumo.dlr.de/pydoc/traci._polygon.html#PolygonDomain-getFilled)       |
| lineWidth (0x4d) | double                  | Returns the line width for drawing unfilled polygon                     | [getLineWidth](https://sumo.dlr.de/pydoc/traci._polygon.html#PolygonDomain-getLineWidth) |

### Response 0xb8: Polygon Variable

|  ubyte   |   string   |            ubyte            |  <return_type\>   |
| :------: | :--------: | :-------------------------: | :--------------: |
| Variable | Polygon ID | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get Polygon Variable"**.