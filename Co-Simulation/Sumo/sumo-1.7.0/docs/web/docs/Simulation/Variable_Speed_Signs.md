---
title: Simulation/Variable Speed Signs
permalink: /Simulation/Variable_Speed_Signs/
---

One of the trigger objects that may be specified within an {{AdditionalFile}} allows the
simulation of variable speed signs. The syntax for such an object is:

```
<additional>
  <variableSpeedSign id="VSS_ID" lanes="LANE_IDS" file="DEF_FILE">
     <step time="<TIME>" speed="<SPEED>"/>

     ... further entries ...

  </variableSpeedSign>
</additional>
```

The following attributes are used within the `VariableSpeedSignal` element:

| Attribute Name | Value Type  | Description                                                  |
| -------------- | ----------- | ------------------------------------------------------------ |
| **id**         | id (string) | The id of the Variable Speed Signal element (user choice)    |
| **lanes**      | id (string) | The id of the lanes of the simulation network                |
| file           | string      | optional file in which the time and speed values are defined |

Each step is a combination of the time the next new speed shall be set
and the speed to set itself. A file name can be supplied, called
<DEF_FILE\> within the schema above. This file must contain the
information about when a certain speed shall be set onto the lane. This
file has the following format:

```
<vss>
  <step time="<TIME>" speed="<SPEED>"/>
  <step time="<TIME>" speed="<SPEED>"/>

  ... further entries ...

</vss>
```

The following attributes are used within the `VariableSpeedSignal` element:

| Attribute Name | Value Type     | Description                                                                                                 |
| -------------- | -------------- | ----------------------------------------------------------------------------------------------------------- |
| **time**       | time (int)     | Time in which the speed will be changed                                                                     |
| speed          | speed (double) | New speed (if no value or a negative value is given, the speed will be reset to the original network value) |

A small example for usage of vss' may be found in the SUMO directory
under "examples/sumo/variable_speed_signs".