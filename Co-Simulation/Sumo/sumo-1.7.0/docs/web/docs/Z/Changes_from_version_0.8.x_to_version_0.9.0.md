---
title: Z/Changes from version 0.8.x to version 0.9.0
permalink: /Z/Changes_from_version_0.8.x_to_version_0.9.0/
---

User-relevant changes:

- GUI
  - Selections
    - Flag icons added to Popup-Menues
    - Shift adds (de)selects the whole street, not only a lane
    - Selection-Editor now saves to \*.txt-files
  - Centering of objects has been revalidated
  - Variable Speed Signs
    - Visualisation of vss added
    - Added the possibility to interact with vss'
  - "Reload Simulation" option added
  - Status-Bar message patched
  - Rotation removed (was buggy)
  - Error on closing the gui if parameter tracker are opened patched
  - Value tracker may now be aggregated and are able to save the tracked values
  - POlygon visualisation added
  - Breakpoints added
  - Setting and Saving of additional net weights added (preliminary, subject to improve)
  - Coloring by type and by route added
- Simulation
  - Reports why the simulation ended if in verbose mode
  - emissions-output error due to a division by zero patched
  - VSS may now control several lanes
- general
  - Options
    - Errors on default value usage patched
- Documentation
  - trying to separate XML-code (green) from command line options (red)
- netconvert
  - \--map-output <FILE\> builds a mapping that shows which edges have been removed while joining
  - guess-tls - options added
  - extended the FastLane-import by the -l parameter for the number of lanes
  - \--explicite-junctions <JUNCTION_NAME_LIST\> allows to say which junctions shall be explicitely tls-controlled
  - \--explicite-no-junctions <JUNCTION_NAME_LIST\> allows to say which junctions shall be explicitely non-tls-controlled
  - \--edges-min-speed <SPEED_MS\> allows to remove all edges from the input on which a lower speed than <SPEED_MS\> is allowed
  - \--elmar allows to import NavTech-GDF-files parsed using Elmar Brockfelds parser
  - \--tiger allows to import tiger files (preliminary, does not look THAAAAT good for an unknown reason, yet)
- Router
  - Generation of random route after the first depart fixed
  - Warnings about missing weights are printed only once per edge
  - Supplementary Weights
- Examples have now a new folder structure

Developer-relevant changes:

- GUI
  - Selected structures are now in a separate class
  - Additional structures are now displayed using the same "API"
- Simulation
  - removed the MSNet::dictionary
  - refactored the MSMeanData-output