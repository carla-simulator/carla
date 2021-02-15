---
title: Simulation/Taxi
permalink: /Simulation/Taxi
---
# Introduction
Since version 1.5.0 SUMO supports simulation of demand responsive transport (DRT)
via the taxi device. This allows a fleet of taxis to service customer requests
based on a configurable dispatch algorithm.

!!! note
    While the taxi capabilities are under development, their status can be checked via Issue #6418.

# Equipping vehicles
A vehicle can be equipped with an Taxi device to make it part of the taxi fleet.
To attach a Taxi device to a vehicle, the [standard device-equipment
procedures](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#devices) can be applied using `<device name>=taxi`.

For instance, a single vehicle can configured as taxi as in the following minimal example

```
    <vehicle id="v0" route="route0" depart="0" line="taxi">
        <param key="has.taxi.device" value="true"/>
        <stop lane="C0D0_1" triggered="person"/>
    </vehicle>
```

Note, that the stop definition is used to keep the taxi waiting in the
simulation until the first dispatch is received.

# Taxi requests

## Direct ride hailing
A person can be defined as taxi customer with the following definition:

```
    <person id="p0" depart="0.00">
        <ride from="B2C2" to="A0B0" lines="taxi"/>
    </person>
```

## Intermodal Routing
A person can also use a taxi by including it as a [personTrip](../Specification/Persons.md#persontrips) mode:

```
    <person id="p0" depart="0.00">
        <personTrip from="B2C2" to="A0B0" modes="taxi"/>
    </person>
```

## Groups of Persons
Multiple persons can travel together as a group using attribute `group` (if the taxi has sufficient capacity):

    <person id="p0" depart="0.00">
        <ride from="B2C2" to="A0B0" lines="taxi" group="g0"/>
    </person>
    <person id="p1" depart="0.00">
        <ride from="B2C2" to="A0B0" lines="taxi" group="g0"/>
    </person>

# Dispatch
The dispatch algorithm assigns taxis to waiting customers. The algorithm is
selected using option **--device.taxi.dispatch-algorithm ALGONAME**. The following
algorithms are available

- greedy: Assigns taxis to customers in the order of their reservations. The
  closest taxi (in terms of travel time) is assigned. If the reservation date is
  too far in the future, the customer is postponed.

- greedyClosest: For each available taxi, the closest customer (in terms of
  travel time) is assigned. If the reservation date is too far in the future,
  the customer is postponed.

- greedyShared: like 'greedy' but tries to pick up another passenger while delivering the first passenger to it's destination. The algorithm supports parameters **absLossThreshold** and **relLossThreshold** to configure acceptable detours.

- routeExtension: like greedy but can pick up any passenger along the route and also extend the original route (within personCapacity limit).

- traci: Dispatch is defered to [traci control](#traci). The algorithm only keeps track of pending reservations

!!! note
    User-contributed dispatch algorithms are welcome.

# Taxi Behavior
To make a taxi stay in the simulation it receives a stop with attribute ```triggered="person"``` after servicing a customer.
It is recommended to also define such a stop in the xml input so that the taxi can wait for the first customer.

By default, taxis will remain in the simulation until all persons have left. To make them leave the simulation at an earlier time, the end time can be defined using a generic parameter in their ```vType``` or ```vehicle```-definition:

```
    <vType id="taxi" vClass="taxi">
        <param key="has.taxi.device" value="true"/>
        <param key="device.taxi.end" value="3600"/>
    </vType>
```

# TraCI
To couple an external dispatch algorithm to SUMO, the following [TraCI](../TraCI.md) functions are provided:

!!! note
    To make use of these functions, the option **--device.taxi.dispatch-algorithm traci** must be set

- traci.person.getTaxiReservations(onlyNew)
- traci.vehicle.getTaxiFleet(taxiState)
- traci.vehicle.dispatchTaxi(vehID, reservations)

This set of API calls can be used to simplify writing custom dispatch algorithms by letting sumo:
- manage existing reservations
- manage the taxi fleet
- dispatch a taxi to service one or more reservations by giving a list of reservation ids (vehicle routing and stopping is then automatic).

# Outputs

The Taxi device generates output within a tripinfo-output file in the following
form:

```
    <tripinfo id="trip_0" ... >
        <taxi customers="5" occupiedDistance="6748.77" occupiedTime="595.00"/>
    </tripinfo>
```
