---
title: Simulation/Output/VTypeProbe
permalink: /Simulation/Output/VTypeProbe/
---

## Instantiating within the Simulation

A "vehicle-type probe" is defined the following way within an {{AdditionalFile}}:
`<vTypeProbe id="<ID>" [ type="<VEHICLE_TYPE>" ] freq="<OUTPUT_FREQUENCY>" file="<OUTPUT_FILE>"/>`

`type` names the vehicle type to observe. Only
the values of vehicles of this type will be written into the output. If
`type` is empty, the information about all
vehicles are included. In contrary to the detectors described above, the
values are not aggregated. This means that frequency does not describe
an aggregation interval but the frequency with which the values shall be
collected and written.

The following table shows the attributes:

| Attribute Name | Value Type        | Description                                               |
| -------------- | ----------------- | --------------------------------------------------------- |
| **id**         | id (string)       | A string holding the id of the detector                   |
| **freq**       | int (time in s)   | The frequency with which information shall be written.    |
| **file**       | <FILE\>            | The path to the output file. The path may be relative.    |
| type           | (vehicle type) id | The type the vehicles must be of in order to be reported. |

Example:

```
<additional>
    <vTypeProbe id="probe1" type="DEFAULT_VEHTYPE" freq="10" file="output.xml"/>
</additional>
```

This file is then loaded in [sumo](../../sumo.md) using the option **--additional-files**

## Generated Output

The output is divided into timestep-sections:

```
<timestep time="<COLLECTION_TIME>" id="<DETECTOR_ID>" vtype="<OBSERVED_TYPE>">
    <vehicle id="<VEHICLE_ID>" lane="<LANE_ID>" pos="<POSITION_ON_LANE>" \
        x="<X-COORDINATE>" y="<Y-COORDINATE>" \
        lat="<LAT-COORDINATE>" lon="<LON-COORDINATE>" \
        speed="<VEHICLE_SPEED>"/>
    ... further vehicles ...
</timestep>
... further time steps ...
```

The values are described in the following table.

| Name           | Type                 | Description                                          |
| -------------- | -------------------- | ---------------------------------------------------- |
| timestep:time  | (simulation) seconds | The time this information describes                  |
| timestep:id    | id                   | The id of the detector                               |
| timestep:vtype | id                   | The id of the vehicle type observed by this detector |
| vehicle:id     | (vehicle) id         | The id of the described vehicle                      |
| lane           | (lane) id            | The id of the lane the vehicle was on                |
| pos            | m                    | The position of the vehicle on lane                  |
| x              | m                    | The x-position of the vehicle within the net         |
| y              | m                    | The y-position of the vehicle within the net         |
| lat            | degrees\*100,000     | The lat-position of the vehicle within the net       |
| lon            | degrees\*100,000     | The lon-position of the vehicle within the net       |
| speed          | m/s                  | The speed of the vehicle within the time step        |