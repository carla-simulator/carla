---
title: Simulation/Turnarounds
permalink: /Simulation/Turnarounds/
---

# Introduction
Vehicles that wish to perform a turn-around maneuver can be a source of jams and deadlocks in the simulation.
A typical reason why vehicles need to perform a turn-around is because they have to depart and arrive in a particular direction on their first and last edge and this does not fit their preferred direction of travel. 

In some way, this behavior is unrealistic because in reality, vehicles that depart from home can often select their initial directly of travel freely without the need for a turn-around. In other situations such as parallel parking on a divided road, turn-arounds are often necessary. 

Consequently, there are two avenues for avoiding the negative effects of turn-arounds described below.

# Avoiding the need for turnarounds

## Trips between junctions
Instead of defining trips between edges, [trips can be defined between junctions](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#routing_between_junctions).
When this is done, vehicles will always depart on an edge that goes into the
correct direction because on initial routing they are free to pick the edge
that best suites their direction of travel.

Besides defining trips manually, trips between junctions can also be achieved
with

- [Random Trips option **--junction-taz**](../Tools/Trip.md#randomtripspy)
- [duarouter options **--write-trips.junctions**](../duarouter.md)

!!! caution
    When loading trips with `fromJunction` or `toJunction` attributes with [sumo](../sumo.md) or [duarouter](../duarouter.md), the option **--junction-taz** must be set or an equivalent TAZ-file must be loaded.

## Trips between TAZ
Instead of departing as junctions (which internally use specialised TAZ), the
user may define traffic larger assignment zones (TAZ / districts) as origins and
destinations of travel. This will also cause vehicles to pick a suitable
direction. If the main goal of these zones is to avoid turnarounds, then the
tool [generateBidiDistricts.py](../Tools/District.md#generatebididistrictspy) may
be used. 

## Modifying existing routes
If routes between edges already exist, the [duarouter option **--remove-loops**](../duarouter.md) can be used to shorten the roads in a way that avoids turn-arounds.

# Controlling where turnarounds may happen
By removing turnaround connections from a network (i.e. with
[netedit](../netedit.md), turnarounds and unsuitable locations can be prevented.

Often it is easier to control the generation of turnaround connections when
importing the network. For this purpose [netconvert](../netconvert.md) provides a number of options.

- **--no-turnarounds** : Disables build turnarounds. This may cause vehicles to
  take long detoures so they can achieve a turnaround by turning right or left repeatedly.
- **--no-turnarounds.tls** : Disables building turnarounds at tls-controlled junctions"
- **--no-turnarounds.geometry** : Disables building turnarounds at geometry-like junctions. This option is active by default to prevent vehicles from turning around on narrow roads
- **--no-turnarounds.except-deadend** : Disables building turnarounds except at dead end junctions. This option ensures that vehicles can exit from a cul-de-sac
- **--no-turnarounds.except-turnlane** : Disables building turnarounds except at at junctions with a dedicated turning lane. This option offers a good compromise between preventing turnarounds related jams and avoiding long detoures. In the presence of a dedicated turning lane, a vehicle that has to wait for turning around is less disruptive to the rest of the traffic.
- **--no-turnarounds.fringe** : Disables building turnarounds at fringe junctions. The purpose for this option is to prevent turn-arounds at the boundary of the network where edges are cut of (like a cul-de-sac) but in reality they would actually continue beyond the boundaries of the scenario. In these locations turn-arounds typically should not happen. In order to take effect, this option requires that boundary junctions have the attribute `fringe="outer"` set. This can be accomplished by setting the option **--fringe.guess**.