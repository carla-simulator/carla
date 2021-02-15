---
title: Topics/Environmental Issues
permalink: /Topics/Environmental_Issues/
---

One of the major burdens traffic poses to society is its impact on the
environment in means of air and noise pollution as well as the
consumption of non-renewable materials. Much work is put into the
development of solutions that reduce these harms. A traffic simulation
should support such development by allowing to measure the amount of
generated pollution and the amount of consumed fuel.
[sumo](../sumo.md) (used for both, [sumo](../sumo.md) and
[sumo-gui](../sumo-gui.md) in the following) includes models and
interfaces that fulfill these needs.

In the following, the topic of "environmental issues" as seen from
[sumo](../sumo.md) perspective is described in the following.

# User Interaction

Pollutants/noise/consumption is computed only if the user asks for it or
if it is used for visualisation. Pollutants emitted by the simulated
vehicles can be visualised using [sumo-gui](../sumo-gui.md) or be
written into output files, both by [sumo](../sumo.md) and
[sumo-gui](../sumo-gui.md). The following output can be used:

- [trip information](../Simulation/Output/TripInfo.md): in
  combination with the emissions device, the tripinfo output contains
  the sum of all pollutants emitted / fuel consumed during a vehicle's
  journey;
- [edge/lane
  emissions](../Simulation/Output/Lane-_or_Edge-based_Emissions_Measures.md):
  these output files contain the pollutants emitted and fuel consumed
  at an edge / a lane, aggregated over a variable time span;
- [edge/lane
  noise](../Simulation/Output/Lane-_or_Edge-based_Noise_Measures.md):
  these output files contain the noise produced at an edge / a lane,
  aggregated over a variable time span.

Further information can be found in the outputs' documentation.

A SUMO-vehicle owns the attribute `"emissionClass"` (see [Vehicle Emission
Classes](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_emission_classes)).
This attribute defines which emission model and which of its parameter
sets shall be used to compute the emissions. The models' resolutions and
coverage of the vehicle population differ among the emission models.

Currently, the noise model only distinguishes two vehicle types:
passenger and heavy duty vehicles. The discrimination is done based on
the vehicle's `emissionClass`; if the vehicle
belongs to a passenger emission class, the noise parameter for a
passenger vehicle are used, otherwise those of a heavy duty vehicle.

# Models

SUMO includes the following emission models:

- [HBEFA v2.1-based](../Models/Emissions/HBEFA-based.md): A
  continuous reformulation of the [HBEFA](http://www.hbefa.net/) [\[1\]](#references)
  v2.1 emissions data base;
- [PHEMlight](../Models/Emissions/PHEMlight.md), a derivation of
  the original
  [PHEM](http://www.ivt.tugraz.at/de/forschung/emissionen.html)[\[2\]](#references)
  emission model.

Both models implement different vehicle emission classes. These classes
can be assigned to vehicles by using the vehicle type attribute
"`emissionClass`" (see [Vehicle Emission
Classes](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicle_emission_classes)).
Available emission classes can be found within the emission model
descriptions ([HBEFA
v2.1-based](../Models/Emissions/HBEFA-based.md),
[PHEMlight](../Models/Emissions/PHEMlight.md)).

The noise model is based on HARMONOISE.

<center>

**Table: pollutants covered by models**

<table>
  <tr>
    <th class="tg-amwm" rowspan="2">model</th>
    <th class="tg-amwm" colspan="6">pollutant / measurement</th>
  </tr>
  <tr>
    <td class="tg-amwm">CO2</td>
    <td class="tg-amwm">CO</td>
    <td class="tg-amwm">HC</td>
    <td class="tg-amwm">NOx</td>
    <td class="tg-amwm">PMx</td>
    <td class="tg-amwm">fuel</td>
  </tr>
  <tr>
    <td class="tg-0lax"><a href="../Models/Emissions/HBEFA-based.html">HBEFA v2.1-based</a></td>
    <td class="tg-baqh">x</td>
    <td class="tg-baqh">x</td>
    <td class="tg-baqh">x</td>
    <td class="tg-baqh">x</td>
    <td class="tg-baqh">x</td>
    <td class="tg-baqh">x</td>
  </tr>
  <tr>
    <td class="tg-0lax"><a href="../Models/Emissions/PHEMlight.html">PHEMlight</a></td>
    <td class="tg-baqh">x</td>
    <td class="tg-baqh">x</td>
    <td class="tg-baqh">x</td>
    <td class="tg-baqh">x</td>
    <td class="tg-baqh">x</td>
    <td class="tg-baqh">x</td>
  </tr>
</table>

</center>

# Research

## Emissions Modelling

The emission models implemented in [sumo](../sumo.md) re-use
existing models and data bases. The first steps including an overview of
fifteen reviewed models as well as the derivation of the [HBEFA
v2.1-based](../Models/Emissions/HBEFA-based.md) model are given in
the deliverable "*D3.1 – Traffic Modelling: Environmental Factors*"[\[3\]](#references)
of the [iTETRIS](http://www.ict-itetris.eu/)[\[4\]](#references) project. The
development of [PHEMlight](../Models/Emissions/PHEMlight.md) is
described in the deliverable "*Deliverable 4.1 - Extended Simulation
Tool PHEM coupled to SUMO with User Guide* (Draft)"[\[5\]](#references) of the
[COLOMBO](http://colombo-fp7.eu/)[\[6\]](#references) project.

## Emission-optimal Routing

Usually, route computation is performed using travel times as weights
for the edges of a road network. But what if one would use the emitted
pollutants instead? Would their emission be reduced? The first
investigations on this topic were performed using a real-world network
within the [iTETRIS](http://www.ict-itetris.eu/)[\[7\]](#references) project and were
reported in its deliverable D3.1[\[3\]](#references). Further steps using real-world
scenarios are given in [\[7\]](#references), [\[8\]](#references) and [\[9\]](#references). To gain deeper
knowledge about the dynamics of the processes, later investigations
([\[10\]](#references), [\[11\]](#references)) were performed using
synthetic scenarios.

## Evaluation of real Traffic Management Actions

European authorities are forced by the "Directive 2008/50/EC of the
European Parliament and of the Council"[\[12\]](#references) to assure certain air
quality. Traffic management, usually operated by local authorities, has
the duty to perform corrective actions that reduce road traffic's
impact, if needed.
[MARLIS](http://www.bast.de/nn_42544/DE/Publikationen/Datenbanken/MARLIS/MARLIS.html)[\[13\]](#references)
is a database that lists such actions performed by traffic management
authorities. A proof-of-concept for simulating such actions using SUMO
is presented in [\[14\]](#references). Tomàs Josep Vergés used this approach to
simulate and evaluate some of such actions in his Master thesis[\[15\]](#references)

# Further Resources

## Further Interfaces

- The tool [traceExporter.py](../Tools/TraceExporter.md) converts
  SUMO's [fcd-output](../Simulation/Output/FCDOutput.md) into
  files that can be directly read by the
  [PHEM](http://www.ivt.tugraz.at/de/forschung/emissionen.html)[\[2\]](#references)
  application. A [tutorial on generating trace files (including PHEM
  input files)](../Tutorials/Trace_File_Generation.md) using this
  tool is available.

## Tools

- [emissionsMap](../Tools/Emissions.md#emissionsmap) generates a
  map of emission for a named or all emission classes by iterating
  over velocity, acceleration, and slope
- [emissionsDrivingCycle](../Tools/Emissions.md#emissionsdrivingcycle)
  generates emissions for a vehicle by virtually following a driving
  cycle

# References

<references />

1.  [*HBEFA* - Handbook emission factors for road
    transport](http://www.hbefa.net/) (German). INFRAS. Last visited on
    8th of January 2014.

2.  [*Emissionen und Energieverbrauch von
    Antriebskonzepten*](http://www.ivt.tugraz.at/de/forschung/emissionen.html)
    (German). Institute for Internal Combustion Engines and
    Thermodynamics at Graz University of Technology. Last visited on 8th
    of January 2014.

3.  Krajzewicz, D.; Nippold, R. and Lazaro, O. *D3.1 – Traffic
    Modelling: Environmental Factors*. iTETRIS consortium, 2009

4.  [*iTETRIS* - An Integrated Wireless and Traffic Platform for
    Real-Time Road Traffic Management
    Solutions](http://www.ict-itetris.eu/). iTETRIS web site. iTETRIS
    consortium. Last visited on 8th of January 2014.

5.  N. Furian, S. Hausberger and D. Krajzewicz *Deliverable 4.1 -
    Extended Simulation Tool PHEM coupled to SUMO with User Guide*
    (Draft). COLOMBO consortium, 2013

6.  [*COLOMBO* - Cooperative Self-Organizing System for low Carbon
    Mobility at low Penetration Rates](http://colombo-fp7.eu/). COLOMBO
    web site. COLOMBO consortium. Last visited on 8th of January 2014.

7.  D. Krajzewicz and L. Bieker. *Investigating Ecological Impacts on
    selected Traffic Management Methods*. 3rd NEARCTIS Workshop, 2010

8. D. Krajzewicz, L. Bieker, E. Brockfeld, R. Nippold and J. Ringel.
    *Ökologische Einflüsse ausgewählter Verkehrsmanagementansätze*.
    Heureka '11, 2011

9. D. Krajzewicz and P. Wagner. *Large-scale Vehicle Routing Scenarios
    based on Pollutant Emission*. In: G. Meyer and J. Valldorf (Eds.).
    *Advanced Microsystems for Automotive Applications 2011*, AMAA 2011,
    Springer, 2011, pp. 237-246.

10. M. Behrisch and Y.-P. Flötteröd and D. Krajzewicz and P. Wagner.
    *Ecological User Equilibrium?* DTA 2012, 2011

11. Y.-P. Flötteröd and P. Wagner and M. Behrisch and D. Krajzewicz.
    *Simulated-based Validity Analysis of Ecological User Equilibrium*.
    In: Winter Simulation Conference Archive, 2012 Winter Simulation
    Conference, 2012

12. The European Parliament and the Council of the European Union.
    *Directive 2008/50/EC*. Official Journal of the European Union, L
    152/1. 2008.

13. [*MARLIS* - Datenbank mit Maßnahmen zur Reinhaltung der Luft in
    Bezug auf Immissionen an Straßen, Version
    3.1](http://www.bast.de/nn_42544/DE/Publikationen/Datenbanken/MARLIS/MARLIS.html)
    (German). BASt database index. Last visited on 8th of January 2014.

14. D. Krajzewicz and Y.-P. Flötteröd. *Simulative Untersuchung
    abstrakter und realer Verkehrsmanagementansätze zur
    Emissionsreduktion*. In: *Kolloquium Luftqualität an Straßen 2013*,
    pp. 42-57. Bundesanstalt für Straßenwesen. 2013.

15. Tomàs Josep Vergés. *Analysis and simulation of traffic management
    actions for traffic emission reduction*. TU Berlin. 2013.

<div style="border:1px solid #909090; min-height: 35px;" align="right">
<span style="float: right; margin-top: -5px;"><a href="http://cordis.europa.eu/fp7/home_en.html"><img src="../images/FP7-small.gif" alt="Seventh Framework Programme"></a>
<a href="http://colombo-fp7.eu/"><img src="../images/COLOMBO-small.png" alt="COLOMBO project"></a></span>
<span style="">This part of SUMO was developed, reworked, or extended within the project 
<a href="http://colombo-fp7.eu/">"COLOMBO"</a>, co-funded by the European Commission within the <a href="http://cordis.europa.eu/fp7/home_en.html">Seventh Framework Programme</a>.</span></div>