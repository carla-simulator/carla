set(microsim_actions_STAT_SRCS
   Command_SaveTLSState.cpp
   Command_SaveTLSState.h
   Command_SaveTLSSwitches.cpp
   Command_SaveTLSSwitches.h
   Command_SaveTLSSwitchStates.cpp
   Command_SaveTLSSwitchStates.h
   Command_SaveTLSProgram.cpp
   Command_SaveTLSProgram.h
   Command_SaveTLCoupledLaneDet.cpp
   Command_SaveTLCoupledLaneDet.h
   Command_SaveTLCoupledDet.cpp
   Command_SaveTLCoupledDet.h
)

add_library(microsim_actions STATIC ${microsim_actions_STAT_SRCS})
set_property(TARGET microsim_actions PROPERTY PROJECT_LABEL "z_microsim_actions")
