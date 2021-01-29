set(router_STAT_SRCS
   ROAbstractEdgeBuilder.h
   ROEdge.cpp
   ROEdge.h
   ROFrame.cpp
   ROFrame.h
   ROHelper.cpp
   ROHelper.h
   ROLane.h
   ROLoader.cpp
   ROLoader.h
   RONet.cpp
   RONet.h
   RONetHandler.cpp
   RONetHandler.h
   RONode.cpp
   RONode.h
   ROPerson.cpp
   ROPerson.h
   RORoutable.h
   RORoute.cpp
   RORoute.h
   RORouteDef.cpp
   RORouteDef.h
   RORouteHandler.cpp
   RORouteHandler.h
   ROVehicle.cpp
   ROVehicle.h
)

add_library(router STATIC ${router_STAT_SRCS})
set_property(TARGET router PROPERTY PROJECT_LABEL "z_router")
