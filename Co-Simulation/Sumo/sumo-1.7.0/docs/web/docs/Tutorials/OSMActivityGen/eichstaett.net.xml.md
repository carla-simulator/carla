---
title: Tutorials/OSMActivityGen/eichstaett.net.xml
permalink: /Tutorials/OSMActivityGen/eichstaett.net.xml/
---

[{{SUMO}}/tests/complex/tutorial/osm_activitygen/eichstaett.net.xml]({{Source}}tests/complex/tutorial/osm_activitygen/eichstaett.net.xml)

SUMO network of the German city Eichstätt. It is based on an [optimized
OpenStreetMap file](../../Tutorials/OSMActivityGen/eichstaett.osm.md)
following the [Tutorials/Import from
OpenStreetMap](../../Tutorials/Import_from_OpenStreetMap.md). The
netconvert call was:

```
    netconvert --xml-type-files ../../edge-type-templates/osm-urban-de.typ.xml --guess-ramps --remove-edges.by-vclass hov,taxi,bus,delivery,transport,lightrail,cityrail,
    rail_slow,rail_fast,motorcycle,bicycle,pedestrian --remove-geometry --remove-isolated --try-join-tls --verbose --seed 1 --osm-files eichstaett.osm --output-file
    eichstaett.net.xml
```

You can use this file if you need a road network of a town with about
13,000 inhabitants. All roads were verified to have the right highway
type, speed limit and one-way attribute. All traffic lights of cars (not
all pedestrian lights) were also verified. See also
[Tutorials/OSMActivityGen/eichstaett.osm](../../Tutorials/OSMActivityGen/eichstaett.osm.md).

<table>
<tbody>
<tr class="odd">
<td><p>Author</p></td>
<td><p><a href="http://sourceforge.net/users/w-bamberger">W. Bamberger</a></p></td>
</tr>
<tr class="even">
<td><p>Source</p></td>
<td><p><a href="../../Tutorials/OSMActivityGen/eichstaett.osm.html" title="wikilink">Tutorials/OSMActivityGen/eichstaett.osm</a> by W. Bamberger.<br />
Map data © <a href="http://www.openstreetmap.org/">OpenStreetMap contributors</a>, <a href="http://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a></p></td>
</tr>
<tr class="odd">
<td><p>History</p></td>
<td><p>This file is located in the SUMO repository. See the log there.</p></td>
</tr>
<tr class="even">
<td><p>License</p></td>
<td><p style="border:1px solid #909090; padding:1px 4px 3px 4px"><img src="../../images/CC-BY-SA-small.png">
This work is licensed under a <a href="http://creativecommons.org/licenses/by-sa/3.0/">Creative Commons Attribution-ShareAlike 3.0 Unported License</a>. The authors are listed in the history.</p>
</td>
</tr>
</tbody>
</table>