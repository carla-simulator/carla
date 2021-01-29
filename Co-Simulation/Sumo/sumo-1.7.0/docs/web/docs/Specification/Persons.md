---
title: Specification/Persons
permalink: /Specification/Persons/
---

# Persons

A person moves through the net by walking or [using
vehicles](../Simulation/Public_Transport.md). The walking behavior
is customizable by selecting a [pedestrian
model](../Simulation/Pedestrians.md#pedestrian_models). A person
element has child elements defining stages of its plan. The stages are a
connected sequence of [ride](../Specification/Persons.md#rides),
[walk](../Specification/Persons.md#walks) and
[stop](../Specification/Persons.md#stops) elements as described
below. Each person must have at least one stage in its plan.

```
<person id="foo" depart="0">
    <walk edges="a b c"/>
    <ride from="c" to="d" lines="busline1"/>
    <ride .../>
    <walk .../>
    <stop .../>
</person>
```

| Attribute           | Type      | Range              | Default         | Remark      |
|---------------------|-----------|--------------------|-----------------|---------------------------|
| id                  | string    | valid XML ids      | -               |                          |
| depart              | float(s)  | ≥0 or 'triggered'  | -               | See [ride](Persons.md#rides) for an explanation of 'triggered'|
| departPos           | float(s)  | ≥0                 | -               | the distance along the edge that the person is created      |
| type                | string    | any declared vType | DEFAULT_PEDTYPE | the type should have vClass pedestrian              |
| width               | float (s) | ≥0                 | 0,48            | The person's width [m]        |
| lenght              | float (s) | ≥0                 | 0,21            | The person's netto-length (length) (in m)       |
| mingap              | float (s) | ≥0                 | 0,25            | Empty space after leader [m]                |
| maxSpeed            | float (s) | ≥0                 | 1,39            | The person's maximum velocity (in m/s)             |
| jmDriveAfterRedTime | float (s) | ≥0                 | -1              | This value causes persons to violate a red light if the duration of the red phase is lower than the given threshold. When set to 0, persons will always walk at yellow but will try to stop at red. If this behavior causes a person to walk so fast that stopping is not possible any more it will not attempt to stop. |

When specifying a `type`, the set of
attributes which are in effect during simulation depend on the selected
[pedestrian model](../Simulation/Pedestrians.md#pedestrian_models).
Attributes such as `width`, `length`, `imgFile` and `color` are always used for visualization.

# Repeated persons (personFlows)

To define multiple persons with the same plan, the element `<personFlow>` can be used.
It uses the same parameters and child elements as `<person>` except for the
departure time. The id of the created persons is
"personFlowId.runningNumber" and they are distributed either equally or
randomly in the given interval. The following additional parameters are
known:

| Attribute Name | Value Type     | Description                                                                                          |
| -------------- | -------------- | ---------------------------------------------------------------------------------------------------- |
| begin          | float(s)       | first person departure time                                                                          |
| end            | float(s)       | end of departure interval (if undefined, defaults to 24 hours)                                       |
| personsPerHour | float(\#/h)    | number of persons per hour, equally spaced (not together with period or probability)                 |
| period         | float(s)       | insert equally spaced persons at that period (not together with personsPerHour or probability)       |
| probability    | float(\[0,1\]) | probability for emitting a person each second (not together with personsPerHour or period), see also [Simulation/Randomness](../Simulation/Randomness.md#flows_with_a_random_number_of_vehicles) |
| number         | int(\#)        | total number of persons, equally spaced                                                              |

## Examples

```
   <personFlow id="p" begin="0" end="10" period="2">
       <walk from="beg" to="end"/>
   </personFlow>
```

```
   <personFlow id="person" begin="0" end="1" number="4" departPos="80">
       <walk from="2/3to1/3" to="1/3to0/3" arrivalPos="55"/>
       <ride from="1/3to0/3" to="0/4to1/4" lines="train0"/>
       <walk from="0/4to1/4" to="1/4to2/4" arrivalPos="45"/>
       <stop lane="1/4to2/4_0" duration="20" startPos="40" actType="singing"/>
       <ride from="1/4to2/4" to="3/4to4/4" lines="car0"/>
   </personFlow>
```

# Simulation input

The input for a person consists of a sequence of *stages* of the 3 types
given below.

!!! note
    Person plans can also be generated with [intermodal routing](../IntermodalRouting.md)

## Rides

Rides define the start and end point of a movement with a single mode of
transport (e.g. a car or a bus). They are child elements of plan
definitions.

| Attribute  | Type     | Range                              | Default | Remark                                            |
| ---------- | -------- | ---------------------------------- | ------- | ------------------------------------------------- |
| from       | string   | valid edge ids                     | \-      | id of the start edge                              |
| to         | string   | valid edge ids                     | \-      | id of the destination edge                        |
| busStop    | string   | valid bus stop ids                 | \-      | id of the destination stop                        |
| lines      | list     | valid line or vehicle ids or *ANY* | \-      | list of vehicle alternatives to take for the ride |
| arrivalPos | float(m) |                                    | \-1     | arrival position on the destination edge          |

The vehicle to use has to exist already (either public transport or some
existing passenger car) and the route to take is defined by the vehicle.
The person enters the vehicle if it stops on the 'from' edge and any of
the following conditions are met

- the 'line' attribute of the vehicle or the 'id' of the vehicle is
  given in the list defined by the 'lines' attribute of the ride OR
  the lines attribute contains 'ANY' and the vehicle stops at the
  destination 'busStop' of the ride (or at the destination edge if no destination busStop is defined).
- the vehicle has a triggered stop and the person is position within
  the range of `startpos,endPos` of the stop.
- the vehicle has a timed stop and the person is waiting within 10m of
  the vehicle position

The position of the person is either it's `departPos` or the arrival position of
the preceding plan element

A given bus stop may serve as a replacement for a destination edge and
arrival position. If an arrival position is given nevertheless it has to
be inside the range of the stop.

### Starting the simulation in a Vehicle
To start the simulation while riding in a vehicle, the following conditions must be met:
- the depart attribute must be set to 'triggered'
- the first stage of the plan must be a ride
- the lines attribute of the first ride must contain the ID of the vehicle to start in
- the starting vehicle must already be loaded in the input file

The person will then depart at the time of the vehicle departure and appear in the simulation as soon as the vehicle enters the network.

## Walks

Walks define a [pedestrian movement](../Simulation/Pedestrians.md).
They are child elements of plan definitions.

| Attribute  | Type       | Range              | Default | Remark                                                                          |
| ---------- | ---------- | ------------------ | ------- | ------------------------------------------------------------------------------- |
| route      | string     | valid route id     | \-      | the id of the route to walk                                                     |
| edges      | list       | valid edge ids     | \-      | id of the edges to walk                                                         |
| from       | string     | valid edge ids     | \-      | id of the start edge                                                            |
| to         | string     | valid edge ids     | \-      | id of the destination edge                                                      |
| busStop    | string     | valid bus stop ids | \-      | id of the destination stop                                                      |
| duration   | float(s)   | \>0                | \-      | (deprecated, determined by the person type and the pedestrian dynamics)         |
| speed      | float(m/s) | \>0                | \-      | (deprecated, determined by the person type and the pedestrian dynamics)         |
| departPos  | float(m)   |                    | 0       | initial position on the starting edge (deprecated, determined by the departPos of the person or the arrival pos of the previous step) |
| arrivalPos | float(m)   |                    | \-1     | arrival position on the destination edge                                        |

You can define either a `route`-id, or a list of `edges` to travel or a `from` and a `to` edge.
In the first and second case the route edges are traveled in the listed
order. They do not need to be joined in the net. If travelling between
stops on the same edge then only include the edge once. In the latter
case a shortest path calculation is performed and it is an error if
there is no path connecting `from` and `to`.

When given as router input input using the attributes `from` and `to` will be
transformed into a walk using the attribute `edges` by routing along edges
permissible for pedestrian (i.e. sidewalks).

A given bus stop may serve as a replacement for a destination edge and
arrival position. If an arrival position is given nevertheless, it has
to be inside the range of the stop.

## Stops

Stops define a delay until the next element of a plan is started. They
can be used to model activities such as working or shopping. Stops for
persons follow the specification at
[Specification\#Stops](../Specification.md#stops). However, only
the attributes `lane`, `duration` and `until` are evaluated. Using these attributes it is
possible to model activities with a fixed duration as well as those with
a fixed end time. If a person needs to be transferred between two
positions without delay, it is possible to use two stops in conjunction.

# Simulation behavior

A person is starting her life at her depart time on the source (resp.
first) edge of the first walk, ride or stop. She tries to start the next
step of her plan.

## Riding

The person checks whether a vehicle with a line from the given list is
stopping at the given edge. If such a vehicle exists and the person is
positioned between the start and end position of the vehicle's stop, the
person will enter the vehicle and start its ride. If such a vehicle
exists but the person is not positioned between the start and end
position of the vehicle's stop, the person will still enter if the
vehicle is triggered by the a person and the distance between person and
vehicle is at most 10 metres. It does not check whether the vehicle has
the aspired destination on the current route. The first time the vehicle
stops (on a well defined stop) at the destination edge, the ride is
finished and the person proceeds with the next step in the plan.

## Walking

The walking behavior of a person depends on the selected [pedestrian
model](../Simulation/Pedestrians.md#pedestrian_models). Generally,
the person follows the given sequence of edges with a speed bounded by
the `maxSpeed` attribute of the persons type. It starts either at the position
from the previous stage of its plan or at the specified `departPos` if no previous
stage exists. The walk concludes at the specified `arrivalPos` which defaults to the
end of the final edge. Both position attributes support the special
values `max` and `random` which work as described [for
vehicles](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#s_vehicles_depart_and_arrival_parameter).

## Stopping

The person stops for the maximum of *currentTime+duration* and *until*.

# Router input
The following definitions can be processed with [duarouter](../duarouter.md) and [sumo](../sumo.md).

## PersonTrips

A personTrip defines the start and end point of a movement with optional
changes in mode. They are child elements to the person. In order to
process a personTrip with mode *public*, the [public transport
network](../IntermodalRouting.md) has to be defined as well.
Currently bicycle and/or car can only be the first mode. It is not
possible to switch to a car or bicycle after a different mode.

If the computed plan starts with a car or bicycle, a vehicle for use by the person will be automatically generated and set to depart="triggered". A car will be named 'PERSON_ID_0' and bicycle will be named 'PERSON_ID_b0' where 'PERSON_ID' is the id of the person. If a vehicle with this id already exists in the simulation, this vehicle will be used instead (and the user has to ensure that it starts at an appropriate location).

| Attribute  | Type     | Range                                         | Default | Remark                                                        |
| ---------- | -------- | --------------------------------------------- | ------- | ------------------------------------------------------------- |
| from       | string   | valid edge ids                                | \-      | id of the start edge                                          |
| to         | string   | valid edge ids                                | \-      | id of the destination edge                                    |
| via        | string   | valid edge ids                                | \-      | ids of the intermediate edges (not implemented yet)           |
| busStop    | string   | valid bus stop ids                            | \-      | id of the destination stop                                    |
| vTypes     | list     | valid vType ids                               | \-      | list of possible vehicle types to take                        |
| modes      | list     | any combination of "public", "car", "bicycle", ["taxi"](../Simulation/Taxi.md) | \-      | list of possible traffic modes (separated by ' '). Walking is always possible regardless of this value.     |
| departPos  | float(m) |                                               | 0       | initial position on the starting edge (deprecated, determined by the departPos of the person or the arrival pos of the previous step) |
| arrivalPos | float(m) |                                               | \-eps   | arrival position on the destination edge                      |

!!! note
    It is an error for subsequent trips to be unconnected.

!!! note
    If no itinerary for performing the trip is found and the option **--ignore-route-errors** is set, the trip will be transformed into a walk which consists of the start and arrival edge. The person will teleport to complete the walk.
    
!!! when attribute vTypes is used, the person may start with any of the given vehicle types at the from-edge. Including 'car' in modes is equivalent to vTypes="DEFAULT_VEHTYPE". Including 'bicycle' in modes is equivalent to vTypes="DEFAULT_BIKETYPE". The vehicles will be automatically generated when used.

# Example

The following is an example for a person who walks to a train station,
rides the train, alights and walks for a bit, then stops for an activity
and finally gets into a car and drives away.

```
<routes>
    <person id="person0" depart="0">
        <walk from="2/3to1/3" to="1/3to0/3" departPos="80" arrivalPos="55"/>
        <ride from="1/3to0/3" to="0/4to1/4" lines="train0"/>
        <walk from="0/4to1/4" to="1/4to2/4" arrivalPos="30"/>
        <stop lane="1/4to2/4_0" duration="20" startPos="40" actType="singing"/>
        <ride from="1/4to2/4" to="3/4to4/4" lines="car0"/>
    </person>

    <vehicle id="train0" depart="50">
        <route edges="1/4to1/3 1/3to0/3 0/3to0/4 0/4to1/4 1/4to1/3"/>
        <stop busStop="busStop0" until="120" duration="10"/>
        <stop busStop="busStop1" until="180" duration="10"/>
    </vehicle>

    <vehicle id="car0" depart="triggered">
        <route edges="1/4to2/4 2/4to3/4 3/4to4/4" departPos="30"/>
        <stop lane="1/4to2/4_0" duration="20" startPos="40" endPos="60"/>
    </vehicle>

</routes>
```

You can omit the from attribute in the second and following movements
and you can use bus stops as destinations. For trains the busStops
should have access lanes. The movement of person0 in the example above
could also be written as

```
<person id="person0" depart="0">
    <walk from="2/3to1/3" busStop="busStop0" departPos="80" arrivalPos="55"/>
    <ride busStop="busStop1" lines="train0"/>
    <walk to="1/4to2/4" arrivalPos="30"/>
    <stop lane="1/4to2/4_0" duration="20" startPos="40" actType="singing"/>
    <ride from="1/4to2/4" to="3/4to4/4" lines="car0"/>
</person>
```

# Visualization

Persons are rendered in the GUI with the configured detail-level. When
assigning a type with attribute *imgFile*, the person may be rendered
with an image. By default, the image will not be rotated (suitable for
abstract icons). However, when setting the types *guiShape*="pedestrian"
the image will be rotated according to the persons location and stage.

# Person Output

Most of the [Simulation outputs](../Simulation/Output.md) are
tailored for vehicles. Only a small number of output formats support
persons:

- [tripinfo-output](../Simulation/Output/TripInfo.md)
- [vehroute-output](../Simulation/Output/VehRoutes.md)
- [fcd-output](../Simulation/Output/FCDOutput.md)
- [netstate-dump](../Simulation/Output/RawDump.md)
- [aggregated simulation
  statistics](../Simulation/Output.md#aggregated_traffic_measures)

# Planned features

The following features are not yet implemented.

- state saving and loading, see #2792
- [Simulation routing for
  persons](../Demand/Automatic_Routing.md)
  (person-device.rerouting)
