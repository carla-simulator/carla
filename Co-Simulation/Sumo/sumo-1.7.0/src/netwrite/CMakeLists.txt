set(netwrite_STAT_SRCS
   NWFrame.cpp
   NWFrame.h
   NWWriter_Amitran.cpp
   NWWriter_Amitran.h
   NWWriter_MATSim.cpp
   NWWriter_MATSim.h
   NWWriter_OpenDrive.cpp
   NWWriter_OpenDrive.h
   NWWriter_DlrNavteq.cpp
   NWWriter_DlrNavteq.h
   NWWriter_SUMO.cpp
   NWWriter_SUMO.h
   NWWriter_XML.cpp
   NWWriter_XML.h
)

add_library(netwrite STATIC ${netwrite_STAT_SRCS})
set_property(TARGET netwrite PROPERTY PROJECT_LABEL "z_netwrite")
