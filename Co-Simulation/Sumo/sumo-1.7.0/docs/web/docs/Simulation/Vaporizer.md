---
title: Simulation/Vaporizer
permalink: /Simulation/Vaporizer/
---

# Vaporizer

Vaporizers remove all vehicles as soon as they move onto a specified
edge. A Vaporizers is set into the simulated network by adding the
following declaration line to an "additional file":

```
<additional>
    <vaporizer id="<EDGE_ID>" from="<START_TIME>" end="<END_TIME>"/>
</additional>
```