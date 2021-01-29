set(utils_router_STAT_SRCS
   AStarLookupTable.h
   AStarRouter.h
   AccessEdge.h
   CarEdge.h
   PedestrianEdge.h
   PublicTransportEdge.h
   StopEdge.h
   CHBuilder.h
   CHRouter.h
   CHRouterWrapper.h
   DijkstraRouter.h
   IntermodalEdge.h
   IntermodalNetwork.h
   IntermodalRouter.h
   IntermodalTrip.h
   RailwayRouter.h
   RailEdge.h
   EffortCalculator.h
   GawronCalculator.h
   LogitCalculator.h
   RouteCostCalculator.h
   PedestrianRouter.h
   RouterProvider.h
   SUMOAbstractRouter.h
   SPTree.h
   FareModul.h
   FareToken.h
   FareZones.h)

if (MSVC)
    add_custom_target(z_utils_router SOURCES ${utils_router_STAT_SRCS})
endif()
