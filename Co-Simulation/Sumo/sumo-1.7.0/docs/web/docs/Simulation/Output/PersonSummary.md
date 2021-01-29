---
title: Simulation/Output/PersonSummary
permalink: /Simulation/Output/PersonSummary/
---

This output contains the simulation-wide number of person that are
loaded, inserted, running, waiting to be inserted, have reached their
destination and how many persons are travelling in which modes.

## Instantiating within the Simulation

You can force the simulation to generate this output using **--person-summary-output** {{DT_FILE}}.

## Generated Output

The information containing all those values is computed for each time
step and the output file looks like following:

```
<summary>
    <step time="<SIMULATION_TIME>" \
            loaded="<LOADED_PERSON_NUMBER>" \
            inserted="<INSERTED_PERSON_NUMBER>" \
            walking="<RUNNING_PERSON_NUMBER>" \
            waiting="<NUMBER_OF_PERSONS_WAITING_FOR_INSERTION>" \
            ended="<ENDED_PERSON_NUMBER>" \

    ... further time steps ...

</summary>
```

Please note, that in contrary to the example above, for each time step,
all those values are reported in one line. A description of the values
is given in the table below.

| Name              | Type                 | Description                                                                                                                                                             |
| ----------------- | -------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| time              | (simulation) seconds | The time step described by the entry                                                                                                                                    |
| loaded            | \#                   | Number of persons that were loaded into the simulation so far (including reported time step)                                                                           |
| inserted          | \#                   | Number of persons inserted so far (including reported time step)                                                                                                       |
| walking           | \#                   | Number of persons that are walking within the reported time step                                                                                                      |
| waitingForRide    | \#                   | Number of persons which are waiting for a vehicle ride within the reported step
| riding            | \#                   | Number of persons which are riding a vehicle ride within the reported step
| stopping          | \#                   | Number of persons which are stopped for a specified duration within the reported step
| jammed            | \#                   | The number of persons that were jammed since the start of the simulation
| ended             | \#                   | Number of persons that have reached their destination so far or that were otherwise removed from the simulation (including reported time step)                         |
| arrived           | \#                   | Number of persons that have reached their destination so far                                                                                                           |
| duration          | ms                   | The computation time for that simulation step in milliseconds).                                                                                                         |