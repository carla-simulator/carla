---
title: Contributed
permalink: /Contributed/
---

Some people extended SUMO or built tools to make it more usable. Not all
of these extensions have found its way to what we would call "SUMO
core".

# Within SUMO

The following extensions became a core part of the SUMO package.

- [TraCI](TraCI.md)

    online interaction with the simulation

# Included in the Distribution

The following contributions are included in the package, but are less
supported.

- [Contributed/Cadyts](Contributed/Cadyts.md)

    a tool by Gunnar Flötteröd which adapts the simulate flows to the
    real flows in a known net

- [SUMOPy](Contributed/SUMOPy.md)

    a tool by Joerg Schweizer supporting the whole SUMO toolchain with a
    GUI especially for demand modelling

- [LiSuM](Tools/LiSuM.md)

    a middleware that couples [LISA+](https://www.schlothauer.de/en/software-systems/lisa/) and SUMO to simulate real-world
    traffic light controllers and execute real-world traffic control software within SUMO.

- [SAGA](https://github.com/lcodeca/SUMOActivityGen/blob/master/docs/SUMOActivityGen.md)
    
    a scriptable and user-defined activity-based mobility scenario generator for SUMO. [Generate intermodal traffic demand from OSM input](https://github.com/eclipse/sumo/tree/master/tools/contributed) [howto](https://github.com/lcodeca/SUMOActivityGen/blob/master/docs/HOWTO.md)

- ~~[Contributed/SUMO Traffic
Modeler](Contributed/SUMO_Traffic_Modeler.md)~~

    ~~a graphical user interface for defining high-level traffic for
    SUMO~~ (obsolete)

# External extensions

The following extensions are managed and supported by other parties.

## Demand Generators

- [Citymob for Roadmaps](http://www.grc.upv.es/Software/c4r.html)

    mobility pattern generator for vehicular networks (VANet focus)

## Scenario and Network Editors

- [eWorld](http://eworld.sourceforge.net/)

    an application that allows to convert and enrich roads networks

- [Sumo2Unreal](https://github.com/AugmentedDesignLab/Sumo2Unreal)

    an importer for SUMO's .net.xml road network files into the Unreal
    Engine 4 environment.

## Connections to Network Simulators

- [Veins](https://veins.car2x.org/)

    connects SUMO to OMNeT++

- [iTetris](https://www.ict-itetris.eu/)

    connects SUMO to ns-3

- [<font color="#0174DF">Tra</font><font color="#FF0000">NS</font>](http://trans.epfl.ch/)

    connects SUMO to ns-2

- [MOVE](http://lens1.csie.ncku.edu.tw/wiki/doku.php?id=%E2%80%A7realistic_mobility_generator_for_vehicular_networks)

    connects SUMO to ns-2

- [VSimRTI](http://www.dcaiti.tu-berlin.de/research/simulation/)

    C2X environment connecting SUMO to different network and application simulators (OMNeT++ and JiST/SWANS)

## Other

- [TraCI4Matlab](http://de.mathworks.com/matlabcentral/fileexchange/44805-traci4matlab)
  
    A Matlab interface for connecting and extending information via [TraCI](TraCI.md)

- [TraaS](TraCI/TraaS.md) 
  
    A SOAP(webservice) interface for connecting and extending information via [TraCI](TraCI.md). Also a Java TraCI client library

- [ocit2SUMO](https://github.com/DLR-TS/sumo-ocit)
    
    Generate traffic light signal plans from OCIT® data

- [FLOW](https://flow-project.github.io/)

    a framework for applying reinforcement learning and custom controllers to SUMO, developed at the University of California, Berkeley
    [[1]](https://github.com/flow-project/flow)

- [Webots](https://www.cyberbotics.com/automobile/sumo-interface.php)

    coupling with a robot simulator

- [TETCOS NetSim](https://www.tetcos.com/vanets.html)

    supports VANET simulations by interfacing with SUMO

# Purgatory

The following extensions exist or have existed, but their state is
unclear.

- [Contributed/iTranSIM](Contributed/iTranSIM.md)

    extension by online-calibration using induction loop data by Tino
    Morenz

- [Contributed/SmallMaps](Contributed/SmallMaps.md)

    prunes road networks to a given boundary; by Andreas Florides

- [Contributed/SUMOPlayer](Contributed/SUMOPlayer.md)

!!! note
    SUMOPlayer was removed in release 0.24.0. You should be able to use [traceExporter.py](Tools/TraceExporter.md) for the same task.