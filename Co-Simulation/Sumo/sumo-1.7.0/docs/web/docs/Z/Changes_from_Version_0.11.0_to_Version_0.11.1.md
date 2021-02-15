---
title: Z/Changes from Version 0.11.0 to Version 0.11.1
permalink: /Z/Changes_from_Version_0.11.0_to_Version_0.11.1/
---

- All
  - configuration xml format changed again to `<section><key value="val"/></section>` (use {{SUMO}}/tools/10to11.py to convert from old to new
    representation)
  - Bugs in Dijkstra implementation fixed (affecting at least
    duarouter, routing with traci and automatic rerouting)
    (01.10.2009)

- Simulation
  - debugged problems when loading TLS definitions with T=0
  - implemented a variable car-following model API; many thanks to Tobias Mayer
    and Christoph Sommer for the collaboration who did most of the work.
  - debugged occurrence of negative vehicle speeds during emit
  - reworked the mean data output (handling multiple lanes
    correctly)

- sumo-gui
  - implemented [request 1641989: No view reset on reload](http://sourceforge.net/tracker/?func=detail&aid=1641989&group_id=45607&atid=443424).
  - the number of colors used for range visualization is now variable
  - debugged problems with not shown junction names

- netconvert
  - making projection mandatory for OSM and DLR-Navteq networks
    (03.09.2009)
  - removed option **--rotation-to-apply** - was not properly working anyway (02.09.2009)
  - removed support for "old" TIGER networks - assuming the current
    ones are given as shape files; **--tiger** {{DT_FILE}} is now mapped onto **--shapefile** {{DT_FILE}} (02.09.2009)
  - removed support for "split" Elmar networks (option **--elmar** {{DT_FILE}}) (02.09.2009)
  - renamed **--elmar2** {{DT_FILE}} to **--dlr-navteq** {{DT_FILE}} (02.09.2009)
  - renamed **--arcview** {{DT_FILE}} to **--shapefile** {{DT_FILE}} (02.09.2009)
    - please note, that all subsequent options have been adapted
      (but the old names still work as aliases)
  - added the possibility to define [changes of lane numbers along an edge](../Networks/PlainXML.md#road_segment_refining)
    in XML-descriptions (03.09.2009)
  - replaced **--use-projection** by **--proj.utm** and **--proj.dhdn** which determine the correct parameters for
    the two widely used projections from the input data
  - for developers: using same loading procedure for all imported
    networks, see [Developer/How To/Net Importer](../Developer/How_To/Net_Importer.md)

- TraCI
  - debugging sending/receiving messages with length\>255 bytes
  - added further [traffic lights variable retrieval options](../TraCI/Traffic_Lights_Value_Retrieval.md)
  - added further [induction loops variable retrieval options](../TraCI/Induction_Loop_Value_Retrieval.md)
  - added further [multi-entry/multi-exit detectors variable retrieval options](../TraCI/Multi-Entry-Exit_Detectors_Value_Retrieval.md)
  - added further [junction variable retrieval options](../TraCI/Junction_Value_Retrieval.md)
  - added further [vehicle variable retrieval options](../TraCI/Vehicle_Value_Retrieval.md)
  - added further possibilities to [change vehicle values](../TraCI/Change_Vehicle_State.md)
  - added possibilities to [retrieve edge variables](../TraCI/Edge_Value_Retrieval.md) and [change edge values](../TraCI/Change_Edge_State.md)
  - added possibilities to [retrieve lane variables](../TraCI/Lane_Value_Retrieval.md) and [change lane values](../TraCI/Change_Lane_State.md)
  - added possibilities to [change PoI values](../TraCI/Change_PoI_State.md) and to [change polygon values](../TraCI/Change_Polygon_State.md)
  - added the possibility assign a vehicle a new route via TraCI

- dfrouter
  - debugged problems with induction loop measure with time\>end
    time
  - debugged problems with spaces in induction loop measures

- od2trips
  - debugged problems with O/D matrices that have no comments, was:
    [defect 148: od2trips breaks on matrices without comments](http://apps.sourceforge.net/trac/sumo/ticket/148);
    thanks to Wilson Wong for pointing us to it