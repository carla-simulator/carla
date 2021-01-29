---
title: Demand/Importing O/D Matrices
permalink: /Demand/Importing_O/D_Matrices/
---

[od2trips](../../od2trips.md) computes trip tables from O/D
(origin/destination) matrices. [od2trips](../../od2trips.md) assumes
the matrix / the matrices to be coded as amounts of vehicles that drive
from one district or traffic assignment zone (TAZ) to another within a
certain time period. Because the generated trips must start and end at
edges, [od2trips](../../od2trips.md) requires a mapping of TAZ to
edges. During conversion of VISUM networks with
[netconvert](../../netconvert.md) districts stored in the VISUM input
file are parsed and stored within the generated SUMO network file. If
you do not use VISUM as input, you must build a TAZ file by your own.
The format is given in
[\#Describing_the_TAZ](#describing_the_taz) below. You have
to pass the file containing the TAZ definitions to
[od2trips](../../od2trips.md) using the **--net-file** {{DT_FILE}} (**--net** {{DT_FILE}} or **-n** {{DT_FILE}} for short) option. TAZ
can be created by drawing polygons in [netedit](../../netedit.md) and
processing them with the tool
[Tools/District\#edgesInDistricts.py](../../Tools/District.md#edgesindistrictspy).

All supported OD-formats are described in
[\#Describing_the_Matrix_Cells](#describing_the_matrix_cells)
below. You may either give a list of matrices to
[od2trips](../../od2trips.md) using the **--od-matrix-files** {{DT_FILE}}[,{{DT_FILE}}]\* (**-d** {{DT_FILE}}[,{{DT_FILE}}]\* for short) option followed
by the list of files separated using a ','.

[od2trips](../../od2trips.md) reads all matrices and generates trip
definitions. The generated trip definitions are numbered starting at
zero. You can also add a prefix to the generated trip definition names
using (**--prefix** {{DT_STR}}). As usual, they are written to the output file named using the
**--output-file** {{DT_FILE}} (**-o** {{DT_FILE}} for short). You can specify a vehicle type to be added to the trip
definitions using **--vtype** {{DT_STR}}. Please remark that vehicles will have no type unless
not given in the O/D-matrices or defined using this option. The command
line option overrides type names given in the O/D-matrices. The type
itself will not be generated. Vehicles will be generated for the time
period between **--begin** {{DT_TIME}} (**-b** {{DT_TIME}}) and -**-end** {{DT_TIME}} (**-e** {{DT_TIME}}), having 0 and 86400 as default values,
respectively. The meaning is the simulation step in seconds, as usual.

Because each O/D-matrix cell describes the amount of vehicles to be
inserted into the network within a certain time period,
[od2trips](../../od2trips.md) has to compute the vehicle's explicit
departure times. Normally, this is done by using a random time within
the time interval a O/D-matrix cell describes. It still is possible to
insert a cell's vehicles with an uniform time between their insertion.
Use the option --spread.uniform to enable this.

You can scale the amounts stored in the O/D-matrices using the **--scale** {{DT_FLOAT}} option
which assumes a float as parameter. All read flows will be multiplied
with this value, the default is 1. When importing O/D-matrices that
cover a whole day, you maybe want to apply a curve which resembles the
spread of the trip begins found in reality. Please read the subchapter
[\#Splitting_large_Matrices](#splitting_large_matrices) on
this.

# Describing the TAZ

A traffic assignment zone (or traffic analysis zone), short TAZ is
described by its id (an arbitrary name) and lists of source and
destination edges.

## Simple Definition

If you do not want to distinguish between source and sink edges and give
all edges the same probability you can use the following abbreviated
form:

```
<tazs>
    <taz id="<TAZ_ID>" edges="<EDGE_ID> <EDGE_ID> ..."/>

    ... further traffic assignment zones (districts) ...

</tazs>
```

## Differentiated Probabilities

To distinguish the set of source and sink edges (or their probabilities
respectively) use the following definition:

```
<tazs>
    <taz id="<TAZ_ID>">
      <tazSource id="<EDGE_ID>" weight="<PROBABILITY_TO_USE>"/>
      ... further source edges ...

      <tazSink id="<EDGE_ID>" weight="<PROBABILITY_TO_USE>"/>
      ... further destination edges ...
    </taz>

    ... further traffic assignment zones (districts) ...

</tazs>
```

A TAZ should have at least one source and one destination edge, each
described by its id and use probability called weight herein. These
edges are used to insert and remove vehicles into/from the network
respectively. The probability sums of each the source and the
destination lists are normalized after loading.

## Creating TAZ files

- TAZ definitions can be created directly in
  [netedit](../../netedit.md#taz_traffic_analysis_zones)
- TAZ definitions can be created by drawing polygons in
  [netedit](../../netedit.md#pois_and_polygons), then using the tool
  [edgesInDistricts.py](../../Tools/District.md#edgesindistrictspy)
  for converting polygons to TAZ.

## Further Usage for TAZ

- TAZ can be used with [route input for
  duarouter](../../Demand/Shortest_or_Optimal_Path_Routing.md#trip_definitions)
- TAZ can be used with trips and flows [for
  SUMO](../../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#incomplete_routes_trips_and_flows).
- TAZ edges appear in the simulation as special edges with the IDs
  *tazID-source* and *tazID-sink*. This can be used when specifying
  vehicle routes via [TraCI](../../TraCI/Change_Route_State.md).

## Optional Attributes

| Attribute Name | Value Type    | Description                                                         |
| -------------- | ------------- | ------------------------------------------------------------------- |
| shape          | Position list | The boundary shape for visualizing the TAZ                          |
| color          | color         | Color for drawing the shape and for coloring edges (*color by TAZ*) |

# Describing the Matrix Cells

To understand how an O/D-matrix is stored, we should remind the meanings
of the values stored herein. Each matrix describes a certain time
period. The indices within the matrix are names of the
origin/destination districts (normally they are equivalent, both lists
are the same). The values stored within the matrix are amounts of
vehicles driving from the according origin district to the according
destination district within the described time period.

The formats used by PTV are described in the VISUM-documentation more
detailed. All start with a line where the type of the O/D-matrix is
given, appended to a '$'. The first following character tells in which
format the table is stored. Then, further characters follow which
describe which values are supplied additionally within the matrix. For
further information we ask you to consult the documentation supported by
PTV. Herein, only the supported variants are described.

The vehicle type information is used by [od2trips](../../od2trips.md)
by passing it to the generated vehicles. The type definition itself will
not be generated, but the vehicle will have set the attribute `type="<TYPE>"`. The time informations are
assumed to be in the form <HOURS\>.<MINUTES\>. Please note that the end is
exclusive; for example, if

```
0.00 1.00
```

is given, the generated vehicles' depart times will be second 0 to
second 3599.

## The V format (VISUM/VISSIM)

The V-format stores the O/D matrix by giving the number of districts
(TAZ) first and then naming them. After this, for each of the named
districts, a list of vehicle amounts that leave this district is given,
sorted by the destination district names as given in the district name
list. An example may look like this:

```
$VMR
* vehicle type
4
* From-Time  To-Time
7.00 8.00
* Factor
1.00
*
* some
* additional
* comments
* District number
3
* names:
         1          2          3
*
* District 1 Sum = 6
         1          2          3
* District 2 Sum = 15
         4          5          6
* District 2 Sum = 24
         7          8          9 
```

The 'M' in the type name indicates that a vehicle type is used, the "R"
that the values shall be rounded randomly. The second information is not
processed by od2trips what means that you can parse both V-, VR-, VMR,
and VM-matrices. Please remark that both the names list and the lists
containing the amounts are written in a way that no more than 10 fields
are stored in the same line. Each of the entries they contain seem to be
left-aligned to a boundary of 11 characters (possibly 10 for the name
and one space character). Both constraints are not mandatory for the
importer used in od2trips.

## The O-format (VISUM/VISSIM)

The O-format instead simply lists each origin and each destination
together with the amount in one line (please remark that we currently
ignore the string after the ';' that occurs after the type identifier
"$OR" in the first line):

```
$OR;D2
* From-Time  To-Time
7.00 8.00
* Factor
1.00
* some
* additional
* comments
         1          1       1.00
         1          2       2.00
         1          3       3.00
         2          1       4.00
         2          2       5.00
         2          3       6.00
         3          1       7.00
         3          2       8.00
         3          3       9.00
```

- The first line is a format specifier that must be included verbatim.
- The lines starting with '\*' are comments and can be omitted
- The second non-comment line determines the time range given as
  HOUR.MINUTE HOUR.MINUTE
- The third line is a global scaling factor for the number of vehicles
  for each cell
- All other lines describe matrix cells in the form FROM TO
  NUMVEHICLES

## The Amitran format

The Amitran format defines the demand per OD pair in time slices for
every vehicle type as follows:

```
<demand xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://sumo.dlr.de/xsd/amitran/od.xsd">
   <actorConfig id="0">
       <timeSlice duration="86400000" startTime="0">
           <odPair amount="100" destination="2" origin="1"/>
       </timeSlice>
   </actorConfig>
</demand>
```

For details on the types and units see the schema at
<https://sumo.dlr.de/xsd/amitran/od.xsd>

!!! note
    The "id" value of "actorConfig" is used as the vehicle type. The Amitran schema limits this to integer values whereas SUMO allows alphanumerical type ids. To overcome this limitation, set option **--xml-validation** never when running [od2trips](../../od2trips.md) or remove the schema definition and begin your Amitran file with `<demand>`.

# Splitting large Matrices

[od2trips](../../od2trips.md) allows splitting matrices which define a
long time period into smaller parts which contain definite percentages
of the whole. There are two ways of defining the amounts the matrix
shall be split into. In both cases, the probabilities are automatically
normed.

## Free Range Definitions

The first possibility is to use the option --timeline directly. In this
case, it should be followed by a list of times and probabilities,
separated by ','. Each time and probability field is made up of two
values, an integer time being the simulation time in seconds and a
floating point number describing the probability. These two values are
separated using a ':'. At least two values must be supplied making the
definition of a timeline in this case being describable by the following
BNF-formula:

```
 <TIME>:<AMOUNT>[,<TIME>:<AMOUNT>]+
```

In this case, the matrix will be split into (fields-1) parts and each
part will have the amount described by the integral within the field.
(This means the amount specified after the last time entry will be
discarded.)

## Daily Time Lines

The second case is rather common in transportation science. It allows to
split the matrix into 24 subparts - this means the number of fields is
fixed to 24 - allowing to spread an O/D-matrix over a day describing it
by hours. To use this, give additionally the option
--timeline.day-in-hours to [od2trips](../../od2trips.md). It the
assumes the values from the --timeline - option being a list of 24
floats, divided by ',', each describing the probability of inserting a
vehicle within the according hour.

Some common daily time lines from Germany may be retrieved from:
Schmidt, Gerhard; Thomas, Bernd: Hochrechnungsfaktoren für manuelle und
automatische Kurzzeitzählungen im Innerortsbereich. Hrsg.:
Bundesministerium für Verkehr, Abteilung Straßenbau: Forschung
Straßenbau und Straßenverkehrstechnik. Heft 732, Bonn-Bad Godesberg,
1996

**Applicability of generic Time Lines**

| Name          | Description              | Reference   |
| ------------- | ------------------------ | ----------- |
| TGw2_PKW.txt | passenger vehicles, Tuesday-Thursday, cities in West Germany, type\#2 = \~streets at inner city border                                                                            | S.95        |
| TGw3_PKW.txt | passenger vehicles, Tuesday-Thursday, cities in West Germany, type\#3 = \~streets at city border                                                                                  | S.95        |
| TGs1_PKW.txt | passenger vehicles, Saturday/Sunday, cities in West Germany, group\#1 = \~inner city streets, large amount of trips to/back work, freeways with no connection to recreation areas | S.100 - 103 |
| TGw_LKW.txt  | transport vehicles, Monday-Thursday, cities in West Germany                                                                                                                       | S.98        |
| TGs_LKW.txt  | transport vehicles, Sunday, type: \~long distance roads, strong heavy duty vehicle numbers                                                                                        | S.102, 105  |

So, in dependence to the week day and the type of traffic, one could
assume using the following time-lines:

| vehicle type             | Monday       | Tuesday-Thursday | Friday | Saturday      | Sunday        |
| ------------------------ | ------------ | ---------------- | ------ | ------------- | ------------- |
| HDV                      | TGw_LKW.txt | TGw_LKW.txt     | X      | X             | TGs_LKW.txt  |
| passenger, far-distance  | X            | TGw3_PKW.txt    | X      | TGs1_PKW.txt | TGs1_PKW.txt |
| passenger, near-distance | X            | TGw2_PKW.txt    | X      | TGs1_PKW.txt | TGs1_PKW.txt |

One may note, that no time lines are given for passenger trips on
Mondays.

The time lines as such are given below, they can be directly copied into
the command line

**Generic Time Lines**

| Name        | Time Line                                                                                        |
| ----------- | ------------------------------------------------------------------------------------------------ |
| TGw_LKW    | 0.3,0.4,0.4,0.6,0.8,2.0,4.8,7.5,9.0,8.7,9.0,9.0,7.5,8.4,7.8,6.9,5.4,4.0,2.7,1.8,1.2,0.9,0.6,0.3  |
| TGw3_PKW   | 0.9,0.5,0.2,0.2,0.5,1.3,7.0,9.3,6.7,4.2,4.0,3.8,4.1,4.6,5.0,6.7,9.6,9.2,7.1,4.8,3.5,2.7,2.2,1.9  |
| TGw2_PKW   | 0.8,0.5,0.4,0.3,0.4,1.2,4.5,7.4,6.6,5.2,5.0,5.0,5.2,5.3,5.6,6.7,8.4,8.6,7.4,5.0,3.9,3.0,2.1,1.6  |
| TGs(1)_PKW | 3.3,2.8,2.0,1.5,1.2,1.3,1.2,1.5,2.5,3.7,4.8,5.5,6.0,6.7,7.0,7.1,6.9,7.4,7.0,6.0,4.7,4.1,3.5,2.3  |
| TGs_LKW    | 1.3,1.1,0.6,0.8,0.9,1.5,2.6,3.1,3.5,3.8,4.5,4.9,5.0,5.3,5.6,5.7,5.9,6.0,5.7,5.3,4.8,4.6,10.0,7.6 |

Remarks:

- All time lines describe the hourly percentage of the complete
  traffic
- values are normalized to 100% (so they don't need to add up to any
  specific value)
- It is not possible to derive time-lines for all week days

The time lines describe how the traffic (100%) spreads over the whole
day. In order to have proper demands for week-end days, one can scale
down the week day traffic. The same reference as above gives the
following scaling factors at page 31.

**Daily time-line scale factors**

| area              | Sat1992 | Sun1992 | Sat1993 | Sun1993 |
| ----------------- | ------- | ------- | ------- | ------- |
| NRW highways      | 76,1%   | 72,7%   | 72,8%   | 69,4%   |
| NRW other streets | 82,6%   | 74%     | 78,3%   | 71,6%   |
| Bavaria           | 75%     | 67,2%   | 73,9%   | 64,7%   |

One may note that this information is 15 years old. Additionally, no
information about the type of vehicles is given.

A 24h time line a given O/D-matrix shall be split by may be given to
[od2trips](../../od2trips.md) using the following options:
**--timeline.day-in-hours --timeline <TIME_LINE\>** where *<TIME_LINE\>*
is a list of 24 percentages as given above. The amount of traffic
defined within the O/D-matrix may be scaled via **--scale <SCALE\>**.
Example call to [od2trips](../../od2trips.md):

```
od2trips -n <NET> -d <MATRIX> -o <OUTPUT> --scale <SKALIERUNG> \
   --timeline.day-in-hours --timeline <TIME_LINE>
```

# Generated traffic modes

By default [od2trips](../../od2trips.md) generates vehicular traffic.
Different types of traffic (passanger cars, trucks,...) can be created by using the option
**--vtype** and **--prefix** (the latter is needed so that different types use distinct vehicle ids).

By setting one of the options **--pedestrians** or **--persontrips**, other modes can be generated.

# Combining trips from multiple calls

Sometimes it is necessary to call [od2trips](../../od2trips.md)
multiple times to generated all the traffic for a given scenario:

- to model different vehicle types which differ in their demand levels
- to model time-varying demand that is heterogeneous between the
  OD-pairs (and therefore cannot be modelled by applying a time-line)

In this case the option **--prefix** {{DT_STR}} must be used with a distinct value for each
call to [od2trips](../../od2trips.md) in order to make avoid duplicate
trip ids.

# Dealing with broken Data

[od2trips](../../od2trips.md) behaves here as following:

**incomplete districts**

- missing origin OR destination district: error
- missing origin AND destination district: warning

**incomplete connections**

- missing connection to an origin OR a destination district: error
- missing connection to an origin AND a destination district: error

<div style="border:1px solid #909090; min-height: 35px;" align="right">
<span style="float: right; margin-top: -5px;"><a href="http://cordis.europa.eu/fp7/home_en.html"><img src="../../images/FP7-small.gif" alt="Seventh Framework Programme"></a>
<a href="https://trimis.ec.europa.eu/project/assessment-methodologies-ict-multimodal-transport-user-behaviour-co2-reduction"><img src="../../images/AMITRAN-small.png" alt="AMITRAN project"></a></span>
<span style="">This part of SUMO was developed, reworked, or extended within the project 
<a href="https://trimis.ec.europa.eu/project/assessment-methodologies-ict-multimodal-transport-user-behaviour-co2-reduction">"AMITRAN"</a>, co-funded by the European Commission within the <a href="https://cordis.europa.eu/about/archives">Seventh Framework Programme</a>.</span></div>
