---
title: Simulation/Output/RawDump
permalink: /Simulation/Output/RawDump/
---

The network dump (also called *netstate output* or *netstate dump*) was
the first output capability that was implemented in SUMO. It contains
detailed information for each edge, each vehicle and each simulation
step. Due to the high level of detail it is very flexible but often
requires additional programming to parse and filter the information that
is actually desired. The
[Simulation/Output/FCDOutput](../../Simulation/Output/FCDOutput.md)
has a similar level of detail but does not group vehicles based on their
edge.

## Instantiating within the Simulation

To force SUMO to build a file that contains the network dump, extend
your command line (or configuration) parameter by **--netstate-dump** {{DT_FILE}} (or **--ndump** {{DT_FILE}} or **--netstate** {{DT_FILE}}).

[Alternatively, add the option to the configuration
file](../../Basics/Using_the_Command_Line_Applications.md#configuration_files),
e.g.:

```
<output>
   <netstate-dump value="<FILE>" />
</output>
```

{{DT_FILE}} is the name of the file the output will be written to. Any other file
with this name will be overwritten, the destination folder must exist.

## Generated Output

The network dump is a xml-file containing for each time step every edge
of the network with every lane of this edge with all vehicles on this
lane. For each vehicle, its name, speed and position on its respective
lane are written. A raw dump-file looks like this:

```
<netstate>
   <timestep time="<TIME_STEP>">
      <edge id="<EDGE_ID>">
         <lane id="<LANE_ID>">
            <vehicle id="<VEHICLE_ID>" pos="<VEH_POSITION>" speed="<VEH_SPEED>"/>

            ... more vehicles if any on this lane ...

         </lane>

         ... more lanes if the edge possesses more ...

         ... optional persons and containers if currently active on that edge
         <person id="<ID>" pos="<OFFSET_FROM_EDGE_BEGIN>" speed="<SPEED>"/>
         ...
         <container id="<ID>" pos="<OFFSET_FROM_EDGE_BEGIN>" speed="<SPEED>"/>
         ...

      </edge>

      ... more edges ....

   </timestep>

... the next timestep ...

</netstate>
```

!!! note
      If persons or containers are transported within a vehicle, they are written as child elements of that vehicle rather than as child elements of the edge.

The meanings of the written values are given in the following table.

| Name            | Type                 | Description                                                                                                                      |
| --------------- | -------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| time            | (simulation) seconds | The time step described by the values within this timestep-element                                                               |
| id              | id                   | The id of the edge/lane/vehicle                                                                                                  |
| pos             | m                    | The position of the vehicle at the lane within the described time step (distance of the front bumper from the start of the lane) |
| speed           | m/s                  | The speed of the vehicle within the described time step                                                                          |
| posLat          | m                    | Offset from the center of the lane (only when using the [sublane model](../../Simulation/SublaneModel.md))                     |
| personNumber    | \#                   | number of persons in the vehicle (not in [MESO](../../Simulation/Meso.md))                                                                |
| containerNumber | \#                   | number of containers in the vehicle (not in [MESO](../../Simulation/Meso.md))                                                             |
| stage           | string               | description of the current [stage](../../Specification/Persons.md#simulation_input) (only for persons and containers)          |

## Additional Options

The option **--netstate-dump.precision** {{DT_INT}} controls the number of significant digits when writing
positions and speeds to the output file. The option **--netstate-dump.empty-edges** {{DT_BOOL}} controls whether
edges without vehicles are included in the output.

## Notes

As you may imagine, this output is very verbose. Its main disadvantage
is the size of the generated file. It's very easy to generate files that
are several GB large within some minutes. It is of course possible to
write some nice tools that parse the file (using a SAX-parser) and
generate some meaningful information, but we do not know anyone who has
made this. Another problem is that the simulation's execution speed of
course breaks down when such an amount of data must be written.

Normally, all lanes are written, even if there is no vehicle on them.
You can change this behavior using the boolean switch **--netstate-dump.empty-edges**. In this case,
only those edges and lanes will be written that contain vehicles.