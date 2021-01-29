---
title: Networks/Import/VISUM
permalink: /Networks/Import/VISUM/
---

[netconvert](../../netconvert.md) can import native
[VISUM](http://www.ptvag.com/traffic/software/visum/)-network files.
Their extension is ".net". If you do not have a file with this
extension, but a ".ver"-file only, you have to generate the ".net"-file
using [VISUM](http://www.ptvag.com/traffic/software/visum/) by exporting
it from the loaded version description (".ver"-file).

The option to load a
[VISUM](http://www.ptvag.com/traffic/software/visum/) ".net"-file into
[netconvert](../../netconvert.md) in order to convert it into a
SUMO-network is named **--visum-file** {{DT_FILE}} or **--visum** {{DT_FILE}} for short. So, the following call to
[netconvert](../../netconvert.md) imports the road network stored in
"my_visum_net.net" and stores the SUMO-network generated from this
data into "my_sumo_net.net.xml":

```
netconvert --visum my_visum_net.net -o my_sumo_net.net.xml
```

!!! caution
    If your network contains geo-coordiantes (lon/lat) you must add option **--proj.utm** in order to project your network into the cartesian plane

# Import Coverage

The following table shows which information is parsed from a given VISUM
network.

!!! caution
    The keywords within a VISUM file are localized. By default netconvert assumes LANGUAGE **DEU**. You can use option **--visum.language-file** {{DT_FILE}} to read a language mapping from DEU into another language.

**Information [netconvert](../../netconvert.md) reads from VISUM
networks**

| Table name  | Imported attributes  | Description  |
|---|---|---|
| VSYS  | VSysCode (CODE)<br>VSysMode (TYP)  | Traffic modes  |
| STRECKENTYP  | Nr<br>v0-IV (V0IV)<br>Rang<br>Kap-IV (KAPIV)  | Edge types  |
| KNOTEN  | Nr<br>XKoord<br>YKoord  | Nodes  |
| BEZIRK  | Nr<br>NAME (unused later)<br>Proz_Q<br>Proz_Z<br>XKoord<br>YKoord  | Districts  |
| STRECKE (STRECKEN)  | Nr<br>VonKnot (VonKnotNr)<br>NachKnot (NachKnotNr)<br>Typ (TypNr)<br>Einbahn  | Edges  |
| ANBINDUNG  | BezNr<br>KnotNr<br>Proz<br>t0-IV<br>Typ<br>Richtung  | District connections  |
| ABBIEGEBEZIEHUNG (ABBIEGER)  | VonKnot (VonKnotNr)<br>UeberKnot (UeberKnotNr)<br>NachKnot (NachKnotNr)<br>VSysCode (VSYSSET)  | Edge Connections  |
| STRECKENPOLY  | VonKnot (VonKnotNr)<br>NachKnot (NachKnotNr)<br>INDEX<br>XKoord<br>YKoord  | Edge geometries  |
| FAHRSTREIFEN  | KNOTNR<br>STRNR<br>FSNR<br>RICHTTYP<br>LAENGE  | Lane descriptions  |
| LSA (SIGNALANLAGE)  | Nr<br>Umlaufzeit (UMLZEIT)<br>StdZwischenzeit (STDZWZEIT)<br>PhasenBasiert  | Traffic lights  |
| KNOTENZULSA (SIGNALANLAGEZUKNOTEN)  | KnotNr<br>LsaNr  | Nodes->TLS  |
| LSASIGNALGRUPPE (SIGNALGRUPPE)  | Nr<br>LsaNr<br>GzStart (GRUENANF)<br>GzEnd (GRUENENDE)<br>GELB  | Signal groups  |
| ABBZULSASIGNALGRUPPE<br>(SIGNALGRUPPEZUABBIEGER)   | SGNR (SIGNALGRUPPENNR)<br>LsaNr<br>VonKnot / VONSTRNR<br>NachKnot / NACHSTRNR<br>UeberKnot (UeberKnotNr)<br>LsaNr  | Edge connections->TLS  |
| LSAPHASE (PHASE)  | Nr<br>LsaNr<br>GzStart (GRUENANF)<br>GzEnd (GRUENENDE)  | Signal phases  |
| LSASIGNALGRUPPEZULSAPHASE  | PsNr<br>LsaNr<br>SGNR  | Signal groups->phases  |
| FAHRSTREIFENABBIEGER  | KNOT (KNOTNR)<br>VONSTR (VONSTRNR)<br>NACHSTR (NACHSTRNR)<br>VONFSNR<br>NACHFSNR  | Lane-to-lane descriptions  |


Well, basically that's all for network import, isn't it? Well, actually
not. In the following, the basic possibilities and further advices and
tricks for working with these is described.

!!! note
    "Rang" from "Streckentypen" is used as edge priority information. As streets with a lower "Rang" are normally higher priorised, an edge type's priority is computed as 100-Rang

## Lane Number

[VISUM](http://www.ptvag.com/traffic/software/visum/) does not work with
the lane numbers of streets, instead, the streets' capacities are used.
This means that under circumstances the information about the edges' lane
numbers are missing. Still, one can try to obtain the lane number from
the given edges' capacities. An approximation is:

```
LANE_NUMBER = MAXIMUM_FLOW / CAPACITY_NORM
```

The value of CAPACITY_NORM is controlled via the option **--capacity-norm** {{DT_FLOAT}} (default:
1800).

In the case the "*ANZFAHRSTREIFEN*" (lane numbers) field within the
net's "*STRECKEN*" (edges)-table is empty, and also the types do not
hold any information about the according edges' lane numbers, this
conversion is done automatically. Still, we also had
[VISUM](http://www.ptvag.com/traffic/software/visum/) nets in our hands,
where a capacity was given, but the lane number field was set to 1 for
all edges. In this case, one has to force
[netconvert](../../netconvert.md) to ignore the lane number attribute
and use the capacity. This is done by using the option
**--visum.recompute-laneno**.

## Dealing with Connectors

[VISUM](http://www.ptvag.com/traffic/software/visum/) as a macroscopic
tool, does not regard single vehicles. As one consequence,
[VISUM](http://www.ptvag.com/traffic/software/visum/) uses "connections"
through which traffic is fed from the districts into the network.

The effects on using such connectors within a microscopic simulations
should be described at a different
page. Here, we want to show the
possibilities to change the connector attributes using
[netconvert](../../netconvert.md).

# See also

[netconvert](../../netconvert.md) is able to guess some information
which is sometimes missing in imported networks. Below, you may find
links to further information of interest.

- Most [VISUM](http://www.ptvag.com/traffic/software/visum/) networks
  do not contain definitions of traffic lights positions; Still,
  [netconvert](../../netconvert.md) is able to [guess tls
  positions](../../netconvert.md#guessingtlspositions) and to [guess
  tls programs](../../netconvert.md#guessingtlsprograms).
- Also, we have not seen a
  [VISUM](http://www.ptvag.com/traffic/software/visum/) network where
  on- and off-ramps where available for highways.
  [netconvert](../../netconvert.md) is able to [guess on- and
  off-ramps](../../netconvert.md#guessingramps).
- In addition to the network, further descriptions of [lane-to-lane or
  edge-to-edge connections](../../netconvert.md#settingconnections)
  may be read.

Other possibilities of [netconvert](../../netconvert.md), such as
projection of geo-coordinates, should not apply when working with
[VISUM](http://www.ptvag.com/traffic/software/visum/) networks.

## Importing other data from VISUM

[VISUM](http://www.ptvag.com/traffic/software/visum/) uses O/D-matrices
as a demand descriptions. There is some further information on
[Demand/Importing O/D
Matrices](../../Demand/Importing_O/D_Matrices.md).

# References

- [PTV AG's
  VISUM-page](http://www.ptvag.com/software/transportation-planning-traffic-engineering/software-system-solutions/visum/visum/)
  (06.09.2011)

# Missing

- Attributes of connectors
- Importing POIs/Shapes
- what is exactly imported (how edge attributes are determined)
- other traffic modes
- Network quality