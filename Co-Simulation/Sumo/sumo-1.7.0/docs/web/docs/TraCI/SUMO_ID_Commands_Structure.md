---
title: TraCI/SUMO ID Commands Structure
permalink: /TraCI/SUMO_ID_Commands_Structure/
---

# General Structure of Plain SUMO ID Commands

All second generation API commands and its answers have the same
structure which will be described here.

## Value Retrieval Commands

These commands ask SUMO about a special property value of a certain
object.

### Command to SUMO

|  ubyte   | string  |
| :------: | :-----: |
| variable | SUMO ID |

The variable depends on the particular command and selects a special
value. They are described in the documentation of the particular
command. The SUMO ID is the ID of the object from which you want to know
the value. Which type of object it belongs to depends on the particular
command.

### Answer from SUMO

|  ubyte   | string  |            ubyte            |  <return_type\>   |
| :------: | :-----: | :-------------------------: | :--------------: |
| variable | SUMO ID | return type of the variable | <VARIABLE_VALUE\> |

Variable and SUMO ID repeat the values from the command. The return type
depends on the variable and can be either a general TraCI data type
described [here](../TraCI/Protocol.md#data_types) or a special
compound type for this variable which is documented with the particular
command.

## State Change Commands

These commands are the opposites of the value retrieval commands above.
They set a certain value of a particular object.

### Command to SUMO

|  ubyte   | string  |       ubyte       | <value_type\> |
| :------: | :-----: | :---------------: | :----------: |
| variable | SUMO ID | type of the value |  new value   |

Additional to variable and SUMO ID like the value retrieval commands we
have a value type and the value itself similar to the answer of the
value retrieval commands. The allowed types depend on the variable which
itself depends on the command. See documentation of the particular
command for details.

### Answer from SUMO

The answer to a state change command always consists only of the status
response described [here](../TraCI/Protocol.md#status_response).
Nothing else will be returned.