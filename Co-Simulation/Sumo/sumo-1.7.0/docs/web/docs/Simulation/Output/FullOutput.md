---
title: Simulation/Output/FullOutput
permalink: /Simulation/Output/FullOutput/
---

The idea behind this option is to improve the capabilities of the
existing netstate-dump option. The produced XML Structure contains
information about the edges, lanes, vehicles and traffic lights. The
intention for this option was to check the simulation results without
having to record all the simulation commands e.g. traci.

## Instantiating within the Simulation

To force SUMO to build a file that contains the full dump, extend your
command line parameter by **--full-output** {{DT_FILE}}. {{DT_FILE}} is the name of the file the output will be
written to. Any other file with this name will be overwritten, the
destination folder must exist.

## Generated Output

The full dump is a xml-file containing informtation about every edge,
lane, vehicle and traffic light for each time step. A full dump-file
looks like this:

```
<full-export>
    <data timestep="<TIME_STEP>">

    <vehicles>
        <vehicle id="<VEHICLE_ID>" eclass="<VEHICLE_ECLASS>" co2="<VEHICLE_CO2>" co="<VEHICLE_CO>" hc="<VEHICLE_HC>"
        nox="<VEHICLE_NOX>" pmx="<VEHICLE_PMX>" fuel="<VEHICLE_FUEL>" electricity="<VEHICLE_ELECTRICITY>" noise="<VEHICLE_NOISE>" route="<VEHICLE_ROUTE>" type="<VEHICLE_TYPE>"
        waiting="<VEHICLE_WAITING>" lane="<VEHICLE_LANE>" pos_lane="<VEHICLE_POS_LANE>" speed="<VEHICLE_SPEED>"
        angle="<VEHICLE_ANGLE>" x="<VEHICLE_POS_X>" y="<VEHICLE_POS_Y>"/>

        ... more vehicles ...

    </vehicles>

    <edges>

        <edge id="<EDGE_ID>" traveltime="<EDGE_TRAVELTIME>">

        <lane id="<LANE_ID>" co="<LANE_CO>" co2="<LANE_CO2>" nox="<LANE_NOX>" pmx="<LANE_CO>"
        hc="<LANE_HC>" noise="<LANE_NOISE>" fuel="<LANE_FUEL>" electricity="<LANE_ELECTRICITY>" maxspeed="<LANE_MAXSPEED>" meanspeed="<LANE_MEANSPEED>"
        occupancy="<LANE_OCCUPANCY>" vehicle_count="<LANE_VEHICLES_COUNT>"/>

            ... more lanes of the edge if exists

        </edge>

            ... more edges of the network

    </edges>

    <tls>
        <trafficlight id="0/0" state="GgGr"/>
        ... more traffic lights

    </tls>

</data>

... the next timestep ...

</full-export>
```

The meanings of the written values are given in the following table.

| Name                | Type                 | Description                                                                                                             |
| ------------------- | -------------------- | ----------------------------------------------------------------------------------------------------------------------- |
| time_step          | (simulation) seconds | The time step described by the values within this timestep-element                                                      |
| id                  | id                   | The id of the vehicle/lane/edge/trafficlight                                                                            |
| eclass              | id                   | The id of the specific emission class of the vehicle                                                                    |
| co2\@vehicle         | mg/s                 | The amount of CO2 emitted by the vehicle in the actual simulation step                                                  |
| co2\@lane            | mg/s                 | The complete amount of CO2 emitted by the vehicles on this lane during the actual simulation step                       |
| co\@vehicle          | mg/s                 | The amount of CO emitted by the vehicle in the actual simulation step                                                   |
| co\@lane             | mg/s                 | The complete amount of CO emitted by the vehicles on this lane during the actual simulation step                        |
| hc\@vehicle          | mg/s                 | The amount of HC emitted by the vehicle in the actual simulation step                                                   |
| hc\@lane             | mg/s                 | The complete amount of HC emitted by the vehicles on this lane during the actual simulation step                        |
| nox\@vehicle         | mg/s                 | The amount of NOX emitted by the vehicle in the actual simulation step                                                  |
| nox\@lane            | mg/s                 | The complete amount of NOX emitted by the vehicles on this lane during the actual simulation step                       |
| pmx\@vehicle         | mg/s                 | The amount of PMX emitted by the vehicle in the actual simulation step                                                  |
| pmx\@lane            | mg/s                 | The complete amount of PMX emitted by the vehicles on this lane during the actual simulation step                       |
| noise\@vehicle       | dB                   | The noise emitted by the vehicle in the actual simulation step                                                          |
| noise\@lane          | dB                   | The noise emitted by the vehicles on the specific lane                                                                  |
| fuel\@vehicle        | ml/s                 | The fuel consumed by the vehicle in the actual simulation step                                                          |
| fuel\@lane           | ml/s                 | The fuel consumed by the vehicles on the specific lane                                                                  |
| electricity\@vehicle | Wh/s                 | The electricity consumed by the vehicle in the actual simulation step                                                   |
| electricity\@lane    | Wh/s                 | The electricity consumed by the vehicles on the specific lane                                                           |
| route               | id                   | The name of the route                                                                                                   |
| type                | id                   | The name of the vehicle type                                                                                            |
| waiting             | seconds              | The total time a vehicle is waiting                                                                                     |
| lane                | id                   | The name of the lane                                                                                                    |
| pos                 | meters               | The position of the vehicle on a specific lane (distance of the front bumper from the start of the lane)                |
| speed               | m/s                  | The actual speed of the vehicle                                                                                         |
| angle               | degree               | The angle of the vehicle                                                                                                |
| pos_x              | \---                 | The absolut X coordinate of the vehicle (center of front bumper). The value depends on the given geographic projection. |
| pos_y              | \---                 | The absolut Y coordinate of the vehicle (center of front bumper). The value depends on the given geographic projection  |
| traveltime          | seconds              | The mean travel time on the specific lane                                                                               |
| fuel\@lane           | l/km/h               | The fuel consumption on the specific lane                                                                               |
| maxspeed            | m/s                  | The maximum speed of the vehicles on the specific lane                                                                  |
| meanspeed           | m/s                  | The mean speed of the vehicles on the specific lane                                                                     |
| occupancy           | %                    | The occupancy of the lane in %                                                                                          |
| vehicles_count     | \#veh                | The number of vehicles on the lane                                                                                      |
| state               | string               | The current [state of a traffic light](../../Simulation/Traffic_Lights.md)                                            |

## Notes

The generated files will be very large, you may pipe this output
directly to a compression tool like bzip2. The advantage of this large
file is that you can write some nice XML Stylesheets to export
everything what is important to you, for example the states of the
traffic lights and the travel times for the lanes.