---
title: Specification/Duarouter
permalink: /Specification/Duarouter/
---

# Inputs

## Network

A good old sumo network maybe with traffic assignment zones (aka taz or
districts). They may also be given in a separate file.

## Demand

In principle the duarouter should understand every route file sumo
understands. This includes correct parsing of routes, vehicle types,
vehicles, flows, trips, various distributions, stops and persons. Per
default it tries to find shortest paths between any two consecutive
edges given in a (trip, flow or route) definition. In the "iterative"
mode it also tries to add new routes between start and destination.

## Partial routes, stops etc.

It is possible to define a flow with a nested route or with from/to/via
and/or with nested stops. Additionally it may have fromTaz/toTaz
attributes. This section describes how these settings (should) interact.
They should in principle transfer to vehicle/trip definitions and to the
usage with ad hoc (one shot) routing as well.

### TAZ

- "from" takes precedence over "fromTaz" (and the same for "to(Taz)")
  unless "--with-taz" is used
- if "with-taz" is used, it is an error if no taz information is
  present
- it may be the case that "from" is used together with "toTaz" or vice
  versa

### nested routes together with from/to

- it is an error, if a nested route definition does not fit the
  information in the from/to/via or the fromTaz/toTaz attributes
- the nested route has precedence over a route referenced by id but a
  warning is emitted if both are given

### nested stops

- stops nested in a flow are merged into the list of stops of the
  route using the stop attributes
- the stop edges are handled just like "via" edges
- if "via" is present as well and is not a superset of the stop edges,
  it is an error
- if the stops do not fit the route, it is an error as well

## Rerouting

There are some special cases when using this concept with the
in-simulation-routing because the vehicle may be already on the way when
a rerouting request occurs. If we are still before vehicle insertion
everything should behave as with the duarouter at least for the first
route search (it is probably hard to implement at the moment to do
rerouting before insertion and keep all information about vias). After
insertion the vehicle already has a real start edge so the routing
source will always be a real edge but the destination may still vary
with the taz info as above. Stops should still be taken into account (if
they are not already done) but vias are ignored for the moment (until we
implement the more general concept of "waypoints" which define
intermediate positions / speeds on the route which are no stops).

# Output

duarouter always generates a route file and a route alternatives file
(with route distributions).

- it always contains vehicles with nested routes (and optionally stops
which are vehicle child elements and not route child elements)
- "from" and "to" are not retained in the output, "fromTaz", "toTaz"
and "via" are
  - alternative: keep "via" only if necessary (if it cannot be
    reconstructed from the stops)

## vehroutes

Should basically generate a usable route file as well.