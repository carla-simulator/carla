---
title: Z/Changes from Version 0.12.0 to Version 0.12.1
permalink: /Z/Changes_from_Version_0.12.0_to_Version_0.12.1/
---

**Release Date: 7.10.2010**

- TraCI
  - added a possibility to access a vehicle's signals, wipers, and
    other things; see [Vehicle Value Retrieval](../TraCI/Vehicle_Value_Retrieval.md), [Change Vehicle State](../TraCI/Change_Vehicle_State.md), and
    [Vehicle Signalling](../TraCI/Vehicle_Signalling.md)
- Simulation
  - corrected direction when departing at a lane's begin
  - introduced a new `departpos` named "`base`". The vehicle starts at
    position=vehicle_length+.1, this means at the begin of the
    lane, but being completely on it. This `departpos` is now the default. (see
    [Definition of Vehicles, Vehicle Types, and Routes\#Vehicles and Routes](../Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#vehicles_and_routes)).
  - addressing teleports
    - corrected "incoming lanes" determination
    - added the option **--lanechange.allow-swap**; if enabled, vehicles blocking each other are exchanged
- GUI
  - debugged traffic lights tracker broken when moving to subseconds
- netconvert
  - OSM-Import
    - osm importer fails fast - furtherly given files are not
      processed in the case of an error
    - assigned default velocities to not-numerical maxspeed values
      ('none':300km/h, 'walk':5km/h); was: [defect 344: The TAPAS Cologne scenario does not work with the current(0.12.0) SUMO version - maxspeed not numeric](https://github.com/eclipse/sumo/issues/344)
    - parsing maxspeed values given as '50 km/h'; was:
      [defect 344: The TAPAS Cologne scenario does not work with the current(0.12.0) SUMO version - maxspeed not numeric](https://github.com/eclipse/sumo/issues/344)
    - new option **--osm.skip-duplicates-check** added which, well, skips checks for duplicate nodes and edges