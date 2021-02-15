---
title: Simulation/Output/FCDOutput
permalink: /Simulation/Output/FCDOutput/
---

The FCD (floating car data) export contains location and speed along
with other information for every vehicle in the network at every time
step. The output behaves somewhat like a super-accurate high-frequency
GPS device for each vehicle. The outputs can be processed further using
the [TraceExporter tool](../../Tools/TraceExporter.md) for adapting
frequency, equipment rates, accuracy and data format.

## Instantiating within the Simulation

The simulation generates this output when setting the option **--fcd-output** {{DT_FILE}}. {{DT_FILE}} is the
name of the file the output will be written to. Any other file with this
name will be overwritten, the destination folder must exist.

By default, fcd-output is enabled for all vehicles and persons in the
simulation with output in each simulation step. Using [device assignment
options or
parameters](../../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#devices)
(i.e. **--device.fcd.probability 0.25**) the set of vehicles which generate fcd output can be reduced.
The output period can be set by using option **--device.fcd.period** {{DT_TIME}}.

## Generated Output

The generated XML file looks like this:

```
<fcd-export>

  <timestep time="<TIME_STEP>">
      <vehicle id="<VEHICLE_ID>" x="<VEHICLE_POS_X>" y="<VEHICLE_POS_Y>" angle="<VEHICLE_ANGLE>" type="<VEHICLE_TYPE>"
      speed="<VEHICLE_SPEED>"/>

      ... more vehicles ...

  </timestep>

  ... next timestep ...

</fcd-export>
```

| Name     | Type                 | Description                                                                                                             |
| -------- | -------------------- | ----------------------------------------------------------------------------------------------------------------------- |
| timestep | (simulation) seconds | The time step described by the values within this timestep-element                                                      |
| id       | id                   | The id of the vehicle                                                                                                   |
| type     | id                   | The name of the vehicle type                                                                                            |
| speed    | m/s                  | The speed of the vehicle                                                                                                |
| angle    | degree               | The angle of the vehicle in navigational standard (0-360 degrees, going clockwise with 0 at the 12'o clock position)    |
| x        | m or longitude       | The absolute X coordinate of the vehicle (center of front bumper). The value depends on the given geographic projection |
| y        | m or latitude        | The absolute Y coordinate of the vehicle (center of front bumper). The value depends on the given geographic projection |
| z        | m                    | The z value of the vehicle (center of front bumper).<br><br>**Note:** This value is only present if the network contains elevation data      |
| pos      | m                    | The running position of the vehicle measured from the start of the current lane.                                        |
| lane     | id                   | The id of the current lane.                                                                                             |
| slope    | degree               | The slope of the vehicle in degrees (equals the slope of the road at the current position)                              |
| signals  | bitset               | The [signal state information](../../TraCI/Vehicle_Signalling.md) (blinkers, etc). Only present when option **--fcd-output.signals** is set.  |

When the option **--fcd-output.geo** is set, the written (x,y)-coordinates will be the
lon/lat geo-coordinates.

### Precision

By default fcd-output returns location values in meter with a precision
of 1cm. (changeable by setting option **--precision**) If you set option
--fcd-output.geo the values are lon,lat as decimal values with a
precision of 6 decimal places (changeable by setting option **--precision.geo**)

## Person and Container Output

Any persons or container in the simulation will cause output of he
following form:

```
<fcd-export>

  <timestep time="<TIME_STEP>">
      <vehicle .../>
      ...
      <person id="..." x="..." y="..." angle="..." type="..." speed="..." edge="..." slope="..."/>
      ...
      <container id="..." x="..." y="..." angle="..." type="..." speed="..." edge="..." slope="..."/>
      ...
  </timestep>

  ... next timestep ...

</fcd-export>
```

If persons or containers are transported within a vehicle, their
respective `<person>` and `<container>` elements will be written as child elements of that
vehicle.

## Filtering / Restricting Output

!!! caution
    The generated output files can become quite large. To write [gzipped](https://en.wikipedia.org/wiki/Gzip) output files, simply name the output file with an `.gz` extension.

### Restricting the set of vehicles that generate output
Output can be restricted to specific vehicle types or vehicle ids by [controlling the set of vehicles that are equipped](../../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#devices)   with the **fcd**-device. The following example restricts output to a
  single vehicle called *ego*:
```
--device.fcd.explicit ego
```

### Restricting the locations
Output can be restricted to a specific set of edges by loading a list of edges from a file with option **--fcd-output.filter-edges.input-file** {{DT_FILE}}. The file format for
this is the same as the one when saving selections in  [netedit](../../netedit.md):
```
edge:id1
edge:id2
...
```

### Restricting output by sensor range
When not all vehicles are equipped with an **fcd**-device, other ehicles and persons in a radius around the equipped vehicles can be included in the output by setting option **--device.fcd.radius** to the desired range in m.

## Further Options

- **--fcd-output.geo** will toggle output coordinates to WGS84 (for
  geo-referenced networks)
- **--fcd-output.signals** will add [signal state
  information](../../TraCI/Vehicle_Signalling.md) to the output
- **--fcd-output.distance** will add [kilometrage](../Railways.md#kilometrage-mileage-chainage) information to the output
- **--fcd-output.acceleration** will add acceleration data to the output (also lateral acceleration when using the [sublane model](../SublaneModel.md)
- **--fcd-output.params KEY1,KEY2,...** adds generic parameters](Simulation/GenericParameters.md) to the output

## NOTES

In combination with the given geometry of the vehicles (shapes) you can
build some nice animations, e.g [NASA
WorldWind](http://worldwind.arc.nasa.gov/java/) or [Google
Earth](http://earth.google.com).