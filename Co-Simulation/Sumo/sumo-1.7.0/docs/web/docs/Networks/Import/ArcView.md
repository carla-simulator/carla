---
title: Networks/Import/ArcView
permalink: /Networks/Import/ArcView/
---

[netconvert](../../netconvert.md) is able to directly read binary
ArcView databases ("shapefiles"). To convert such databases, at least
three files are needed: a file with the extension ".dbf", one with the
extension ".shp" and one with the extension ".shx". Additionally, having
a projection file with the extension ".proj" is of benefit. The option
to load a shape-file into [netconvert](../../netconvert.md) in order
to convert it into a SUMO-network is named **--shapefile-prefix** {{DT_FILE}}. Because shape-file
descriptions use more than a single file, one has to supply the file
name without extension, only. So, the following call to
[netconvert](../../netconvert.md) should be used to import the road
network stored in "my_shape_files.shp", "my_shape_files.shx",
"my_shape_files.proj", and "my_shape_files.dbf":

```
netconvert --shapefile-prefix my_shape_files
```

Unfortunately, shape files describe how information is stored
physically, but neither which is stored nor how the entries of the
according database (\*.dbf) are named. Due to this, one has to examine
how a given road network is stored within the database file and give
this information to [netconvert](../../netconvert.md); a plain call
almost always fails.

# Defining the Input

The table below shows which information
[netconvert](../../netconvert.md) is trying to read from the given
input files, what the standard values are, and how they can be changed.
Also, it shows whether the information is mandatory - must be given - or
optional.

<center>

**Table: Information [netconvert](../../netconvert.md) reads from
shape-files**

</center>

| Information  | Mandatory  | Default field name  | Option  |
|--------------|------------|---------------------|---------|
| The id of an edge  | y  | **LINK_ID**  | **--shapefile.street-id <FIELD_NAME\>**  |
| The street name of an edge (need not be unique, for displaying)  | n  | **ST_NAME**  | **--shapefile.name <FIELD_NAME\>**  |
| The name of the node the edge starts at  | y  | **REF_IN_ID**  | **--shapefile.from-id <FIELD_NAME\>**  |
| The name of the node the edge ends at  | y  | **NREF_IN_ID**  | **--shapefile.to-id <FIELD_NAME\>**  |
| The type of the street  | n  | **ST_TYP_AFT**  | **--shapefile.type-id <FIELD_NAME\>**  |
| Speed category  | n (see below)  | **SPEED_CAT**  |   |
| Lane category  | n (see below)  | **LANE_CAT**  |   |
| Road class, used to determine the priority  | n (see below)  | **FUNC_CLASS** 	  |   |
| Allowed speed on a road  | n (see below)  | **SPEED** or **speed**  |  **--shapefile.speed <FIELD_NAME\>**  |
| Number of lanes a road has  | n (see below)  | **NOLANES**, **nolanes** or **rnol**  | **--shapefile.laneNumber <FIELD_NAME\>**  |
| Direction of travel ("B": both, "F": forward, "T": backward)  | n  | **DIR_TRAVEL**  |   |

If being familiar with NavTeq, you may have noticed that the defaults
are the ones that are used by NavTeq.

Some shape file databases do not contain explicit information about the
edges' attributes (number of lanes, priority, allowed speed) at all. It
is possible use [SUMO edge type file](../../SUMO_edge_type_file.md)
for describing the edges' attributes. In this case, the column to
retrieve an according street's type name from must be named using **--shapefile.type-id** {{DT_IDList}} and a
[SUMO edge type file](../../SUMO_edge_type_file.md) must be given to
[netconvert](../../netconvert.md) using **--type-files** {{DT_FILE}}. If something fails with the
types or the explicit values, it can be catched using **--shapefile.use-defaults-on-failure**. In these cases,
the default [netconvert](../../netconvert.md) values are used. Besides
this, it is possible to load own [connection
descriptions](../../Networks/PlainXML.md#connection_descriptions).

!!! note
    One can insert own attributes into the database using a GIS. This means, one can also insert own fields for the number of lanes, priority, or allowed speed, naming them as described above.

ArcView-networks are encoded using geocoordinates which have to be
converted to the cartesian coordinates system used by SUMO. To describe
how to convert the coordinates, one should know in which UTM-zone your
network is located. Use projection
options to set the correct one.

!!! caution
    Road geometries must be encoded with type 'linestring'

# ArcView Import Options

The complete list of options used for reading shapefiles is given in the
table below. You may find further options which control the import
behavior on [netconvert](../../netconvert.md).

| Option                              | Description                                                           |
|-------------------------------------|-----------------------------------------------------------------------|
| **--shapefile-prefix** {{DT_FILE}}           | Read shapefiles (ArcView, Tiger, ...) from files starting with 'FILE' |
| **--shapefile.street-id** {{DT_STR}}      | Read edge ids from column STR                                         |
| **--shapefile.from-id** {{DT_STR}}        | Read from-node ids from column STR                                    |
| **--shapefile.to-id** {{DT_STR}}          | Read to-node ids from column STR                                      |
| **--shapefile.type-id** {{DT_STR}}        | Read type ids from column STR                                         |
| **--shapefile.use-defaults-on-failure** | Uses edge type defaults on problems; *default: **false***                   |
| **--shapefile.all-bidirectional**       | Insert edges in both directions; *default: **false***                       |
| **--shapefile.guess-projection**        | Guess the proper projection; *default: **false***                           |

# Examples

## '36_NEW_YORK' from TIGER database

The network is available at the [TIGER2008 ftp server
(?)](ftp://ftp2.census.gov/geo/tiger/TIGER2008/36_NEW_YORK/36061_New_York_County/).
**<font color="red">Missing copyright information; please add</font>**.

Opening the edges-dbf (*tl_2008_36061_edges.dbf*) of which we hope
that it contains information about roads shows us, that:

- the from- and the to-nodes of the streets are described in fields
  named **TNIDF** and **TNIDT**, respectively.
- the field **tlid** may be stored as the one to name the edges by
- there is a further information about the type (mtfcc)

As a begin, we try to import the road graph. We use the information we
have found and apply a projection:

```
netconvert -v --shapefile-prefix tl_2008_36061_edges -o net.net.xml \
   --shapefile.from-id TNIDF --shapefile.to-id TNIDT --arcview.street-id tlid \
   --shapefile.use-defaults-on-failure
```

As a result, we obtain the network shown below.

![tl_2008_36061_edges.gif](../../images/Tl_2008_36061_edges.gif
"tl_2008_36061_edges.gif")

**Figure 1.1. The converted network of New York**

There are several issues one should note:

- The types were not used - no information about their meanings was
  investigated
- All roads are unidirectional
- The projection has not been verified (though it should be valid)
- Maybe the further files which are included in the zip contain
  additional information. This has not been investigated.

## 'Frida' network (city of Osnabrück)

The network is available at the
[Frida-homepage](http://frida.intevation.org/) and is licensed under the
GPL.

Our main interest is of course the street network. The following files
describe this: *strassen.dbf*, *strassen.shp*, *strassen.shx*
("strassen" is the german word for "streets"). When opening
"strassen.dbf" we have to realize that there is only a few information
stored herein - neither the node names are given nor the street
attributes. Instead, the street attributes seem to be stored in an
additional database and are references by type names (column "strTypID"
- strassen_typ_id = street_type_id). Also, the names of this
database's columns have other names than expected.

Ok, let's solve these problems one after another.

- Different field naming

  The only problem with this is that we can not extract street names
  properly. Still, within [FRIDA](http://frida.intevation.org/), the
  edges are numbered, and we may use the street id as name.
  The call has to be extended by: **--arcview.street-id strShapeID**


- Missing node names

  [netconvert](../../netconvert.md) can deal with networks which do
  not have node names (since version 0.9.4).


- Parsing street attributes from a [SUMO edge type
  file](../../SUMO_edge_type_file.md)

  The possibility to describe edges using attributes was already
  available in [netconvert](../../netconvert.md) and may be used in
  combination with ArcView files since version 0.9.4. Still, the types
  have to be translated into XML. The generated file (*frida.typ.xml*)
  looks like this:

```
<types>
  <!-- "noch nicht attributiert" (= not yet attributed) -->
  <type id="0"  priority="1" numLanes="1" speed="13.89"/>
  <!-- "Autobahn" (highway) -->
  <type id="1"  priority="5" numLanes="3" speed="41.67"/>
  <!-- "Bundesstrasse" (motorway) -->
  <type id="2"  priority="4" numLanes="1" speed="22.22"/>
  <!-- "Hauptstrasse" (main (city) road) -->
  <type id="3"  priority="3" numLanes="2" speed="13.89"/>
  <!-- "Nebenstrasse" (lower priorised (city) road) -->
  <type id="4"  priority="2" numLanes="1" speed="13.89"/>
  <!-- "Weg" (path) -->
  <type id="5"  priority="1" numLanes="1" speed="5"/>
  <!-- "Zone 30" (lower street with a speed limit of 30km/h) -->
  <type id="6"  priority="2" numLanes="1" speed="8.33"/>
  <!-- "Spielstrasse" (a street where children may play (10km/h)) -->
  <type id="7"  priority="1" numLanes="1" speed="1.39"/>
  <!-- "Fussgaengerzone" (pedestrains zone) -->
  <type id="8"  priority="0" numLanes="1" speed="0.1"/>
  <!-- "gesperrte Strasse" (closed street) -->
  <type id="9"  priority="0" numLanes="1" speed="0.1"/>
  <!-- "sonstige Strasse" (something else) -->
  <type id="10" priority="0" numLanes="1" speed="0.1"/>
  <!-- "Fussweg" (way for pedestrians) -->
  <type id="11" priority="0" numLanes="1" speed="0.1"/>
</types>
```

The call has to be extended by: **-t frida.typ.xml --arcview.type-id strTypID**

After having applied all those changes, the network was buildable, but
looked quite messy. After having played with geocoordinate projections,
this was fixed. So the call (so far) looks like this:

```
netconvert --arcview strassen -o frida.net.xml \
  --arcview.street-id strShapeID -t frida.typ.xml \
  --arcview.type-id strTypID --use-projection
```

### Data Quality

Looking a bit deeper at the network, we had to realise two further
problems. At first, highway on- and off-ramps are marked as "highway".
this yields in a network where on- and offramps have the same number of
lanes as the highways themselves. And it's definitely not fitting to
reality, as the next picture shows:

![frida_uni_highway_ramp.png](../../images/Frida_uni_highway_ramp.png "frida_uni_highway_ramp.png")

**Figure 2.2. Detail view showing problems with (unidirectional) highway
on-/off-ramps**

Furthermore, all streets are unidirectional - even highways. This makes
the network not usable for traffic simulations when left in the orignal
state. Trying to convert the network with **--arcview.all-bidi**, that means trying to insert
edges bidirectional, makes the city usable, but the highways are even
worse, now, because also the on-/off-ramps are bidirectional, then...

![frida_bidi_highway_ramp.png](../../images/Frida_bidi_highway_ramp.png "frida_bidi_highway_ramp.png")

**Figure 2.3. Detail view showing problems with (bidirectional) highway
on-/off-ramps**

### Demand

There is no demand available for Frida - at least none we know about.

### Conclusion

Using the current features we are able to parse the network from the
Frida-project but we can not state it is completely usable for traffic
simulations. At least areas around highways are not realistic, because
on-/offramps lack an explicit declaration and are due to this as wide as
the highways themselves. Furthermore, all streets within the network are
coded in just one direction. Extending them to be bidirectional solves
the problem in inner-city areas, but yields in an unacceptable result
for highways.

<p style="border:1px solid #909090; padding:1px 4px 3px 4px"><img src="../../images/CC-BY-SA-small.png" alt="Creative Commons License">
This work is licensed under a <a href="http://creativecommons.org/licenses/by-sa/3.0/">Creative Commons Attribution-ShareAlike 3.0 Unported License</a>. The authors are listed in the history.</p>