---
title: Simulation/Output/VehRoutes
permalink: /Simulation/Output/VehRoutes/
---

The vehicle routes output contains information about which route a
vehicle took and if his route was replaced at any time by a new one,
each of the previous routes together with the edge at the time their
replacement took place is reported. Furthermore, the times the vehicle
has entered and has left the network are stored herein.

In normal conditions, when all vehicles use predefined routes, the
output does not contain any information that could not be retrieved from
the routes and the tripinfo output. But as soon as you reroute your
vehicles within the simulation, f.e. using rerouters, it will contain
new information.

## Instantiating within the Simulation

This output is enabled for a simulation using the option **--vehroute-output** {{DT_FILE}} or **--vehroutes** {{DT_FILE}}. Further
options affecting this output are listed at
[SUMO\#Output](../../sumo.md#output).

## Generated Output

The generated file look like this:

```
<routes>
    <vehicle id="<VEHICLE_ID>" [type="<TYPE_ID>"] depart="<INSERTION_TIME>" arrival="<ARRIVAL_TIME>" [routeLength="<LENGTH>"]>
        <routeDistribution>
            <route replacedOnEdge="<EDGE_ID>" replacedAtTime="<TIME>" probability="0" edges="<PREVIOUS_ROUTE>"/>

            ... further replaced routes ...

            <route edges="<LAST_ROUTE>" [exitTimes="<EXIT_TIMES>"]/>
        <routeDistribution>
    </vehicle>

    <person id="<PERSON_ID>" depart="<INSERTION_TIME>" arrival="<ARRIVAL_TIME>">
        <ride from="..." to="..." lines="..."/>
        <walk edges="..." speed="..."/>
    </person>

    ... information about further vehicles and persons ...

</routes>
```

| Name             | Type            | Description                                                                                 |
| ---------------- | --------------- | ------------------------------------------------------------------------------------------- |
| id               | (vehicle) id    | The id of the vehicle this entry describes                                                  |
| type             | vehicle type id | The id of the vehicle type if different from the default                                    |
| depart           | s               | The time the vehicle was emitted into the network                                           |
| arrival          | s               | The time the vehicle was removed from the simulation (due to arriving at the route end)     |
| routeLength      | m               | The total length of the vehicle's route (if activated by the vehroutes.route-length option) |
| replacedOnEdge   | (edge) id       | The edge the vehicle was on when the described route was replaced                           |
| replacedAtTime   | s               | The time step of this replacement                                                           |
| <PREVIOUS_ROUTE\> | \[(edge) id\]+  | The replaced route                                                                          |
| <LAST_ROUTE\>     | \[(edge) id\]+  | The final vehicle route                                                                     |
| <EXIT_TIMES\>     | \[time in s\]+  | The leave time for every edge in the route, if enabled with the option **--vehroute-output.exit-times**  |

!!! note
    Additional attributes of the vehicle are also included if they were set.

Both the previous and the final routes are complete, that means that
they contain all the edges the vehicle was meant to pass as long as the
route was not replaced, yet. The information replacedOnEdge and
replacedAtTime are available only for routes which were replaced. The
probability field is only given to make the file directly usable as
input to the simulation which lets the vehicle travel its final route.
For proper simulation input the file should be also sorted by departure
time. This can be accomplished by also setting option **--vehroute-output.sorted**.

## Output only for selected vehicles or vehicle types
By default all vehicles will generated vehroute-output. By [assigning a vehroute device](../../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#devices) to selected vehicles or types, this can be changed.

Example:
Setting SUMO-option **--device.vehroute.probability false** will disable the device for all vehicles by default.
The following definition overrides this default end enables the device for busses:
```
<vType id="bus">
  <param key="has.vehroute.device" value="true"/>
</vType>
```
