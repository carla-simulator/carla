---
title: Networks/Import
permalink: /Networks/Import/
---

[netconvert](../netconvert.md) allows to import road networks from
different third-party formats. By now, the following formats are
supported:

- OpenStreetMap databases, see
  [Networks/Import/OpenStreetMap](../Networks/Import/OpenStreetMap.md)
- PTV VISUM (a macroscopic traffic simulation package), see
  [Networks/Import/VISUM](../Networks/Import/VISUM.md)
- PTV VISSIM (a microscopic traffic simulation package), see
  [Networks/Import/Vissim](../Networks/Import/Vissim.md)
- OpenDRIVE networks, see
  [Networks/Import/OpenDRIVE](../Networks/Import/OpenDRIVE.md)
- MATsim networks, see
  [Networks/Import/MATsim](../Networks/Import/MATsim.md)
- ArcView-data base files, see [Importing ArcView networks (shapefiles)](../Networks/Import/ArcView.md)
- Elmar Brockfelds unsplitted and splitted NavTeq-data, see
  [Networks/Import/DlrNavteq](../Networks/Import/DlrNavteq.md)
- RoboCup Rescue League folders, see
  [Networks/Import/RoboCup](../Networks/Import/RoboCup.md)

In most of these cases, [netconvert](../netconvert.md) needs only
two parameter: the option named as the source application/format
followed by the name of the file to convert and the name of the output
file (using the **--output-file** option). In the case, a VISUM network shall be
imported, the following code will convert it into a SUMO-network:

```
netconvert --visum=MyVisumNet.inp --output-file=MySUMONet.net.xml
```

The import can be influenced using [further netconvert options](../Networks/Further_Options.md).

The native format comes in the variant of
[*plain-xml*](../Networks/PlainXML.md#node_descriptions)
which is designed as a simple input format for
[netconvert](../netconvert.md) and in *.net.xml* which is the
output format of [netconvert](../netconvert.md) and which is
enriched with heuristically derived data such as right-of-way rules and
junction geometry. Both formats can be converted into each other without
data loss.