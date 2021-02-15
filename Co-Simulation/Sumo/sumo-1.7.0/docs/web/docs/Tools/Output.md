---
title: Tools/Output
permalink: /Tools/Output/
---

All of the tools described below exist in *tools/output* directory.

## generateITetrisIntersectionMetrics.py

Tool used for generating the intersection metrics, including (but not
limited to):

- CO emission
- CO2 emission
- HC emission
- PMx emission
- NOx emission
- fuel consumption

Execute the *generateITetrisIntersectionMetrics.py* script with *--help*
option to get details about usage and available options.

## generateITetrisNetworkMetrics.py

Tool used for generating the network metrics, including (but not limited
to):

- CO emission
- CO2 emission
- HC emission
- PMx emission
- NOx emission
- fuel consumption

Execute the *generateITetrisINetworkMetrics.py* script with *--help*
option to get details about usage and available options.

## generateMeanDataDefinitions.py

Script for generating mean data definitions from detector definitions.

Execute the *generateITetrisINetworkMetrics.py* script with *--help*
option to get details about usage and available options.

## generateTLSE1Detectors.py

Script for generating [E1 detectors (induction loops)](../Simulation/Output/Induction_Loops_Detectors_(E1).md)
for each junction in the supplied network file.

Example usage:
```
python tools\output\generateTLSE1Detectors.py -n .net.net.xml -o detectors.add.xml
```

Execute the *generateTLSE1Detectors.py*script with *--help* option to
get details about additional options.

## generateTLSE2Detectors.py

Script for generating [E2 detectors (lanearea detectors)](../Simulation/Output/Lanearea_Detectors_(E2).md) for
each junction in the supplied network file.

Example usage:
```
python tools\output\generateTLSE2Detectors.py -n .net.net.xml -o detectors.add.xml
```

Execute the *generateTLSE2Detectors.py* script with *--help* option to
get details about additional options.


## generateTLSE3Detectors.py

Script for generating [E3 detectors (multi-entry/multi-exit detectors)](../Simulation/Output/Multi-Entry-Exit_Detectors_(E3).md)
around all TLS-controlled intersections (default) or for an arbitrary
list of intersections (**--junction-ids**). By default each entry edge gets its own
detector. When setting option **--joined** there will be one detector per junction
instead. When setting option **--interior**, delays within the intersection will be
included as well.


Example usage:
```
python tools\output\generateTLSE3Detectors.py -n .net.net.xml -o detectors.add.xml
```

Execute the *generateTLSE3Detectors.py* script with *--help* option to
get details about additional options.


## netdumpdiff.py

Script creating a diff of two netdump files.

Execute the *netdumpdiff.py* script with *--help* option to get details
about usage and available options.

## netdumpmean.py

Script calculating the mean values from two netdump files.

Execute the *netdumpmean.py* script with *--help* option to get details
about usage and available options.

## timingStats.py

Script for gathering statistics from several SUMO runs.

Execute the *timingStats.py* script with *--help* option to get details
about usage and available options.

## accelerations.py

Script for computing aggregate statistics about vehicle accelerations
based on [--netstate-dump](../Simulation/Output/RawDump.md) output.

## vehLanes.py

Script for computing
[vehroute](../Simulation/Output/VehRoutes.md)-like output for lanes
based on [--netstate-dump](../Simulation/Output/RawDump.md) output.
Output data also includes information about the number of lane changes
for each vehicle.

usage:

```
python vehLanes.py <netstate_dump.xml> <output_file>
```

## edgeDataDiff.py

Computes the numerical difference of
[edgeData](../Simulation/Output/Lane-_or_Edge-based_Traffic_Measures.md)
values for each edge and interval. The resulting file can be used to
[visualize changes between two traffic scenarios](../sumo-gui.md#visualizing_edge-related_data). Both
input files must contain the same edges and intervals.

usage:

```
python edgeDataDiff.py <edgeData1.xml> <edgeData2.xml> <diffFile.xml>
```

## vehrouteDiff.py

Computes the difference in travel times between two sets of **--vehroute-output** files. The
files must contain the same vehicles and the same routes and may only
differ in travel times. The must have been generated with the option **--vehroute-output.exit-times**
for the script to work.

usage:

```
python vehrouteDiff.py routes1.rou.xml routes2.rou.xml result.xml
```

## tripinfoDiff.py

Computes the difference in travel times, route length, time loss,
departure- and arrival times between two sets of **--tripinfo-output** files. The files
should contain the same vehicles.

usage:

```
python tripinfoDiff.py tripinfos1.xml tripinfos2.xml result.xml
```

By default only `<tripinfo>` elements are considered. By setting option **--persons**, the
difference for `<personinfo>` elements is computed instead.

## tripinfoByTAZ.py

Aggregates tripinfo data by origin/destination TAZ. The TAZ data can
either by taken from the original input file (if it contains 'fromTaz'
and 'toTaz' attributes) or from a TAZ file.

```
python tripinfoByTAZ.py -t tripinfos.xml -r trips.xml
python tripinfoByTAZ.py -t tripinfos.xml -z taz.axml
```

By default traveltime (tripinfo attribute 'duration') is aggregated.
Other attributes can be selected using option **--attribute** (e.g. 'routeLength').
Output is given as plain text on the command line or in xml format if
option **--output** is set.

## attributeStats.py

Computes statistics on an arbitrary numerical attribute in a flat xml
document. (eg. attribute *timeLoss* for element *tripinfo* in
[tripinfo-output](../Simulation/Output/TripInfo.md)) and writes the
results to standard output. When the attribute contains time data in HH:MM:SS format, the values will automatically be converted to seconds.

```
python tools/output/attributeStats --element tripinfo --attribute timeLoss
```

With option **--hist-output** {{DT_FILE}} a histogram data file (e.g. for plotting with gnuplot) is
generated.

## computeCoordination.py

This tool reads fcd-outut and a corridor definition. It computes the fraction of vehicles that passed the corridor without significant slow-dow.

Example:
```
python tools/output/computeCoordination.py --fcd-file fcd.xml --filter-route B,C,D,E --entry C --min-speed 5
```
This computes the fraction of vehicles that passed the edges *B,C,D,E* in order (possibly with gaps) and were delayed after passing edge *C* to less then 5m/s.

With option **--full-output** {{DT_FILE}} Each vehicle that passed the corridor is recorded with entry time and the time at which it was delayed (-1 it it was not delayed).