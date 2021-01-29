---
title: Contributed/SUMOPy/GUI/Demand Modelling
permalink: /Contributed/SUMOPy/GUI/Demand_Modelling/
---

In the framework of SUMO, demand generation means essentially the
generation of traffic participants (persons and/or different types of
vehicles ) and the generation of a route for each traffic participant.
It is recommended to start with demand generation only after the network
has been edited. In any case it is good practice to save a scenario with
the network only (without demand info).

All available vehicle types\[1\] can be browsed and modified under:

`scenario.demand.vtypes`

Note that each vehicle type belongs to a vehicle class, also called
modes.

The different demand generation methods, as described below, will
generate trips and routes, which can be browsed in

`scenario.demand.trips` and `scenario.demand.trips.routes`

The currently implemented demand generation methods are:

- [Zone to zone demand flows](../../../Contributed/SUMOPy/Demand/Zone_To_Zone.md)
- [Turn flows](../../../Contributed/SUMOPy/Demand/Turn_Flows.md)
- Virtual Population

The different demand generation methods can be used in combination: For
example trips generated with method *Zone to zone demand flows* can be
simulated together with the demand generated with the *Virtual
Population* method.

1.  Of course in a traffic scenario we have usually more than one
    participant of each type