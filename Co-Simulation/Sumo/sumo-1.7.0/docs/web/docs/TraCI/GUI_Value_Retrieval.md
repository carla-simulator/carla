---
title: TraCI/GUI Value Retrieval
permalink: /TraCI/GUI_Value_Retrieval/
---

## Command 0xac: Get GUI Variable

|  ubyte   | string  |
| :------: | :-----: |
| Variable | View ID |

Asks for the value of a certain GUI variable. The view id is usually
something like "View \#0", where the last digit counts the open
(sub-)windows in the simulation GUI. The id is shown in the view
title-bar when un-maximizing the view subwindow (this happens
automatically when [opening a new
view](../sumo-gui.md#multiple_views)).

The following variable values can be retrieved, the type of the return
value is also shown in the table.

<center>**Overview Retrievable GUI Variables**</center>

| Variable           | ValueType   | Description            | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)                |
| ------------------ | ----------- | ---------------------------------------------------------------- | -------------------------------------------------------- |
| zoom (0xa0)        | double      | the current zoom level (in %)                    | [getZoom](https://sumo.dlr.de/pydoc/traci._gui.html#GuiDomain-getZoom)         |
| offset (id 0xa1)   | 2D-position | the center of the currently visible part of the net              | [getOffset](https://sumo.dlr.de/pydoc/traci._gui.html#GuiDomain-getOffset)     |
| schema (id 0xa2)   | string      | the visualization scheme used (e.g. "standard" or "real world")  | [getSchema](https://sumo.dlr.de/pydoc/traci._gui.html#GuiDomain-getSchema)     |
| boundary (id 0xa3) | 2D-polygon  | the lower left and the upper right corner of the visible network | [getBoundary](https://sumo.dlr.de/pydoc/traci._gui.html#GuiDomain-getBoundary) |
| has view (id 0xa7) | bool        | Whether a view with the given ID exists                          | [hasView](https://sumo.dlr.de/pydoc/traci._gui.html#GuiDomain-hasView) |


## Response 0xbc: GUI Variable

|  ubyte   | string  |            ubyte            |  <return_type\>   |
| :------: | :-----: | :-------------------------: | :--------------: |
| Variable | View ID | Return type of the variable | <VARIABLE_VALUE\> |

The respond to a **"Command Get GUI Variable"**.