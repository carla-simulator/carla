---
title: Developer/Implementation Notes/Sumo Modules
permalink: /Developer/Implementation_Notes/Sumo_Modules/
---

## Module Purposes

| **module**     | **submodule**     | **submodule** | **purpose**                                                                          |
| -------------- | ----------------- | ------------- | ------------------------------------------------------------------------------------ |
| *foreign*      |                   |               | *some imported modules (see [Libraries Licenses](../../Libraries_Licenses.md))*    |
|                | *mersenne*        |               | system-independant random number generator                                           |
|                | *fontstash*       |               | draws text in openGL                                                                 |
|                | *rtree*           |               | rtree implementation                                                                 |
|                | *tcpip*           |               | portable socket implementation                                                       |
| *gui*          |                   |               | [sumo-gui](../../sumo-gui.md)-specific windows and threads                         |
| *guinetload*   |                   |               | derived from netload, these classes build guisim-classes instead of microsim-classes |
| *guisim*       |                   |               | classes derived from microsim, extended by visualization and interaction methods     |
| *icons*        |                   |               | application icons (MS Windows)                                                       |
| *microsim*     |                   |               | implementation of the micro simulation                                               |
|                | *actions*         |               | output coupled to tls <font color="red">(rework\! move to output?)</font>            |
|                | *devices*         |               | on-board devices (c2c, gsm)                                                          |
|                | *logging*         |               | value-passing classes <font color="red">(rework\! move to utils?)</font>             |
|                | *output*          |               | generation of microsim-output                                                        |
|                | *traffic_lights* |               | different traffic light algorithms and program switching                             |
|                | *trigger*         |               | classes changing the state of microsim-structures <font color="red">(rework)</font>  |
| *netbuild*     |                   |               | building/preparation of SUMO-networks                                                |
| *netgen*       |                   |               | generate abstract networks                                                           |
| *netload*      |                   |               | loads SUMO-networks for simulation, builds microsim classes                          |
| *od2trips*     |                   |               | Import and conversion of O/D-matrices to trip definitions                            |
| *polyconvert*  |                   |               | Import and conversion of named, colored, geometrical objects                         |
| *router*       |                   |               | Base classes for routing applications                                                |
| *routing_df*  |                   |               | implementation of df-routing based on router-classes                                 |
| *routing_dua* |                   |               | implementation of dua-routing based on router-classes                                |
| *routing_jtr* |                   |               | implementation of jtr-routing based on router-classes                                |
| *utils*        |                   |               | *sets of utility classes*                                                            |
|                | *common*          |               | some helpers with different purposes used by all applications                        |
|                | *distribution*    |               | different distribution representations                                               |
|                | *foxtools*        |               | extensions of the plain fox-toolkit                                                  |
|                | *geom*            |               | geometrical primitives in 2D (line, point, polygon)                                  |
|                | *gui*             |               | *sets of classes used by the gui*                                                    |
|                |                   | *div*         | Different helper classes                                                             |
|                |                   | *drawer*      | Draw network structures, give them colors                                            |
|                |                   | *events*      | Events passed between gui-threads                                                    |
|                |                   | *globjects*   | base class for objects that may be visualized and "touched" within the gui           |
|                |                   | *images*      | Icons and images used within the gui together with an allocating subsystem           |
|                |                   | *tracker*     | time-line visualization                                                              |
|                |                   | *windows*     | Base for windows                                                                     |
|                | *importio*        |               | helper for reading text files                                                        |
|                | *iodevices*       |               | output devices                                                                       |
|                | *options*         |               | the options subsystem used by all applications                                       |
|                | *shapes*          |               | named and colored geometry classes                                                   |
|                | *xml*             |               | xml-helper (basically a layer between the xerces-sax layer and sumo-applications)    |


## Doxygen-Coverage

| **module**                                    | **doxygen** | **throw** | **param** | **remarks**                                |
| --------------------------------------------- | ----------- | --------- | --------- | ------------------------------------------ |
| <font color="orange">*microsim/output*</font> | yes         | yes       | yes       | e2ol is completely broken and undocumented |
| <font color="green">*netgen*</font>           | yes         | yes       | yes       |                                            |
| <font color="green">*od2trips*</font>         | yes         | yes       | yes       |                                            |
| <font color="green">*utils/gui/div*</font>    | yes         | yes       | yes       |                                            |
| <font color="green">*utils/importio*</font>   | yes         | yes       | yes       |                                            |
| <font color="green">*utils/iodevices*</font>  | yes         | yes       | yes       |                                            |
| <font color="orange">*utils/options*</font>   | yes         | yes       | no        |                                            |
| <font color="orange">*utils/xml*</font>       | yes         | yes       | no        |                                            |