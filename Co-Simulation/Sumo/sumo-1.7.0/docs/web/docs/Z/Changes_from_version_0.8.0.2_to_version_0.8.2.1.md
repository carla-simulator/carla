---
title: Z/Changes from version 0.8.0.2 to version 0.8.2.1
permalink: /Z/Changes_from_version_0.8.0.2_to_version_0.8.2.1/
---

**Please remind that several option names have changed!** The most
important: "net-files" (router, simulation) is now "net-file"...

Check user documentation and man-pages for this.

User-relevant changes:

- GUI
  - "Clear Message Window" menu added
  - the icons now allow loading instantly after the previous loading has failed
  - Renamed to "guisim"
  - using gradients instead of pure interpolation
  - windows-icons patched
  - added ability to take snapshots
  - using popups instead of comboboxes for finding structures and switching between coloring schemes in order to save place
  - speeded up handling of errors
  - Locators hidden behind a popup
  - ToolTips do now not hide dialog windows
  - Configuration errors are now reported to the window
  - Added the possibility to display a vehicle's route
  - added the possibility to save timelines
- Simulation
  - building dump-files now reports about failures
  - Patched some of the lane-changing bugs
  - time-to-teleport -option added
  - replace "treshhold" by "treshold"
- netconvert
  - Plain output of node and edges added (--plain-output)
  - got rid of the arcview/shapelib-import chaos
  - \--explicite-junctions -\> --explicite-tls
  - \--explicite-no-junctions -\> --explicite-no-tls
  - \--keep-edges.input-file
  - \--keep-edges - option added
  - \--keep-edges.input-file
  - Geometry computation bug patched
- Router
  - Bug on processing all time steps patched
  - Route output reformatted
  - max-alternatives
- Documentation
  - Man-pages added

Developer-relevant changes:

- GUI
  - Have reworked the class dependencies completely in order to make derivation more easy
- Simulation
  - output moved to "output"
  - outputs write to OutputDevices, not ostreams
  - reworked lanechanging