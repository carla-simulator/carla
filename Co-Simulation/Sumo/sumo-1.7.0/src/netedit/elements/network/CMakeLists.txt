set(netedit_elements_network_SRCS
   GNENetworkElement.h
   GNENetworkElement.cpp
   GNEEdge.h
   GNEEdge.cpp
   GNELane.h
   GNELane.cpp
   GNEInternalLane.h
   GNEInternalLane.cpp
   GNEConnection.h
   GNEConnection.cpp
   GNECrossing.h
   GNECrossing.cpp
   GNEJunction.h
   GNEJunction.cpp
   GNEProhibition.h
   GNEProhibition.cpp
   )

add_library(netedit_elements_network STATIC ${netedit_elements_network_SRCS})
set_property(TARGET netedit_elements_network PROPERTY PROJECT_LABEL "z_netedit_elements_network")