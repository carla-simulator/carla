---
title: TraCI/Change GUI State
permalink: /TraCI/Change_GUI_State/
---

# Command 0xcc: Change GUI State

|  ubyte   | string  |       ubyte       | <value_type\> |
| :------: | :-----: | :---------------: | :----------: |
| Variable | View ID | Type of the value |  New Value   |

Changes the simulation visualization. The view id that must be supplied
is usually something like "View \#0", where the last digit counts the
open (sub-)windows in the simulation GUI. The id is shown in the view
title-bar when un-maximizing the view subwindow (this happens
automatically when [opening a new view](../sumo-gui.md#multiple_views)).

Because it is possible to change different values, the number of
parameter to supply and their types differ between commands. The
following values can be changed, the parameter(s) which must be given
are also shown in the table.

<center>**Overview Changeable GUI Variables**</center>

| Variable             | ValueType                    | Description                                                                                                                                                                                                       | [Python Method](../TraCI/Interfacing_TraCI_from_Python.md)                  |
| -------------------- | ---------------------------- | -------------------------------------------------------------------- | ------------------------------------------ |
| zoom (0xa0)          | double                       | Sets the current zoom level in %           | [setZoom](https://sumo.dlr.de/pydoc/traci._gui.html#GuiDomain-setZoom)    |
| offset (0xa1)        | 2D-position (double, double) | Moves the center of the visible network to the given position   | [setOffset](https://sumo.dlr.de/pydoc/traci._gui.html#GuiDomain-setOffset)       |
| schema (0xa2)        | string     | Sets the visualization scheme (e.g. "standard")    | [setSchema](https://sumo.dlr.de/pydoc/traci._gui.html#GuiDomain-setSchema)       |
| boundary (0xa3)      | 2D-polygon (length 2)        | Sets the boundary of the visible network. If the window has a different aspect ratio than the given boundary, the view is expanded along one axis to meet the window aspect ratio and contain the given boundary. | [setBoundary](https://sumo.dlr.de/pydoc/traci._gui.html#GuiDomain-setBoundary)   |
| screenshot (0xa5)    | filename (string)            | Save a screenshot to the given file    | [screenshot](https://sumo.dlr.de/pydoc/traci._gui.html#GuiDomain-screenshot)  |
| track vehicle (0xa6) | vehicle ID (string)     | tracks the given vehicle in the GUI  | [trackVehicle](https://sumo.dlr.de/pydoc/traci._gui.html#GuiDomain-trackVehicle) |


The message contents are as following:

### zoom (0xa0)

|        ubyte        |     double     |
| :-----------------: | :------------: |
| value type *double* | New zoom level |

### offset (0xa1)

|          ubyte          |    double    |    double    |
| :---------------------: | :----------: | :----------: |
| value type *position2D* | x-coordinate | y-coordinate |

### schema (0xa2)

|        ubyte        |   string   |
| :-----------------: | :--------: |
| value type *string* | New schema |

### boundary (0xa3)

|        ubyte         | ubyte |         double          |         double          |          double          |          double          |
| :------------------: | :---: | :---------------------: | :---------------------: | :----------------------: | :----------------------: |
| value type *polygon* |   2   | x-coordinate lower left | y-ccordinate lower left | x-coordinate upper right | y-coordinate upper right |

### screenshot (0xa5)

|        ubyte        |       string        |
| :-----------------: | :-----------------: |
| value type *string* | Screenshot filename |

### track vehicle (0xa6)

|        ubyte        |      string       |
| :-----------------: | :---------------: |
| value type *string* | id of the vehicle |