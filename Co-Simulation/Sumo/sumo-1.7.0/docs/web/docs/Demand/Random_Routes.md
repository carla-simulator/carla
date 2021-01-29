---
title: Demand/Random Routes
permalink: /Demand/Random_Routes/
---

[duarouter](../duarouter.md) and [jtrrouter](../jtrrouter.md)
used to generate random routes for a given road network with the option
**--random-per-second** {{DT_FLOAT}} (or **-R** {{DT_FLOAT}} for short). Due to a number of bugs in this feature and because
those routes are highly unrealistic, this behavior was replaced with a
python [random trip generation](../Tools/Trip.md#randomtripspy)
script which can also produce a route set employing the
[duarouter](../duarouter.md).

# See Also

- [randomTrips.py](../Tools/Trip.md#randomtripspy) script for random routes generation
- "bug" [random routes only created for timestep 1](http://sourceforge.net/tracker/?func=detail&aid=1914360&group_id=45607&atid=443421)
- "bug" [About Generating random Routes](http://sourceforge.net/tracker/?func=detail&aid=2746763&group_id=45607&atid=443421)