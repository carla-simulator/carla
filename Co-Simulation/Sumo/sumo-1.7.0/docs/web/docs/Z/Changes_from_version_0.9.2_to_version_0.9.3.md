---
title: Z/Changes from version 0.9.2 to version 0.9.3
permalink: /Z/Changes_from_version_0.9.2_to_version_0.9.3/
---

User-relevant changes

- GUI
  - version information in gui patched
  - segfaults on loading broken configs patched
  - catching opening a second file using recent files added
  - subwindows are now deleted on (re)loading the simulation
  - some improvements on visualisation (unfinished (GridBuilder))
  - viewport changer debugged
  - problems on loading geometry items patched
  - list of selected items is now cleared on closing/reloading (was
  buggy before)
  - table editing debugged
  - rerouters are now correctly named
- Simulation
  - debugging numeric issues in MSVehicle
  - Emitters reworked
- Netconversion
  - output during netbuilding patched
  - network geometry reworked (unfinished)
  - "speed-in-km" is now called "speed-in-kmh"
  - "type-file" is now called "xml-type-files"
  - removed two files definition for arcview
  - allowed further information to be stored in arcview-files
- Documentation
  - User docs, netconversion partly updated
  - further work on tls-api-description
- All
  - copyright information patched

Developer-relevant changes

- GUI
  - unused addTimeout/removeTimeout-APIs removed
  - debugged building of the gui-application with MSVC7
- Simulation
  - (MS|GUI)TriggeredEmitter is now (MS|GUI)Emitter
  - TLS-API: MSEdgeContinuations added
  - usage of internal lanes is now optional at building
- All
  - got rid of the old MSVC memory leak checker
  - several memory leaks removed
  - made checking the geometry for correctness optional on building
    (CHECK_UNIQUE_POINTS_GEOMETRY)
  - made memory checking optional on building (CHECK_MEMORY_LEAKS)
  - foreign libraries will now be moved to src/foreign