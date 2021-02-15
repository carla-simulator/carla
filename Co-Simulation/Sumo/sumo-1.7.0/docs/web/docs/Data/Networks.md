---
title: Data/Networks
permalink: /Data/Networks/
---

You should read the main page on [netconvert](../netconvert.md)
before reading this.

# OpenStreetMap-project

- *URL*: <https://www.openstreetmap.org/>
- *Area*: the complete world :-)
- *Demand data*: not available
- *Extraction*:

    1. [Networks/Import/OpenStreetMapDownload](../Networks/Import/OpenStreetMapDownload.md)
        explains how to obtain OSM-data.

    2. [Networks/Import/OpenStreetMap](../Networks/Import/OpenStreetMap.md)
        explains how to create Sumo-networks from OSM-data using
        [netconvert](../netconvert.md).

*Comments*: A great source of networks and man-made infrastucture.
However, infrastructure data and definitions are still changing
which complicates import.

*See also*:
[Data/Scenarios/TAPASCologne](../Data/Scenarios/TAPASCologne.md) - a complete scenario based on OSM and TAPAS

# Frida-project

- *URL*: <http://frida.intevation.org/>
- *Area*: city of Osnabrück
- *Demand data*: not available
- *Extraction*: **netconvert --shapefile strassen -o frida2.net.xml
--shapefile.street-id strShapeID -t frida.typ.xml
--shapefile.type-id strTypID -v --proj "+proj=utm +ellps=bessel
+units=m"**

*Comments*: Not really applicable because the streets lack
information about the number of lanes and the direction. Especially
the second makes the converted network quite useless. Trying to use
the option **--shapefile.all-bidi** solves the problem for
inner-city roads but makes highway on/offramps bidirectional, too.
The projection might be wrong as well, give **--proj.utm** a try.

*See also*:

[Networks/Import/ArcView](../Networks/Import/ArcView.md) -
(notes on) importing ArcView networks (shapefiles)