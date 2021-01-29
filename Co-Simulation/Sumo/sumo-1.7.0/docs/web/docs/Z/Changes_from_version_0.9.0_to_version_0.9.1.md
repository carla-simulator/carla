---
title: Z/Changes from version 0.9.0 to version 0.9.1
permalink: /Z/Changes_from_version_0.9.0_to_version_0.9.1/
---

User-relevant changes

- GUI
  - bug on not loading junction geometry patched
- Simulation
  - actuated tls-lights bug patched

Developer-relevant changes

- GUI
  - unused addTimeout/removeTimeout-APIs removed
- Simulation
  - renamed NLNetHandler to NLHandler
  - each MSJunction has a reference to the net; net allows retrieval of the tls-logics container (see helpers/NamedObjectCont.h)
  - removed several memory leaks, mainly occuring after broken loading
  - bugs in actuated traffic lights logic patched
- Router
  - renamed all ROJP\*-classes to ROJTR\*