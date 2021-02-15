---
title: Simulation/Randomness
permalink: /Simulation/Randomness/
---

Stochasticity is an important aspect of reproducing reality in a
simulation scenario. There are multiple ways of adding stochasticity to
a simulation. They are described below.

# Random number generation (RNG)

Sumo implements the [Mersenne
Twister](http://en.wikipedia.org/wiki/Mersenne_twister) algorithm for
generating random numbers. This random number generator (RNG) is
initialized with a seed value which defaults to the (arbitrary) value
**23423**. This setup makes all applications deterministic by default as
the sequence of random numbers is fixed for a given seed. The seed may
be changed using the option **--seed** {{DT_INT}}. When using the option **--random** the seed will be
chosen based on the current system time resulting in truly random
behavior.

The simulation uses multiple RNG instances to decouple different
simulation aspects

- randomness when loading vehicles (typeDistribution, speed
  deviation,...)
- probabilistic flows
- vehicle driving dynamics
- vehicle devices

The decoupling is done to ensure that loading vehicles does not affect
simulation behavior of earlier vehicles. All RNGs use the same seed.

# Route Distributions

Vehicles can be added to the simulation with a fixed route (`<vehicle>`) or with an origin-destination pair (`<trip>`).
A third alternative is to specify a set of routes (`<routeDistribution>`) and let the vehicle draw a random route from such a distribution. For details, see [route distributions](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#route_distributions).

# Vehicle Type Distributions

A simple way of of modelling a heterogeneous vehicle fleet works by defining a `<vTypeDistribution>` and let each vehicle pick it's type randomly from this distribution. For details, see [vehicle type distributions](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_type_distributions).

# Speed distribution

By default, vehicles in SUMO adhere to the maximum speed defined for the
lane they are driving on (if the maximumSpeed of their vehicle type
allows it). This behavior can be modified using the `<vType>`-attribute `speedFactor` which
makes vehicles drive with that factor of the current speed limit. The
attribute also allows the specification of the parameters of a normal
distribution with optional cutoffs. The random value is selected once
for each vehicle at the time of its creation. Using a speed deviation is
the recommended way for getting a heterogenous mix of vehicle speeds.
By default, a speed distribution with a standard deviation of 10% is active.
For details, see [speed distribution](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#speed_distributions)

# Car-Following

The default [car-following model
*Krauss*](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#car-following_models)
supports stochastic driving behavior through the `vType`-attribute `sigma` (default
0.5). When this value is non-zero drivers will randomly vary their speed
based on the RNG described above. Other car-following models also use
this attribute.

# Departure times

The departure times of all vehicles may be varied randomly by using the
option **--random-depart-offset** {{DT_TIME}}. When this option is used each vehicle receives a random offset
to its departure time, equidistributed on \[0, {{DT_TIME}}\].

# Flows with a fixed number of vehicles

The [duarouter](../duarouter.md), [dfrouter](../dfrouter.md)
and [jtrrouter](../jtrrouter.md) applications support the option **--randomize-flows**.
When this option is used, each vehicle defined by a `<flow>`-element will be
given a random departure time which is equidistributed within the time
interval of the flow. (By default vehicles of a flow are spaced equally
in time).

# Flows with a random number of vehicles

Both [duarouter](../duarouter.md) and [sumo](../sumo.md)
support loading of `<flow>` elements with attribute `probability`. When this attribute is
used (instead of `vehsPerHour,number`, or `period`), a vehicle will be emitted randomly with the
given probability each second. This results in a [binomially
distributed](https://en.wikipedia.org/wiki/Binomial_distribution) flow
(which approximates a [Poisson
Distribution](https://en.wikipedia.org/wiki/Poisson_distribution) for
small probabilities). When modeling such a flow on a multi-lane road it
is recommended to define a `<flow>` for each individual lane.

When simulating with subsecond time resolution, the random decision for insertion is take in every simulation step and the probability for insertion is scaled with step-length so that the per-second probability of insertion is independent of the step-length. 
!!! note
    The effective flow may be higher at lower step-length because the discretization error is reduced (vehicles usually cannot be inserted in subsequent seconds due to safety constraints and insertion in every other second does not achieve maximum flow).

# Departure and arrival attributes

The `<flow>`, `<trip>` and `<vehicle>` elements support the value "random" for their attributes `departLane`, `departPos`,
`departSpeed` and `arrivalPos`. The value will be chosen randomly on every insertion try (for the
departure attributes) or whenever there is a need to revalidate the
arrival value (i.e. after rerouting).

# Further sources of randomness

- The tool [randomTrips.py](../Tools/Trip.md#randomtripspy)
  allows generating traffic between random edges. It also supports
  randomizing arrival rates.
- [od2trips](../od2trips.md) adds randomness when drawing
  individual trips from an O/D-Matrix
- [duarouter](../duarouter.md) adds randomness when performing
  [Demand/Dynamic_User_Assignment](../Demand/Dynamic_User_Assignment.md)
- [duarouter](../duarouter.md) can randomly disturb the fastest-paths by setting option **--weights.random-factor**
- [Simulation routing can be
  randomized](../Demand/Automatic_Routing.md#randomness) to
  ensure usage of alternative routes.