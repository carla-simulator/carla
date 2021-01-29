---
title: Geo-Coordinates
permalink: /Geo-Coordinates/
---

# Geo-Referenced Networks

SUMO networks are always encoded in Cartesian coordinates (meters) and
may contain geo-referencing information to allow conversion to lon,lat.
By default the Cartesian coordinates use the UTM-projection with the
origin shifted to so that the lower left corner of the network is at
0,0.

!!! note
    the projection information is encoded in the `<location>`-element at the top of the *.net.xml* file.

- When importing a network from
  [OSM](Networks/Import/OpenStreetMap.md), geo-referencing is
  automatically included in the generated *.net.xml* file
- When importing a network from
  [plain-xml](Networks/PlainXML.md)
  files, coordinates may be given in lon,lat and importing using a
  projection option such as **--proj.utm**
- When importing a network from Shapefile, the availability of
  geo-referencing depends on the format of the source data.

# Checking Geo-Coordinates

In [sumo-gui](sumo-gui.md) when right-clicking anywhere in a
geo-referenced network, the option *copy cursor geo-position to
clipboard* is available. The resulting *lat,lon* coordinates are
suitable for pasting into any map engine such as \[maps.google.com\] or
\[maps.bing.com\]. Also, the network coordinates as well as the
geo-coordinates at the cursor position are shown in the bottom
right-corner of the window.

# Performing coordinate-transformations

- using
  [TraCI](TraCI/Simulation_Value_Retrieval.md#command_0x82_position_conversion),
  coordinates can be transformed between network-coordinates (m,m) and
  geo-coordinates (lon,lat) and vice versa
- using [sumolib](Tools/Sumolib.md#coordinate_transformations)
  , coordinates can be transformed between network-coordinates (m,m)
  and geo-coordinates (lon,lat) and vice versa

# Obtaining output with geo-coordinates

- A network can be exported as *plain-xml* in geo-coordinates using
  the netconvert command

```
netconvert --sumo-net-file myNet.net.xml --plain-output-prefix plain --proj.plain-geo
```

- [FCD-output](Simulation/Output/FCDOutput.md) can be obtained
  in geo-coordinates by adding the option **--fcd-output.geo**