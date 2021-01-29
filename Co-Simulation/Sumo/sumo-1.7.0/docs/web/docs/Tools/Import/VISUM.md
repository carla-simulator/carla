---
title: Tools/Import/VISUM
permalink: /Tools/Import/VISUM/
---

### Importing "Zaehlstellen"

[VISUM](https://vision-traffic.ptvgroup.com/de/products/ptv-visum/) can store count
positions, and optionally, their values. As count positions may be
extended by user attributes, we have set up a small script which
extracts both, the positions, and the user attributes, and fixes the
positions on the given network. The tool is named
"*visum_parseZaehlstelle.py*" and can be found in
{{SUMO}}\\tools\\import\\visum.

The call is:

```
visum_parseZaehlstelle.py <SUMO-net> <VISUM-net> <output>
```

This means that you have to give both, the converted, and the
original net to this tool. *<output\>* is the name of the output file to generate. The tool converts the
"ZAEHLSTELLE" elements into a list of pois, located at the first
(rightmost) lanes of the network; these can be used as [additional Polygons and POIs within the Simulation](../../Simulation/Shapes.md).