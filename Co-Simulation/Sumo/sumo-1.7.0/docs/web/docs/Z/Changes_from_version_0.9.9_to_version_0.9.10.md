---
title: Z/Changes from version 0.9.9 to version 0.9.10
permalink: /Z/Changes_from_version_0.9.9_to_version_0.9.10/
---

- Building
  - debugged [bug 2042630 --enable-traci disables traci?](http://sourceforge.net/tracker/index.php?func=detail&aid=2050900&group_id=45607&atid=443421)

- All
  - solved [feature request 1725623 comma separated option lists](https://sourceforge.net/tracker/index.php?func=detail&aid=1725623&group_id=45607&atid=443424)
  - <font color="red">Network format has changed\! The elements
      'cedge' (net/edge/cedge) and 'edges' (net/edges) do not longer
      exist.</font> You do NOT have to rebuild your networks.

- Simulation
  - solved <b>"Simulation reports 'Error: The number of edges in the
      list mismatches the edge count.'"</b>
  - solved several issues reported by Gerrit Lammert (thank you):
      false ordering of loaded stops, problems with stops near to an
      end edge

- Guisim
  - vehicles now again have a default color

- TraCI
  - patched building tracitestclient_main.cpp under Fedora Core 9
      (thanks to Niek Sanders)

- Tools
  - patched dua-iterate.py - **--move-on-short** does no longer
      exist

- duarouter
  - patched behavior on missing files
  - debugged [bug 2042630 remove-loops breaks application](http://sourceforge.net/tracker/index.php?func=detail&aid=2042630&group_id=45607&atid=443421)