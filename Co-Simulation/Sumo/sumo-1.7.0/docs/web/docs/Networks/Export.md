---
title: Networks/Export
permalink: /Networks/Export/
---

# Supported export formats

## SUMO

The default output format, when using no other options. While being
partly similar in structure to the plain format below, this file is not
meant to be edited by hand since there are complex dependencies between
the various parts.

## Plain

Generated when using the option **--plain-output-prefix** {{DT_FILE}}, it generates four files containing the
nodes, the edges, the connections and the traffic light logics as
described in
[Networks/Building_Networks_from_own_XML-descriptions](../Networks/PlainXML.md).

## OpenDRIVE

The **--opendrive-output** {{DT_FILE}} option writes networks in the
[OpenDRIVE](../Networks/Import/OpenDRIVE.md) format adhering to
version 1.3.

## MATsim

The **--matsim-output** {{DT_FILE}} creates MATsim networks.

## DlrNavteq

The **--dlr-navteq-output** {{DT_FILE}} generates a links, a nodes and a traffic lights file matching
extraction version 6.0.

## Amitran

The Amitran network format consists of a single XML file conforming to
the schema at <http://sumo-sim.org/xsd/amitran/network.xsd>. The option **--amitran-output** {{DT_FILE}}
writes the data to a file with the following format

```
<?xml version="1.0" encoding="utf-8"?>

<network xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo-sim.org/xsd/amitran/network.xsd">
   <node id="0" type="rightBeforeLeft"/>
   <node id="1" type="priority"/>
   ...
   <link id="0" from="1" to="0" roadClass="4" length="136448" speedLimitKmh="20" laneNr="1"/>
   ...
</network>
```

where all values are integers (the length is in units of 0.01m, the road
class is a functional road class in the Navteq sense)

<div style="border:1px solid #909090; min-height: 35px;" align="right">
<span style="float: right; margin-top: -5px;"><a href="http://cordis.europa.eu/fp7/home_en.html"><img src="../images/FP7-small.gif" alt="Seventh Framework Programme"></a>
<a href="http://amitran.eu/"><img src="../images/AMITRAN-small.png" alt="AMITRAN project"></a></span>
<span style="">This part of SUMO was developed, reworked, or extended within the project 
<a href="http://amitran.eu/">"AMITRAN"</a>, co-funded by the European Commission within the <a href="http://cordis.europa.eu/fp7/home_en.html">Seventh Framework Programme</a>.</span></div>