---
title: Contributed/SUMOPy/Demand/Zone To Zone
permalink: /Contributed/SUMOPy/Demand/Zone_To_Zone/
---

We first explain the general concept of Zone to zone demand generation
before we describe how to procede with SUMOPy.

1.  *Zone definition*: Definition of Zones, in SUMO also called “Traffic
    Assignment Zones” (TAZ). A TAZ defines the area where participants
    depart (zone of origin) or arrive (zone of destination). A TAZ does
    typically contain several network edges.
2.  *Zone-to-Zone flow definition*: Definition the number of trips
    between each zone of origin and a zone of destination (= OD-flow).
    This structure is widely known as the Origin-to Destination matrix
    OD Matrix. OD-flows are defined.
  - for different time intervals of the day.
  - for different transport mode.
3.  *Trip generation*: Each OD-flow is disaggregated into a discrete
    number of individual trips, departing at different edges (and edge
    positions) within the zone of origin, and at different time
    instances within the specified time interval; and arriving at
    different edges (and edge positions) within the zone of destination.
4.  *Routing*: A route is computed for each individual trip, connecting
    the edge within the zone of origin, with the edge within the zone of
    destination.

These steps can be performed with SUMOPy as follows....

From the SUMOPy network manipulation tools, select the `Add zone tool`.

After giving the zone a name \[1\] a polygon can be drawn on the network
with a series of `<Button-Left>` - clicks. Complete the zone with a final
`<Double-Button-Left>` - click. A `<Double-Button-Right>` - click will
aboard the current zone drawing. Currently, zones must be convex,
otherwise edge detection problems occur.

Only edges which are located entirely inside a zone are considered part
of a zone. Only edges inside a zone are considered for departure or
arrival of vehicles in the respective zone. After creating the zones you
can identify all edges in each zone by selecting the menu item

`Landuse>Zones>Identify zone edges`

It is possible to see all zone edges and and change zone names by using
the information tool and by clicking on the green zone border. Zones are
accessible under `scenario.demand.zones`. Zones can also be deleted with
the `Delete tool` of the network editor.

Zone-to-Zone flows can be added by selecting the menu item

`Demand>Zone-to-zone demand>Add zone-to-zone flows`

An “Add OD flow wizard” will pop up as shown in Fig.
[below](../../../images/Fig_sumopy_gui_odwizard.png).

![fig_sumopy_gui_odwizard.png](../../../images/Fig_sumopy_gui_odwizard.png
"Add OD wizard")

With this wizard, specify the time interval (in entire seconds) and the
transport mode. On the menu of the wizard select `Edit>Add OD-flow to
table`. Then a new row will appear in the table. Enter the zones of
origin and destination and the respective number of trips between them.

Instead of entering the OD flows manually, the wizard offers also the
possible to import OD-flows from a CSV file. Select from the wizard
menu:

`File>Import CSV ...`

and choose a CSV file from the file-dialog window. The CSV file must
have the following format:

```xml
<zone name origin 1>, <zone name destination 1>,<trip number 1>
<zone name origin 2>, <zone name destination 2>,<trip number 2>
...
```

Once the OD flows are all entered, press `Save flows`. The scale factor
can be used to multiply all entered trips with a constant (default is 1)
while saving. The demand flows are now saved to SUMOPy and can be
browsed and modified under

`scenario.demand.odintervals`

Generate trips from OD flows by selecting menu item

`Demand>Zone-to-zone demand>Generate trips from flows`

The generated trips can be browsed and modified under

`scenario.demand.trips`

Perform a shortest path routing for each trip by selecting

`Demand>Trips and router>Trips to routes with shortest path`

The generated route can be browsed under

`scenario.demand.trips.routes`

With this method, SUMO’s duarouter is used to perform the routing. Note
that each trip is now linked to a route (see ID route column in trips).
If there is no route then the edge in the zone of origin is probably not
connected to the edge in the zone of destination \[2\]. The router does
not route pedestrians, but this is not necessary as their exact route
will be determined during simulation.

The scenario is now ready to be simulated by selecting

`Simulation>Sumo>export routes and simulate...`

Proceed as described in Sec. [\[1\]](../../../Contributed/SUMOPy/GUI/Getting_Started.md#simulating_the_scenario).

A test file for OD demand is located in

`SUMOHOME/tools/contributed/sumopy/testscenario/demo_dem_od_bikes.csv`

1.  zone names can added/modified later via browser
2.  there can be several reasons for this, usually the destination edge
    is in access-restricted areas, or there are one-way roads, impeding
    access. Actually this should not happen too often, as the
    disaggregation algorithm should verify accessibility