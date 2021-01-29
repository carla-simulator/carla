---
title: Simulation/Bicycles
permalink: /Simulation/Bicycles/
---

# Bicycle Simulation

This page describes simulations of bicycles in SUMO. To build an
intermodal simulation scenario with bicycles, additional steps have to
be take in comparison to a plain vehicular simulation.

!!! caution
    The simulation of bicycles is a developing subject and still carries some difficulties. These are discussed below.

# Approaches to bicycle modelling

Currently, no exclusive movement model for bicycles is implemented.
Existing models need to be re-purposed

## Bicycles as slow vehicles

In this case, vehicles are specified as vehicles with the appropriate
type:

```
<vType id="bike" vClass="bicycle"/>
<vehicle type="bike" .../>
```

Note, that that the `guiShape="bicycle"` along with [sensible default
parameters](../Vehicle_Type_Parameter_Defaults.md) are
automatically used when specifying `vClass="bicycle"`. By adapting [vType-parameters for
acceleration,deceleration,maximumSpeed,etc..](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_types)
different cyclist types can be modelled.

### Problems and workarounds

- Turning left by crossing twice does not work. Extra edges need to be
  added to accommodate these trajectories.
- No bi-directional movements on bicycle lanes
- No shared space for bicycles and pedestrians
- No overtaking by vehicles on a single-lane road. This can be fixed
  by using the [Sublane Model](../Simulation/SublaneModel.md).
- The intersection model has no special adaptations for bicycles. This
  results in unrealistic (large) safety gaps when bicycles are
  approaching a large priority intersection from a prioritized road
- The road speed limit is not meaningful for bicycles. To [model a
  speed distribution for bicycles with a single vehicle
  type](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#speed_distributions),
  a speed limit corresponding to the median speed of the bicycles
  should be set for the cycling lanes.

One way for overcoming most of these problems is to control bicycle
movements at intersections with an [external control
script](../TraCI.md). This approach is described in [Integration of
an external bicycle model in SUMO, Heather
Twaddle 2016](https://www.researchgate.net/publication/302909195_Integration_of_an_External_Bicycle_Model_in_SUMO).

## Bicycles as fast pedestrians

In this case, persons walking at high speed are used.

### Problems and workarounds

  - No support for proper visualization
  - Movement model is not validated

# Building a network for bicycle simulation

## Automatic import

The import of bicycle lanes from OpenStreetMap is supported since
version 0.24.0. To use this, [an appropriate
typemap](../Networks/Import/OpenStreetMap.md#recommended_typemaps)
must be loaded.

## Generating a network with bike lanes

A bike lane is a lane which only permits the vClass *bicycle*. There are various different options for generating a network with bike lanes which are explained below. All of these options recognize the presence of an existing bike lane and will not add another lane in that case. 

### Explicit specification of additional lanes

Bike lanes may be defined explicitly in plain XML input when describing edges [edges
(plain.edg.xml)](../Networks/PlainXML.md#lane-specific_definitions). This is done by defining an additional lane which only permits the vClass “bicycle” and setting the appropriate width. In this case it may be useful to disallow bicycles on other lanes. Also, any pre-exisiting connection definitions must be modified to account for the new bike lane. 

### Explicit specification of bike lanes

Alternatively to the above method, the `<edge>`-attribute [`bikeLanWidth` may be used](../Networks/PlainXML.md#edge_descriptions). It will cause a bike lane of the specified width to be added to that edge, connections to be remapped and bicycle permissions to be removed from all other lanes.

!!! note
    The heuristic methods described below, also perform automatic connection shifting and removal of bicycle permissions from non-bike lanes

### Type-base generation

When importing edges with defined types, it is also possible to declare that certain types should receive a sidewalk. This can be used to automatically generate bike lanes for residential streets while omitting them for motorways when importing OSM data. 

```
<types>
   <type id="highway.motorway" numLanes="3" speed="44.44" priority="13" oneway="true" disallow="pedestrian bicycle"/>
   <type id="highway.unclassified"   numLanes="1" speed="13.89" priority="5" bikeLaneWidth="1" disallow="bicycle"/>
   <type id="highway.residential"    numLanes="1" speed="13.89" priority="4" bikeLaneWidth="1" disallow="bicycle"/>
   <type id="highway.living_street"  numLanes="1" speed="2.78"  priority="3"/>
   ...
</types>
```

A special type file that imports bike lanes based on additional OSM attributes can be found in [{{SUMO}}/data/typemap/osmNetconvertBicycle.typ.xml]({{Source}}data/typemap/osmNetconvertBicycle.typ.xml). This is to be preferred for importing bike lanes from OSM as it uses more accurate data. 


### Heuristic generation

A third option which can be used if no edge types are available is a heuristic based on edge speed. It adds a bike lane for all edges within a given speed range. This is controlled by using the following options:

| Option                                 | Description                                                                        |
|----------------------------------------|------------------------------------------------------------------------------------|
| **--bikelanes.guess** {{DT_BOOL}}              | Guess bike lanes based on edge speed                                               |
| **--bikelanes.guess.max-speed** {{DT_FLOAT}}    | Add bike lanes for edges with a speed equal or below the given limit *default:13.89* |
| **--bikelanes.guess.min-speed** {{DT_FLOAT}}    | Add bike lanes for edges with a speed above the given limit *default:5.8*            |
| **--bikelanes.guess.exclude** {{DT_ID}}[,<ID>]* | Specify a list of edges that shall not receive a bike lane                         |

### Permission-based generation

Option **--bikelanes.guess.from-permissons** {{DT_BOOL}} is suitable for networks which specify their edge permissions (such as [DlrNavteq](../Networks/Import/DlrNavteq.md)). It adds a bike lane for all edges which allow bicycles on any of their lanes. The option **--bikelanes.guess.exclude** {{DT_IDList}}[,{{DT_IDList}}\]* applies here as well. 

### Adding bike lanes with [netedit](../netedit.md)

To add bike lanes to a set of edges in [netedit](../netedit.md) select these and right click on them. From the context-menu select *lane operations->add restricted lane->Bikelane*. 

## Notes on Right-of-Way rules

When using bicycle lanes in a network, right-turning vehicles must yield
for straight-going bicycles. The intersection model supports these
right-of-way rules and builds internal junctions where appropriate.

Likewise, left-turning bicycles one a bicycle lane (on the right side of
the road) must yield to straight-going vehicles.

!!! caution
    The trajectories of left-turning bicycles use a wide curve rather than going straight twice. Currently, this can only be remedied by setting [custom shapes for these internal lanes](../Networks/PlainXML.md#custom_shapes_for_internal_lanes_crossings_and_walkingareas).
    
# Bicycle routing

When [routing bicycles in the simulation](../Demand/Automatic_Routing.md) the option **--device.rerouting.bike-speeds** can be used to enable separate tracking of bicycle speeds. This ensure that routing for bicycles which can use a dedicated bicycle lane is not affected by jammed cars.