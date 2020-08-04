add_subdirectory(tempstructs)
add_subdirectory(typeloader)

set(netimport_vissim_STAT_SRCS
   NIImporter_Vissim.cpp
   NIImporter_Vissim.h
   NIVissimElements.h
)

add_library(netimport_vissim STATIC ${netimport_vissim_STAT_SRCS})
set_property(TARGET netimport_vissim PROPERTY PROJECT_LABEL "z_netimport_vissim")
