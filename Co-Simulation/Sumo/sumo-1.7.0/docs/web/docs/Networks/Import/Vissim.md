---
title: Networks/Import/Vissim
permalink: /Networks/Import/Vissim/
---

Although Vissim is a microscopic simulation as SUMO is, it follows a
completely different concept of modelling traffic. Due to this, the
import is quite clumsy, does not work with all networks, and manual work
on the imported network is necessary. The option which forces
[netconvert](../../netconvert.md) to read a vissim file is **--vissim-file** {{DT_FILE}} or **--vissim** {{DT_FILE}} for
short. A usage example for [netconvert](../../netconvert.md)'s Vissim
import may look like:

```
netconvert --vissim-file=<VISSIM_FILE> --output-file=MySUMOFile.net.xml
```

Vissim-networks do possibly not contain explicit definitions of an
edge's speed. We have to propagate once set velocities, but even then
some edges' speeds may not be defined. The option **--vissim.default-speed** {{DT_FLOAT}} may change the
default speed used in the case an edge's speed is not defined. The
default value for this parameter is 13.89m/s, being around 50km/h. The
second parameter **--vissim.speed-norm** {{DT_FLOAT}} describes the factor to multiply a described flows
maximum velocity to gain the velocity to use. The default value is 1.

Furthermore, as we have to use geometrical heuristics for joining
connections to nodes, a further factor steers the process of converting
Vissim-networks: simply spoken, **--vissim.offset** {{DT_FLOAT}} holds the information how near two
nodes must be (in meters) to be joined.

During import, different actions must be done which may yield in some
loss of data and may be watched in part by setting the verbose option.

As of **SVN revision \[18592\]** the import parser used by netconvert
was extended to accomodate for the new XML input file format (`*.inpx`)
of VISSIM version \>= 6 (which makes a lot of things a lot easier)...

# Converted VISSIM net elements

- links (`STRECKEN`)
- connectors (`VERBINDER`)
- aggregation of connectors to SUMO junctions
- conflict areas (`KONFLIKTFLÄCHEN`) for the proper translation of
  priorities

## Conversion of further VISSIM-net features

is currently done by some python scripts contained in the

[VISSIM Import Toolbox](../../Tools/Import/VISSIM.md) (description in German only, currently)

which include the conversion of following elements:

- **Route definitions** and **In-flows** (vehicle sources)
- **Traffic light (LSA) program** definitions
- **Vehicle (loop-) detector** definitions

# Known Problems

- All actuated traffic lights are mapped onto the same type of traffic
  light (MSActuatedTrafficLight)
- Additional source and sink links are built

## For input files (`*.inp`) from **before VISSIM version 6**

- this works with German networks only (see [\#Converting VISSIM
  networks to
  German](#converting_vissim_networks_to_german) for tips
  on conversion)
- reading of `.inp` files containing z-coordinates for network
  geometries (link polylines) is **not supported and erroneous**.
- conflict areas (Konfliktflächen) are not being processed

# Converting VISSIM networks to German

If you have VISSIM and the German language is available, it should be
fairly simple to convert your English language network to a German file,
so that it can be imported by SUMO.

This assumes that you are starting with an English-language VISSIM.

- Click on Menu View, Options...
- Go to the Language & Units tab
- Select Language, Deutsch
- Click OK
- Click on the (now in a different language) menu Datei, Speichern
  unter (File, Save as)
- Save the file somewhere else
- Click OK on any warning messages
- To get back to English, click menu item Ansicht, Optionen (View,
  Options)
- Select tab Sprache & Einheiten
- Select Sprache, English
- Click OK