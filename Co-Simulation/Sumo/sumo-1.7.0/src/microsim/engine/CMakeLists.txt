set(microsim_engine_STAT_SRCS
   EngineParameters.cpp
   EngineParameters.h
   FirstOrderLagModel.cpp
   FirstOrderLagModel.h
   GenericEngineModel.cpp
   GenericEngineModel.h
   RealisticEngineModel.cpp
   RealisticEngineModel.h
   VehicleEngineHandler.cpp
   VehicleEngineHandler.h
)

add_library(microsim_engine STATIC ${microsim_engine_STAT_SRCS})
set_property(TARGET microsim_engine PROPERTY PROJECT_LABEL "z_microsim_engine")
