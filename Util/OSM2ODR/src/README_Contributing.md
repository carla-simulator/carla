Please feel free to add and / or modify files here but keep in mind that
EPL requires that "derivative works" be licensed under the terms of the EPL.
If the extension is a separate module and is not published under the EPL
it will not be adopted in the SUMO mainline repository.

Modifications to the files and directories listed here are considered _derivative work_:

- activitygen
- dfrouter
- duarouter
- foreign
- gui
- guinetload
- guisim
- jtrrouter
- marouter
- mesogui
- mesosim
- microsim
- netbuild
- netedit
- netgen
- netimport
- netload
- netwrite
- od
- osgview
- polyconvert
- router
- tools
- traci-server
- traci_testclient
- utils
- guisim_main.cpp
- netconvert_main.cpp
- od2trips_main.cpp
- sumo_main.cpp

Directories containing interface code for _separate modules_ (with separate lists) are:

- car following models that inherit from MSCFModel (excluding [modifications of the existing models](microsim/cfmodels/README_Contributing.md))
- lane changing models that inherit from MSAbstractLaneChangeModel (excluding [modifications of the existing models](microsim/lcmodels/README_Contributing.md))
- simulation output modules (excluding [changes to the existing classes](microsim/output/README_Contributing.md))
- vehicle device modules (excluding [changes to the existing classes](microsim/devices/README_Contributing.md))
- network import modules (excluding [changes to the existing classes](netimport/README_Contributing.md))
- network export modules (excluding [changes to the existing classes](netwrite/README_Contributing.md))
