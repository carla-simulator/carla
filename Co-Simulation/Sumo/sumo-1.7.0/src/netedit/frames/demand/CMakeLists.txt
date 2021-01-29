set(netedit_frames_demand_SRCS  
   GNERouteFrame.h
   GNERouteFrame.cpp
   GNEStopFrame.h
   GNEStopFrame.cpp
   GNEPersonFrame.h
   GNEPersonFrame.cpp
   GNEPersonPlanFrame.h
   GNEPersonPlanFrame.cpp
   GNEVehicleFrame.h
   GNEVehicleFrame.cpp
   GNEVehicleTypeFrame.h
   GNEVehicleTypeFrame.cpp
   GNEPersonTypeFrame.h
   GNEPersonTypeFrame.cpp
   )

add_library(netedit_frames_demand STATIC ${netedit_frames_demand_SRCS})
set_property(TARGET netedit_frames_demand PROPERTY PROJECT_LABEL "z_netedit_frames_demand")