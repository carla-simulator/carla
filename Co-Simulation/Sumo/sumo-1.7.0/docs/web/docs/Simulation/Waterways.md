---
title: Simulation/Waterways
permalink: /Simulation/Waterways/
---

# Waterway Simulation

This page describes simulations of (inland) waterways in SUMO.

!!! caution
    The simulation of waterways is a developing subject and still carries some difficulties. These are discussed below.

# Approaches to ship modelling

Currently, no exclusive movement model for ships is implemented. Instead
the existing models for vehicle movement need to be re-purposed. By
setting `guiShape="ship"`, vehicles are drawn in a more appropriate shape.

## Problems and workarounds

- Due to the one-way nature of edges in sumo, ships cannot overtake
  each other by using the whole width of the waterway. To allow
  overtaking, multiple lanes have to be defined.
- Since the right-of-way rules for ships are more complex than those
  for road vehicles, waterway intersections are set to `uncontrolled` by default.

# Building a network for waterway simulation

Waterways can be imported from
[OSM](../Networks/Import/OpenStreetMap.md) by adding the type map
[osmNetconvertShips.typ.xml]({{Source}}data/typemap/osmNetconvertShips.typ.xml).
They can also be explicitly specified by setting `allow="ship"`.