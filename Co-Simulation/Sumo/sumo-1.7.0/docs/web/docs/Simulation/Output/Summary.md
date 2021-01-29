---
title: Simulation/Output/Summary
permalink: /Simulation/Output/Summary/
---

This output contains the simulation-wide number of vehicles that are
loaded, inserted, running, waiting to be inserted, have reached their
destination and how long they needed to finish the route. The last value
is normalised over all vehicles that have reached their destination so
far.

## Instantiating within the Simulation

You can force the simulation to generate this output using **--summary** {{DT_FILE}}.

## Generated Output

The information containing all those values is computed for each time
step and the output file looks like following:

```
<summary>
    <step time="<SIMULATION_TIME>" \
            loaded="<LOADED_VEHICLE_NUMBER>" \
            inserted="<INSERTED_VEHICLE_NUMBER>" \
            running="<RUNNING_VEHICLE_NUMBER>" \
            waiting="<NUMBER_OF_VEHICLES_WAITING_FOR_INSERTION>" \
            ended="<ENDED_VEHICLE_NUMBER>" \
            meanWaitingTime="<MEAN_WAITING_TIME>" \
            meanTravelTime="<MEAN_TRAVEL_TIME>"/>

    ... further time steps ...

</summary>
```

Please note, that in contrary to the example above, for each time step,
all those values are reported in one line. A description of the values
is given in the table below.

| Name              | Type                 | Description                                                                                                                                                             |
| ----------------- | -------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| time              | (simulation) seconds | The time step described by the entry                                                                                                                                    |
| loaded            | \#                   | Number of vehicles that were loaded from input files up to this time step. This can included vehicle with depart times in the future.
| inserted          | \#                   | Number of vehicles inserted so far (including reported time step)                                                                                                       |
| running           | \#                   | Number of vehicles that were running within the reported time step                                                                                                      |
| waiting           | \#                   | Number of vehicles which were waiting for insertion (could not be inserted) within the reported time step                                                               |
| ended             | \#                   | Number of vehicles that have reached their destination so far or that were otherwise removed from the simulation (including reported time step)                         |
| arrived           | \#                   | Number of vehicles that have reached their destination so far                                                                                                           |
| halting           | \#                   | The number of vehicles in the network with speed below 0.1m/s (which are not waiting at a `<stop>`).                                                                            |
| collisions        | \#                   | The number of vehicles that were involved in a collision                                                                                                                |
| teleports         | \#                   | The number of vehicles in the network that were teleported (due to jamming or collisions)                                                                               |
| meanWaitingTime   | s                    | The mean time all vehicles up to now and within the reported time step had to wait for being inserted; -1 if no vehicle has been inserted, yet                          |
| meanTravelTime    | s                    | The mean travel time of all vehicles that have left the simulation within the previous and the reported time;-1 if no vehicle has been removed from the simulation, yet |
| meanSpeed         | m/s                  | The mean speed over all vehicles in the network (which are not waiting at a `<stop>`).                                                                                          |
| meanSpeedRelative |                      | The mean speed over all vehicles in the network relative to the speed limit (which are not waiting at a `<stop>`).                                                              |
| duration          | ms                   | The computation time for that simulation step in milliseconds).                                                                                                         |
