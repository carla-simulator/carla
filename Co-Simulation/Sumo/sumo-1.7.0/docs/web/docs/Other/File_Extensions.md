---
title: Other/File Extensions
permalink: /Other/File_Extensions/
---

## Native SUMO Files

To ease the usage of the supplied files, all of which are within a
XML-derivate, we use a naming convention for the file extensions to
allow a distinction between the contents at first sight. The list of
used extensions is shown below. We of course highly encourage you to use
this pattern.

- Configuration files (always the first four letters of the
corresponding executable with "cfg" appended)
  - **\*.sumocfg** (formerly .sumo.cfg): Configuration file for
    [sumo](../sumo.md) and [sumo-gui](../sumo-gui.md)
    ([xsd](http://sumo.dlr.de/xsd/sumoConfiguration.xsd))
  - **\*.netccfg** (formerly .netc.cfg): Configuration file for
    [netconvert](../netconvert.md)
    ([xsd](http://sumo.dlr.de/xsd/netconvertConfiguration.xsd))
  - **\*.netgcfg** (formerly .netg.cfg): Configuration file for
    [netgenerate](../netgenerate.md)
    ([xsd](http://sumo.dlr.de/xsd/netgenerateConfiguration.xsd))
  - **\*.duarcfg** (formerly .rou.cfg): Configuration file for
    [duarouter](../duarouter.md)
    ([xsd](http://sumo.dlr.de/xsd/duarouterConfiguration.xsd))
  - **\*.jtrrcfg** (formerly .jtr.cfg): Configuration file for
    [jtrrouter](../jtrrouter.md)
    ([xsd](http://sumo.dlr.de/xsd/jtrrouterConfiguration.xsd))
  - **\*.dfrocfg** (formerly .df.cfg): Configuration file for
    [dfrouter](../dfrouter.md)
    ([xsd](http://sumo.dlr.de/xsd/dfrouterConfiguration.xsd))
  - **\*.od2tcfg** (formerly .od2t.cfg): Configuration file for
    [od2trips](../od2trips.md)
    ([xsd](http://sumo.dlr.de/xsd/od2tripsConfiguration.xsd))
  - **\*.acticfg** (formerly .act.cfg): Configuration file for
    [activitygen](../activitygen.md)
    ([xsd](http://sumo.dlr.de/xsd/activitygenConfiguration.xsd))

- Data files
  - **\*.net.xml**: network file
    ([xsd](http://sumo.dlr.de/xsd/net_file.xsd),
    [description](../Networks/SUMO_Road_Networks.md))
  - **\*.rou.xml**: routes file
    ([xsd](http://sumo.dlr.de/xsd/routes_file.xsd),
    [description](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md))
  - **\*.rou.alt.xml**: route alternatives file
    ([xsd](http://sumo.dlr.de/xsd/routes_file.xsd),
    [description](../Demand/Dynamic_User_Assignment.md#general_behavior))
  - **\*.add.xml**:
    [sumo](../sumo.md)/[sumo-gui](../sumo-gui.md) - {{AdditionalFile}}
    ([xsd](http://sumo.dlr.de/xsd/additional_file.xsd), missing description)
    - traffic lights only
      ([xsd](http://sumo.dlr.de/xsd/tllogic_file.xsd), missing
      description)
  - **\*.edg.xml**: [netconvert](../netconvert.md) - edges file
    ([xsd](http://sumo.dlr.de/xsd/edges_file.xsd),
    [description](../Networks/PlainXML.md#edge_descriptions))
  - **\*.nod.xml**: [netconvert](../netconvert.md) - nodes file
    ([xsd](http://sumo.dlr.de/xsd/nodes_file.xsd),
    [description](../Networks/PlainXML.md#node_descriptions))
  - **\*.con.xml**: [netconvert](../netconvert.md)- connection
    file ([xsd](http://sumo.dlr.de/xsd/connections_file.xsd),
    [description](../Networks/PlainXML.md#connection_descriptions))
  - **\*.typ.xml**: [netconvert](../netconvert.md)- edge types
    file ([xsd](http://sumo.dlr.de/xsd/types_file.xsd),
    [description](../SUMO_edge_type_file.md))
  - **\*.trips.xml**: trip definitions for
    [duarouter](../duarouter.md),[sumo](../sumo.md)
    ([description](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#incomplete_routes_trips_and_flows))
  - **\*.flows.xml**: flow definitions for
    [jtrrouter](../jtrrouter.md),[duarouter](../duarouter.md),[sumo](../sumo.md)
    ([description](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#incomplete_routes_trips_and_flows))
  - **\*.turns.xml**: turn and sink definitions for
    [jtrrouter](../jtrrouter.md)
    ([xsd](http://sumo.dlr.de/xsd/turns_file.xsd),
    [description](../Demand/Routing_by_Turn_Probabilities.md))
  - **\*.taz.xml**: traffic analysis zones (or districts) file
    mainly for
    [od2trips](../od2trips.md),[duarouter](../duarouter.md),[sumo](../sumo.md)
    ([xsd](http://sumo.dlr.de/xsd/taz_file.xsd))
    ([description](../Demand/Importing_O/D_Matrices.md#describing_the_taz))

- Output files
  - **\*.xml**: inductive loop output
    ([xsd](http://sumo.dlr.de/xsd/det_e1_file.xsd),
    [description](../Simulation/Output/Induction_Loops_Detectors_(E1).md)
  - **\*.xml**: areal lane detector output
    ([xsd](http://sumo.dlr.de/xsd/det_e2_file.xsd),
    [description](../Simulation/Output/Lanearea_Detectors_(E2).md))
  - **\*.xml**: areal lane detector output
    ([xsd](http://sumo.dlr.de/xsd/det_e3_file.xsd),
    [description](../Simulation/Output/Multi-Entry-Exit_Detectors_(E3).md))
  - **\*.xml**: emissions output
    ([xsd](http://sumo.dlr.de/xsd/emission_file.xsd),
    [description](../Simulation/Output/EmissionOutput.md))
  - **\*.xml**: fcd output
    ([xsd](http://sumo.dlr.de/xsd/fcd_file.xsd),
    [description](../Simulation/Output/FCDOutput.md))
  - **\*.xml**: full output
    ([xsd](http://sumo.dlr.de/xsd/full_file.xsd),
    [description](../Simulation/Output/FullOutput.md))
  - **\*.xml**: meandata output
    ([xsd](http://sumo.dlr.de/xsd/meandata_file.xsd),
    [description](../Simulation/Output/VTypeProbe.md))
  - **\*.xml**: netstate output
    ([xsd](http://sumo.dlr.de/xsd/netstate_file.xsd),
    [description](../Simulation/Output/VTypeProbe.md))
  - **\*.xml**: queue output
    ([xsd](http://sumo.dlr.de/xsd/queue_file.xsd),
    [description](../Simulation/Output/QueueOutput.md))
  - **\*.xml**: summary output
    ([xsd](http://sumo.dlr.de/xsd/summary_file.xsd),
    [description](../Simulation/Output/Summary.md))
  - **\*.xml**: tripinfo output
    ([xsd](http://sumo.dlr.de/xsd/tripinfo_file.xsd),
    [description](../Simulation/Output/TripInfo.md))
  - **\*.xml**: vtypeprobe output
    ([xsd](http://sumo.dlr.de/xsd/vtypeprobe_file.xsd),
    [description](../Simulation/Output/VTypeProbe.md))

- Other files
  - **\*.xml**: edge diff
    ([xsd](http://sumo.dlr.de/xsd/edgediff_file.xsd), missing
    description)

## Imported Files

- **\*.osm**: OpenStreetMap XML databases as imported by
[netconvert](../netconvert.md) and
[polyconvert](../polyconvert.md), see [OpenStreetMap
file](../OpenStreetMap_file.md)
- **\*.xodr**: OpenDRIVE XML network files as imported by
[netconvert](../netconvert.md)
- **\*.inp**: VISSIM network files as imported by
[netconvert](../netconvert.md)
- **\*.net**: VISUM network files as imported by
[netconvert](../netconvert.md) and
[polyconvert](../polyconvert.md), see
[Networks/Import/VISUM](../Networks/Import/VISUM.md)
- **\*.shp, \*.shx, \*.dbf**: ArcView-network descriptions (shapes,
shape indices, definitions) as imported by
[netconvert](../netconvert.md) and
[polyconvert](../polyconvert.md)
- **\*.xml**:
  - MATSim road networks as imported by
    [netconvert](../netconvert.md), see
    [Networks/Import/MATsim](../Networks/Import/MATsim.md)

## Exported Files

- **\*.xml**:
  - MATSim road networks, see [Networks/Further Outputs](../Networks/Further_Outputs.md)
  - OMNET: mobility-traces, see
    [Tools/TraceExporter](../Tools/TraceExporter.md)
  - Shawn: snapshot-files, see
    [Tools/TraceExporter](../Tools/TraceExporter.md)
- **\*.xodr**: OpenDRIVE XML network, see [Networks/Further
Outputs](../Networks/Further_Outputs.md)
- **\*.tcl**: ns2/ns3 trace-files, activity-files, and mobility-files,
see [Tools/TraceExporter](../Tools/TraceExporter.md)
- **\*.dri**, **\*.str**, **\*.fzp**, **\*.flt**: PHEM input files,
see [Tools/TraceExporter](../Tools/TraceExporter.md)
- **unknown**:
  - GPSDAT, see
    [Tools/TraceExporter](../Tools/TraceExporter.md)