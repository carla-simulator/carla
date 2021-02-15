---
title: Other/Glossary
permalink: /Other/Glossary/
---

## A

- **application**: 
An application is a computer program, herein, an
"application" mostly means one of the main, compiled programs that
are included in the SUMO package.
  
- **additional-file**:
A file that can be included in a sumo
configuration. This is needed to load pois, shapes, bus stops and
variable speed signs. Refer to [the format
description](../sumo.md#format_of_additional_files) for
details.

## C

- **connection(s)-file**:
An XML-file describing connections between edges
or lanes, see [connection descriptions for importing networks
defined as
XML](../Networks/PlainXML.md#connection_descriptions)

## D

- **DELPHI**:
Deutschlandweite Echtzeit Verkehrs-Lage und Prognose im
Ereignisfall
  
- **destination**:
The position of the end of a vehicle journey. Mostly a
TAZ processed by OD2TRIPs within which a "sink" edge lies
  
- **detector**:
An artificial device for object (mainly vehicle) states
recognition and/or logging
  
- **DFD**:
Deutsches Fernerkundungsdatenzentrum des DLR
  
- **district**:
see TAZ
  
- **DLR**:
Deutsches Zentrum für Luft- und Raumfahrt
  
- **DUA**:
Dynamic User Assignment
  
- **DUE**:
Dynamic User Equilibrium

## E

- **edge**:
A single-directed street connection between two points
(junctions/nodes). An edge contains at least one lane
  
- **edge(s)-file**:
A XML-file describing edges of a road network, see
[edge descriptions for importing networks defined as
XML](../Networks/PlainXML.md#edge_descriptions)

## F

- **FCD**:
Floating Car Data

## I

- **induction loop**:
A detector placed on a certain position on a lane
which recognizes vehicles passing it and logs their attributes
(speed, size, etc.)

## J

- **junction**:
The place an edge begins or ends at (same as node)
  
- **junction logic**:
The part of a junction which determines the vehicle
behavior at the according junction by using certain right-of-way -
rules

## L

- **link**:
A connection between two lanes within a junction. Within
microsim, each lane has links (connections) to the following lanes.
A link contains the information whether the vehicle has to
decelerate in front of the junction.

## M

- **macroscopic**:
In a macroscopic traffic (flow) simulation, the atomic
instances are roads; the flow is simulated directly (see
microscopic)
  
- **mesoscopic**:
In a mesoscopic traffic (flow) simulation, the streets
are partitioned into sections which multiple vehicles may enter and
leave at each simulation step
  
- **microscopic**:
In a microscopic traffic (flow) simulation, the atomic
instances are vehicles; the flow is simulated by simulating the
vehicles (see macroscopic)
  
- **multimodal**:
A multimodal traffic simulation is capable of processing
different types of traffic (busses, trains, vehicles etc.)

## N

- **(street) network**:
In our terms, a network is the combination of
junctions (nodes) and edges (streets)
  
- **node**:
A node (junction) is a single point were at least one edge
(road) starts or ends
  
- **node(s)-file**:
A XML-file describing nodes of a road network, see
[node descriptions for importing networks defined as
XML](../Networks/PlainXML.md#node_descriptions)

## O

- **O/D-matrix (od-matrix)**:
Origin/Destination-matrix; describes how
many vehicles are moving from each origin to each destination within
a certain time period
  
- **option**:
A parameter for the application which determines what or how
an application shall do
  
- **origin**:
The position of the begin of a vehicle journey. Mostly a TAZ
processed by OD2TRIPs within which a "source" edge lies

## P

- **poi**:
short for "point of interest"; A position of an object that may
be interesting

## R

- **route**:
A route is a complete description of a vehicle's path over
the network; it contains the information when the vehicle departs
(the route starts) and over which edges the vehicle shall drive

## S

- **script**:
A small computer program which normally doees not have to be
compiled. The SUMO package includes several scripts, most written in
Python
  
- **source**:
A source means a place at which vehicles are inserted into
the street network
  
- **source-tar-ball**:
A file containing the sources (program code) for a
project. The source code must be compiled in order to get the
runnable application. It is called "tar-ball" because of using the
application "tar" to join all the source files into a single file.
  
- **submicroscopic**:
In a submicroscopic traffic (flow) simulation, the
atomic instances are parts of the vehicle or the driver (gearing or
behavior is modelled)

## T

- **TAZ**:
Traffic Assignment Zone, [a collection of edges describing a
part of a road network's
area](../Demand/Importing_O/D_Matrices.md#describing_the_taz)
(also called 'district')
  
- **TLS**:
traffic lights system
  
- **trip**:
A reduced information about a vehicle's movement; contains
only the departure time, the begin, and the end edge. Must be
transformed into a route using a router for being used within the
simulation
  
- **trip-table (trip list)**:
A file containing several trips
  
- **TS**:
Institute of Transportation Systems at the German Aerospace
Center (DLR)

- **TS-BS**:
TS Standort Braunschweig
  
- **TS-BA**:
TS Standort Berlin-Adlershof
  
- **type(s)-file**:
A XML-file describing types of streets, see [type
descriptions for importing networks defined as
XML](../Networks/PlainXML.md#type_descriptions)

## X

- **XML**:
"e**X**tensible **M**arkup **L**anguage is a universal format
for structured documents and data" as the w3c says. Further
information may be found at <http://www.w3c.org/XML/> or
<http://en.wikipedia.org/wiki/XML>. Most of the data read and
written by SUMO is stored as XML, see also the page on [file
types](../Other/File_Extensions.md).
  
- **XSD**:
**X**ML **S**chema **D**efinitions "... provide a means for
defining the structure, content and semantics of XML documents" as
the w3c says. Further information may be found at
<http://www.w3.org/XML/Schema> or
<http://en.wikipedia.org/wiki/XML_Schema_%28W3C%29>. A part of the
XML data formats used by SUMO is covered by xsd, see the page on
[file types](../Other/File_Extensions.md).