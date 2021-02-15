---
title: Networks/Import/MATsim
permalink: /Networks/Import/MATsim/
---

For reading MATsim networks, give [netconvert](../../netconvert.md)
the option **--matsim-files** {{DT_FILE}}\[,{{DT_FILE}}\]\* or **--matsim** {{DT_FILE}}\[,{{DT_FILE}}\]\* for short. For example:

```
netconvert --matsim myMATsimNetwork.xml -o mySUMOnetwork.net.xml
```

MATsim does not work with the lane numbers of streets, instead, the
streets' capacities are used. This means that under circumstances the
information about the edges' lane numbers are missing. The importer tries
to determine the lane number from the given edges' capacities using:

```
LANE_NUMBER = MAXIMUM_FLOW / CAPACITY_NORM
```

The value of CAPACITY_NORM is controlled via the option **--capacity-norm** {{DT_FLOAT}} (default:
1800).