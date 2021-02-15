---
title: Tools/Import/MATSim
permalink: /Tools/Import/MATSim/
---

# Introduction

[MATSim](https://www.matsim.org/) is an open source traffic simulation. It
contains tools for generating a virtual population and their daily travel plans.

## matsim_importPlans.py

This tool allows to import person plans from MATSim output.

The call is:

```
tools/import/matsim/matsim_importPlans.py --plan-file matsim_plans.xml -o out.rou.xml
```

The option **--vehicles-only** allows to import only the passenger traffic rather than intermodal trip chains.
