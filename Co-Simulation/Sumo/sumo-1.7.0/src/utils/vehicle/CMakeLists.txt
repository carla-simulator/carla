set(utils_vehicle_STAT_SRCS
   SUMORouteHandler.cpp
   SUMORouteHandler.h
   SUMORouteLoader.cpp
   SUMORouteLoader.h
   SUMORouteLoaderControl.cpp
   SUMORouteLoaderControl.h
   SUMOVehicle.h
   SUMOTrafficObject.h
   SUMOVehicleParameter.cpp
   SUMOVehicleParameter.h
   SUMOVehicleParserHelper.cpp
   SUMOVehicleParserHelper.h
   SUMOVTypeParameter.cpp
   SUMOVTypeParameter.h)

add_library(utils_vehicle STATIC ${utils_vehicle_STAT_SRCS})
set_property(TARGET utils_vehicle PROPERTY PROJECT_LABEL "z_utils_vehicle")
