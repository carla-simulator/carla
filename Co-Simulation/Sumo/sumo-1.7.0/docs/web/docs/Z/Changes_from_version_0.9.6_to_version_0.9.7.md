---
title: Z/Changes from version 0.9.6 to version 0.9.7
permalink: /Z/Changes_from_version_0.9.6_to_version_0.9.7/
---

- 23.05.07: sumo-gui now contains storages for recent configs AND
  networks, usage of "RecentFiles" reworked
- 23.05.07: DOCS: generated pdfs are copied to <SUMO_DIST\>/docs
- 24.05.07: DOCS: removed references to man-pages in docs section
- 24.05.07: DOCS: links to FAQ and Publications point to the wiki now
- 24.05.07: duarouter/SIM: removed the snipplet embedding prototype
  (was not yet used)
- 24.05.07: TOOLS: added script to apply Astyle on the complete code
- 24.05.07: TOOLS: flowrouter.py and flowFromRoutes.py can deal with
  non-integer flows
- 24.05.07: TOOLS: flowrouter.py has an option to ignore detector
  types
- 24.05.07: NETGEN: replaced an occurence of rand() with randSUMO()
  and reworked the randomness code a little
- 30.05.07: dfrouter: moved to the new exception-handling concept (see
  [Developer/Implementation Notes/Error Handling](../Developer/Implementation_Notes/Error_Handling.md))
- 30.05.07: dfrouter: removed bug "\[ 1724214 \] no user information
  about missing file description header"
- 31.05.07: declaring ";" list divider as deprecated
- 06.06.07: large code recheck: moved warning pragmas to
  windows-config; patched inclusion of the correct configuration file
- 07.06.07: started to rework exception handling (no more
  XMLBuildingExceptions, mostly ProcessErrors)
- 12.06.07: changed API for xml-handlers (now only supplying the
  numerical element ids); made handler functions not pure virtual
- 12.06.07: made PROJ and GDAL optional
- 20.06.07: new XML header in most output files
- 22.06.07: Version info added automatically
- 27./29.06.07: reworked computation of lane-to-lane directions and
  main direction (see also OpenProblems)
- 02.07.07: Added tests for error handling while loading broken
  networks (sumo, jtrrouter, duarouter, dfrouter)
- somewhere between: reworked application subsystem (removing
  OptionsSubSys)
- renaming variables to the myVar scheme
- 16.07.07: removed -l to be used as a shortcut for --lane-weights
  because -l is already used as a shortcut for --log-file (duarouter,
  jtrrouter)
- 16.07.07: removed the omit-unbuild-edges - option; there should be
  no other functionality than for --dismiss-loading-errors
  (netconvert, NETGEN)
- 17.07.07: refactored VISUM-net importer; now using 2 classes instead
  of 18 (netconvert)
- 17.07.07: debugged sumo-gui's options handling
- 17.07.07: removed support for ARTEMIS (never used since 2003)
- OutputDevice supports TCP sockets
- skipping MSVC6 / 7 support
- DFRouter beautification
- JTRRouter uses vehicle classes
- possibility to switch traffic lights off
- preparing subsecond simulation
- Cell/Fastlane support removed
- unsupported applications removed
- starting detector refactorization
- emitting at the end of the step
- ITM-RemoteServer renamed to TraCI
- Visual Studio 64bit build (without Proj, GDAL and Fox)
- Route handling changed (multiref deprecated)