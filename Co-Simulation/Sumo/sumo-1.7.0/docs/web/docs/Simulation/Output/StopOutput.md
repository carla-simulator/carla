---
title: Simulation/Output/StopOutput
permalink: /Simulation/Output/StopOutput/
---

## Instantiating within the Simulation

Stop output is activated by setting the simulation option **--stop-output** {{DT_FILE}} on the
command line or in a *.sumocfg* file. This output contains the
information about each vehicle's scheduled
[`<stop>`](../../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#stops):
time of arrival and departure, stopping place and number of persons or
container that were loaded and unloaded. The information is generated
every time a stop ends.

## Generated Output

The format is the following:

```
<stops>
    <stopinfo ''ATTRIBUTES''...\>
    ...
</stops>
```

The following output attributes are generated:

| Name               | Type          | Description                                                          |
| ------------------ | ------------- | -------------------------------------------------------------------- |
| id                 | (vehicle) id  | The name of the vehicle that stopped                                 |
| type               | (type) id     | The type of the vehicle that stopped                                 |
| lane               | id            | The lane on which the vehicle stopped                                |
| pos                | double        | The lane position at which the vehicle stopped                       |
| parking            | bool          | Whether the vehicle left the road (*true*) or stayed on it (*false*) |
| started            | time (s)      | The time at which the vehicle stopped                                |
| ended              | time (s)      | The time at which the vehicle resumed driving                        |
| initialPersons     | int           | The number of persons in the vehicle when arriving at the stop       |
| loadedPersons      | int           | The number of persons that entered the vehicle at this stop          |
| unloadedPersons    | int           | The number of persons that left the vehicle at this stop             |
| initialContainers  | int           | The number of containers in the vehicle when arriving at the stop    |
| loadedContainers   | int           | The number of containers that entered the vehicle at this stop       |
| unloadedContainers | int           | The number of containers that left the vehicle at this stop          |
| busStop            | id (optional) | The id of the busStop if this stop took place at a named `<busStop/>`|
| containerStop      | id (optional) | The id of the containerStop if this stop took place at a named `<containerStop/>` |
| parkingArea        | id (optional) | The id of the parkingArea if this stop took place at a named  `<parkingArea/>`    |
| chargingStation    | id (optional) | The id of the chargingStation if this stop took place at a named `<chargingStation/>`  |