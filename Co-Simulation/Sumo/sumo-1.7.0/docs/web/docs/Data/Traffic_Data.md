---
title: Data/Traffic Data
permalink: /Data/Traffic_Data/
---

# Traffic Data

## FSP - Data from the *Freeway Service Patrol Evaluation Project*

<font color="red">This description is not yet completed</font>

The data includes:

- Traffic measures at induction loops
  - per-vehicle speed and lengths

### Data Descriptions

#### per-vehicle speed and lengths

The data is given as a tar file named
[fsp_speed_and_lengths.tar](http://prdownloads.sourceforge.net/sumo/fsp_speed_and_lengths.tar?download)
(\~40MB). Within this tar-file you will find a folder which contains
compressed (gzip) data files. Each data file represents a day for one
direction (north/south). The following days are given:

- 16.02.93
- 17.02.93
- 18.02.93
- 19.02.93
- 22.02.93
- 24.02.93

The files are tab-delimited with the following fields:

- Detector position (in meters)
- Detector lane
- Time (in ?)
- vehicle speed (probably in m/s)
- vehicle length (in m)

### References

Project and data descriptions:

- Petty K.F.; Noeimi H.; Sanwal K.; Rydzewski D.; Skabardonis A.;
  Varaiya P.; Al-Deek H.; "The freeway service patrol evaluation
  project: database support programs, and accessibility"; In:
  Transportation Research Part C: Emerging Technologies, Volume 4,
  Number 2, April 1996 , pp. 71-85(15), Elsevier

Usage examples:

- ["An Example of Microscopic Car Models Validation using the open
  source Traffic Simulation
  SUMO"](http://sumo.dlr.de/pdf/dkrajzew_ESS2002_LOG14_Validation.pdf);
  Daniel Krajzewicz, Georg Hertkorn, Christian Rössel, Peter Wagner;
  In: Proceedings of Simulation in Industry, 14th European Simulation
  Symposium, 2002, SCS European Publishing House, 2002, pp. 318-322,
  Simulation in Industry, 14th European Simulation Symposium, Oct.
  2002, Dresden / Germany
  
  Available as: [.pdf, \~320KB](http://sumo.dlr.de/pdf/dkrajzew_ESS2002_LOG14_Validation.pdf).