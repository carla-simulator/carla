---
title: Simulation/VehicleInsertion
permalink: /Simulation/VehicleInsertion/
---

# Loading

Vehicles are either loaded [from XML
inputs](../index.md#demand_modelling) or [added
at runtime via TraCI](../TraCI.md). When loading from XML, not all
vehicles are loaded at once. Instead, vehicles are loaded in chunks, the
size of which can be configured with the option **--route-steps**. This is done to
conserve memory when performing long simulations.

Once vehicles are loaded they can be accessed and manipulated via TraCI
(to some degree). Using the functions 'move to' or 'move to XY' they can
even be forcefully inserted into the network.

# Insertion (Departure)

In order for a vehicle to be inserted into the road network, some
necessary constraints must be fulfilled:

- The vehicle (from its rear to its front position + *minGap*) must
  not intersect with other vehicles (including their *minGap*
- The vehicle must be at a safe distance to any leader vehicles
  according to its *carFollowModel*
- Any follower vehicles must be at a safe distance according to their
  *carFollowModel*
- The vehicle must be able to brake for any upcoming non-prioritized
  intersections along its route
- The vehicle must be able to brake for any scheduled stops along its
  route

If a vehicle cannot be inserted due to any of the above reasons it's
insertion is delayed (see below). This *departDelay* is recorded in the
[tripinfo-output](../Simulation/Output/TripInfo.md) and can also be
inspected in [sumo-gui](../sumo-gui.md) via the vehicle parameter
dialog.

The precise nature of insertion in regard to position, speed and depart
time depend on [many
parameters](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicles_and_routes)
as well as on the current state of the road network.

## Delayed Departure

If there is insufficient space for inserting the vehicle at it's
designated departure time, that vehicle is put into an insertion queue
and insertion is repeatedly attempted in subsequent simulation steps. If
the option **--max-depart-delay** {{DT_TIME}} is used, vehicles are discarded if they could not be
inserted after the specified {{DT_TIME}} in seconds.

There are two methods for inserting vehicles into the simulation:

1.  try to insert every vehicle in every simulation step
2.  for every edge with vehicles to insert, abort trying to insert
    vehicles after one of the vehicles could not be inserted

In an uncongested networks these methods behave similar but in a
congested network with lots of vehicles which cannot be inserted variant
2) is much faster. In older version of sumo 1) was the default and one
could switch to 2) using the option **--sloppy-insert** Since version 0.18.0, variant 2) is
the default and one may switch to 1) using the option **--eager-insert**.

## Investigating insertion delay

Using [sumo-gui](../sumo-gui.md) several options exist for showing
insertion delay:

- Color vehicles *by insertion delay*
- Color Streets *by insertion backlog*
- Opening Vehicle statistics lists the number of *insertion-backloged
  vehicles* for the whole network.
- The parameter Dialog for individual vehicles lists *desired depart*
  and *depart delay*
  
## Effect of simulation step-length
Insertion attemps can only happen in every simulation step. This may cause artifacts in insertion spacing because at the default step-length of 1s is (usually) too short for vehicles to be inserted in successive steps on the same depart location.
By default, the next attempt happens 2 seconds after the first vehicle has departed and this gap may be much larger then mandated by the carFollowModel. There are multiple ways to avoid this effect:

- the step-length can be reduced
- the insertion location must be modified by any of the following methods
  - specifying a specific departPos
  - using `departPos="last"`
  - setting option **--extrapolate-departpos**

## Forcing Insertion / Avoiding depart delay

- Make sure that all lanes are used for insertion i.e. by setting `departLane="random"` (or `free` or `best`)
- Insert with `departSpeed="max"`: vehicle speeds at insertion will be adjusted to the
  maximum safe speed that allows insertion at the specified time to
  succeed
- Insert with `departPos="last"`: vehicle position will be adjusted to the maximum
  position behind the leader vehicle that allows driving with the
  given speed (or maximum allowed speed when using `departSpeed="max"`)
- insert with a fixed departSpeed (numerical value) and set option **--ignore-route-errors**.
  Vehicles will be inserted with unsafe speeds at the specified time
  if emergency-braking would still allow collision-free dynamics
- use
  [traci.vehicle.moveTo](../TraCI/Change_Vehicle_State.md#move_to_0x5c)
  or
  [traci.vehicle.moveToXY](../TraCI/Change_Vehicle_State.md#move_to_xy_0xb4)
  to force the vehicle into the network at the specified time and
  place.

## Global options that affect Departure

- **--random-depart-offset** {{DT_TIME}}: randomly delays departure time for all vehicles
- **--max-num-vehicles** {{DT_INT}}: limits the total amount of vehicles that may exist in the network.
  Setting this may cause delayed insertion
- **--max-depart-delay** {{DT_TIME}}: removes vehicles from the insertion queue after a set amount of
  time
- **--eager-insert** {{DT_BOOL}}: tries to insert all vehicles that are insertion-delayed on each
  edge. By default, insertion on an edge stops after the first failure
  in each time step
- **--step-length** {{DT_FLOAT}}: the frequency of insertion attempts can be raised and also the number of possible headways (for a fixed departPos) is increased by setting a lower step-length.
- **--extrapolate-departpos** {{DT_BOOL}}: Moves the default depart position of a vehicle downstream if it's departure time is a fraction of the simulation step-length. The offset in depart position is proportional to the depart delay from step-length discretization.

# Miscellaneous

- For tips on how to achieve high insertion flows see [the
  FAQ](../FAQ.md#how_do_i_get_high_flowsvehicle_densities)
- Vehicles can also be inserted through the use of
  [calibrators](../Simulation/Calibrator.md)
- Vehicle rerouting may be triggered even before departure [when using
  device.rerouting](../Demand/Automatic_Routing.md)
- The departure edge can be determined at run-time when using [Traffic
  Assignment Zones
  (TAZ)](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#traffic_assignement_zones_taz)
