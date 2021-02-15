set(netedit_elements_demand_SRCS
   GNERouteHandler.h
   GNERouteHandler.cpp
   GNEDemandElement.h
   GNEDemandElement.cpp
   GNERoute.h
   GNERoute.cpp
   GNEVehicleType.h
   GNEVehicleType.cpp
   GNEVehicle.h
   GNEVehicle.cpp
   GNEStop.h
   GNEStop.cpp
   GNEPersonStop.h
   GNEPersonStop.cpp
   GNEWalk.h
   GNEWalk.cpp
   GNERide.h
   GNERide.cpp
   GNEPersonTrip.h
   GNEPersonTrip.cpp
   GNEPerson.h
   GNEPerson.cpp
   )

add_library(netedit_elements_demand STATIC ${netedit_elements_demand_SRCS})
set_property(TARGET netedit_elements_demand PROPERTY PROJECT_LABEL "z_netedit_elements_demand")
