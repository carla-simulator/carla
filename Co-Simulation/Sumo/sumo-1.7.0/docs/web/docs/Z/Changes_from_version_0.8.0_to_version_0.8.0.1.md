---
title: Z/Changes from version 0.8.0 to version 0.8.0.1
permalink: /Z/Changes_from_version_0.8.0_to_version_0.8.0.1/
---

- GUI
  - The gui now remembers the last used folder
  - The gui now remembers his
  - The instance chooser was reworked and shows now which items are selected
  - An selected-items dialog with the ability to save the names of the selected items was implemented
  - New colors for aggregated view (aggregated views are still a prototype)
  - Fixed a bug at optional texture disabling
  - Update of trackers and tables on simulation steps added
  - About-dialog debugged
  - Message window scrolls if new text is appended
- Router
  - Output holding the number of loaded, build, and discarded routes
    added
  - \--move-on-short function implemented
  - Bug on repeatedly emitted vehicles patched
- Simulation
  - Bug on repeatedly emitted vehicles patched
- netconvert
  - Made the geometry of junctions pretty again
  - Too conservative right-of-way rules on importing vissim-networks was reworked
- general implementation issues
  - Replaced most output to cerr by MsgHandler-calls
- Documentation
  - extended the detector-documentation (user)
  - extended the gui-documentation
  - extended the description of the build process
  - added missing pictures
  - resized some pictures
  - improved the visibility of XML-format descriptions

... and some other things ...