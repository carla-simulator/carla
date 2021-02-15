set(utils_emissions_STAT_SRCS
   HelpersEnergy.cpp
   HelpersEnergy.h
   HelpersHarmonoise.cpp
   HelpersHarmonoise.h
   HelpersHBEFA.cpp
   HelpersHBEFA.h
   HelpersHBEFA3.cpp
   HelpersHBEFA3.h
   HelpersPHEMlight.cpp
   HelpersPHEMlight.h
   PHEMCEP.h
   PHEMCEP.cpp
   PHEMCEPHandler.h
   PHEMCEPHandler.cpp
   PHEMConstants.h
   PollutantsInterface.h
   PollutantsInterface.cpp
)

add_library(utils_emissions STATIC ${utils_emissions_STAT_SRCS})
set_property(TARGET utils_emissions PROPERTY PROJECT_LABEL "z_utils_emissions")
