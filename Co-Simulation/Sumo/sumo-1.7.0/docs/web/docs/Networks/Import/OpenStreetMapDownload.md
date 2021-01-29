---
title: Networks/Import/OpenStreetMapDownload
permalink: /Networks/Import/OpenStreetMapDownload/
---

From <http://www.openstreetmap.org/>:

> *"[OpenStreetMap](http://www.openstreetmap.org/) is a free editable
> map of the whole world. It is made by people like you."*
>
> *"[OpenStreetMap](http://www.openstreetmap.org/) creates and provides
> free geographic data such as street maps to anyone who wants them. The
> project was started because most maps you think of as free actually
> have legal or technical restrictions on their use, holding back people
> from using them in creative, productive, or unexpected ways."*

The amount of data contained within
[OpenStreetMap](http://www.openstreetmap.org/), and their quality, is
really amazing. Even though no demand data is available, the quality of
the road networks makes it worth to regard
[OpenStreetMap](http://www.openstreetmap.org/) as a data source for
traffic simulations, especially if one is interested in multi- or
inter-modal simulations.

This page describes how OpenStreetMap data can be obtained. OSM data
must be converted before it can be used with Sumo (see
[Networks/Import/OpenStreetMap](../../Networks/Import/OpenStreetMap.md)).
The page [OpenStreetMap file](../../OpenStreetMap_file.md) gives
further general information on the data format.

# Downloading a small rectangular area

## Using the [osmWebWizard](../../Tools/Import/OSM.md)

Data can be downloaded by selecting an area in the browser. In addition
to downloading and building the network, demand for various traffic
modes can be generated automatically.

## Directly from the Browser

Data can be downloaded comfortably at
<http://www.openstreetmap.org/export> by selecting a rectangular area.
However, this page limits the amount of data that can be downloaded.
Depending on the data-density in the selected area this may limit the
downloadable area to less than a square kilometer (although downloading
a whole city may be possible).

## With the [Java OpenStreetMap Editor (JOSM)](http://josm.openstreetmap.de/)

The [Java OpenStreetMap Editor (JOSM)](http://josm.openstreetmap.de/) is
a mature tool for handling OSM-data. It allows for comfortable
downloading of rectangular areas using an interactive map. It even
supports searching for an area by name (i.e. Berlin). It also supports
filtering and editing of OSM Data.

# Downloading a Larger Rectangular Area Using the OpenStreetMap API

Larger amounts of OSM-data can be downloaded using a web browser or
preferably with a program such as
[wget](http://wget.addictivecode.org/). For detailed information refer
to the [API overview](http://wiki.openstreetmap.org/wiki/API)

Downloading OSM-data via the API is a two step process: First you must
determine the geo-coordinates for your area. Then you can download
OSM-data using those coordinates.

## Obtain the geo-coordinates

You need the (longitude, latitude)-numbers for the south-west corner and
for the north-east corner of your chosen area. You can get them at [this
page](http://www.openstreetmap.de/karte.html) by pointing your mouse at
the appropriate map location and observing the coordinates in the status
bar. The inner city of Berlin lies within the following coordinates:

```
SW-corner: 13.278 52.473
NE-corner: 13.471 52.552
```

## Download the area

Construct an [URI (web
adress)](http://en.wikipedia.org/wiki/Uniform_Resource_Identifier) from
the geo-coordinates and download the data from that URI. The URI takes
the following form:

```
http://api.openstreetmap.org/api/0.6/map?bbox=<SW-longitude,SW-latitude,NE-longitude,NE-latitude>
```

for the above example this becomes

```
http://api.openstreetmap.org/api/0.6/map?bbox=13.278,52.473,13.471,52.552
```

you can download your data by entering the URI in a web-browers or by
using another program such as [wget](http://wget.addictivecode.org/).

```
wget.exe "http://api.openstreetmap.org/api/0.6/map?bbox=13.278,52.473,13.471,52.552" -O berlin.osm.xml
```

**Important:** the API version number *0.6* will eventually be outdated.
Refer to the [openstreetmap API
page](http://wiki.openstreetmap.org/wiki/API) for the latest version.

# Downloading a City

Modelling a single city is a common simulation use case. [OSM Overpass
API](http://www.overpass-api.de/) provides a comfortable interface to
obtain the road network and infrastructure within a city boundary (or
other administrative region).

This is two step Process: First you must obtain the Id (Identification
Number) of the area you want to download. Then you can use that Id to
download the area.

## Obtaining an area Id

Copy the following query into the input box on the the [OSM Overpass API
query page](http://www.overpass-api.de/query_form.html). Replace
*Berlin* by the city name of your choice and click the accompanying
button "Explore".

```
 <query type="relation">
 <has-kv k="boundary" v="administrative"/>
 <has-kv k="name" v="Berlin"/>
 </query>
 <print mode="body"/>
```

This will prompt you to download or open a file containing the answer to
your query.

```
 <relation id="62422">
   <member type="way" ref="36991886" role="outer"/>
   ...
   <member type="way" ref="44020341" role="outer"/>
   <tag k="de:amtlicher_gemeindeschluessel" v="11"/>
   <tag k="TMC:cid_58:tabcd_1:LocationCode" v="266"/>
   <tag k="type" v="multipolygon"/>
   <tag k="boundary" v="administrative"/>
   <tag k="admin_level" v="4"/>
   <tag k="TMC:cid_58:tabcd_1:Class" v="Area"/>
   <tag k="name" v="Berlin"/>
   <tag k="source" v="http://wiki.openstreetmap.org/wiki/Import/Catalogue/Kreisgrenzen_Deutschland_2005"/>
   <tag k="TMC:cid_58:tabcd_1:LCLversion" v="9.00"/>
 </relation>
 <relation id="119876">
   <member type="way" ref="33628254" role="outer"/>
   <tag k="tiger:PLACEFP" v="07304"/>
   <tag k="tiger:PLACENS" v="02403860"/>
   <tag k="tiger:PLCIDFP" v="1307304"/>
   <tag k="wikipedia" v="Berlin,_Georgia"/>
   <tag k="type" v="multipolygon"/>
   <tag k="place" v="city"/>
   <tag k="boundary" v="administrative"/>
   <tag k="admin_level" v="8"/>
   <tag k="name" v="Berlin"/>
   <tag k="border_type" v="city"/>
   <tag k="created_by" v="polyshp2osm-multipoly"/>
   <tag k="source" v="TIGER/Line® 2008 Place Shapefiles (http://www.census.gov/geo/www/tiger/)"/>
   <tag k="tiger:reviewed" v="no"/>
   <tag k="is_in:country" v="USA"/>
   ...
 </relation>
```

The important parts of result data are the `<relation id="...">` elements.
This `id` attribute will be used to download the OSM-data for your
chosen city.

**Important:** as in the *Berlin*-query above, there may be multiple
results. Usually the accompanying tags will be sufficiently helpful to
distinguish between *Berlin,Germany* and *Berlin,USA/Georgia*. If the
output does not contain the region you were looking for, it might be
helpful to add the type of region e.g. *Berlin, city*. If problems
persist read on at the [OSM Overpass API](http://www.overpass-api.de/)
site.

## Downloading an area by Id

The Id obtained in the previous step for the city of *Berlin* was 62422.
Technically this is only the Id of the *border* of Berlin and we must
add the number 3,600,000,000 to obtain the Id of the area of berlin.
`3,600,000,000 + 62,422 = 3,600,062,422`

Using this number we construct another query for an input box on the
[OSM Overpass API query
page](http://www.overpass-api.de/query_form.html).

```
 <osm-script timeout="180" element-limit="20000000">
 <union>
   <area-query ref="3600062422"/>
   <recurse type="node-relation" into="rels"/>
   <recurse type="node-way"/>
   <recurse type="way-relation"/>
 </union>
 <union>
   <item/>
   <recurse type="way-node"/>
 </union>
 <print mode="body"/>
 </osm-script>
```

Replace the number for the `<area-query ref="...">` by the number from
above and click the accompanying button "Download". This will prompt you
to download or open a file containing the OSM-data for your city.

**Important:** For large cities you may have to adapt the values for
`timeout` and `element-limit`.

# Downloading a very large area

## Pre-packaged Areas

A large amount of pre-packaged areas are available at
<http://www.geofabrik.de/data/download.html>. Many of these are quite
large and will probably require filtering before they can be reasonably
processed. Importing the whole of Germany with \[netconvert\] takes
about 60GB of RAM\!

## Custom Areas

The OpenStreetMap-API still limits the size of an area to extract. The
help script *osmGet.py* located within {{SUMO}}/tools/import/osm allows to
extract a larger area by splitting the requests. The call is:

```
osmGet.py <PREFIX> <BOUNDING_BOX> <TILES_NUMBER>
```

The bounding box must be given as
<LAT_MIN\>,<LONG_MIN\>,<LAT_MAX\>,<LONG_MAX\>.

!!! note
    "wget" must be installed and located in the execution path.

This script will extract OSM-data as *n* files with *n* =<TILES_NUMBER\>,
named "<PREFIX\><INDEX\>_<TILES_NUMBER\>.osm.xml". These files can be
imported by the scripts *osmBuild.Py* and *osmBuildPolys.py*, also
located in {{SUMO}}/tools/import/osm. This is described in
[Networks/Import/OpenStreetMap\#Import
Scripts](../../Networks/Import/OpenStreetMap.md#import_scripts).